#!/usr/bin/env python3
"""Freeze corpus split and build the symmetric constrained-move baseline."""

import argparse
import concurrent.futures
import csv
import json
import math
from pathlib import Path

from build_knight_outpost_sample import stable_hash
from external_move_quality_pilot import StockfishUCI, atomic_json

ENGINE = None
NODES = 0


def load_tsv(path):
    with open(path, newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def prepare_split(corpus_path, split_path, determinism_path):
    rows = load_tsv(corpus_path)
    if len(rows) != 1000:
        raise RuntimeError("reference corpus must contain exactly 1000 positions")
    games = {}
    for row in rows:
        games.setdefault(row["source_game"], []).append(row)
    ordered = sorted(games, key=lambda game: (stable_hash(game + "|validation"), game))
    reachable = {0: []}
    for game in ordered:
        size = len(games[game])
        for total, selected in list(reachable.items())[::-1]:
            next_total = total + size
            if next_total <= 200 and next_total not in reachable:
                reachable[next_total] = selected + [game]
        if 200 in reachable:
            break
    if 200 not in reachable:
        raise RuntimeError("could not construct exact game-disjoint validation split")
    validation_games = set(reachable[200])
    output = []
    for row in rows:
        split = "validation" if row["source_game"] in validation_games else "train"
        output.append({"position_id": row["position_id"], "fen": row["fen"], "result": "0.5",
                       "group": row["source_game"], "split": split, "weight": "1"})
    fields = ["position_id", "fen", "result", "group", "split", "weight"]
    Path(split_path).parent.mkdir(parents=True, exist_ok=True)
    with open(split_path, "w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fields, delimiter="\t"); writer.writeheader(); writer.writerows(output)
    subset = sorted(output, key=lambda row: (stable_hash(row["position_id"] + "|determinism"), row["position_id"]))[:20]
    with open(determinism_path, "w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fields, delimiter="\t"); writer.writeheader(); writer.writerows(subset)
    print(f"train: {sum(row['split']=='train' for row in output)}")
    print(f"validation: {sum(row['split']=='validation' for row in output)}")
    print(f"train_games: {len(games)-len(validation_games)}")
    print(f"validation_games: {len(validation_games)}")


def init_worker(stockfish, nodes, hash_mb):
    global ENGINE, NODES
    NODES = nodes
    ENGINE = StockfishUCI(stockfish, threads=1, hash_mb=hash_mb)


def analyze_move(task):
    position_id, fen, move = task
    records, nodes = ENGINE.analyze(fen, NODES, 1, move)
    record = records[0]
    record["move"] = record["pv"][0]
    total = record["wdl_wins"] + record["wdl_draws"] + record["wdl_losses"]
    record["quality"] = (record["wdl_wins"] + 0.5 * record["wdl_draws"]) / total
    return position_id, move, record, nodes


def percentile(values, fraction):
    values = sorted(values)
    position = (len(values) - 1) * fraction
    low, high = math.floor(position), math.ceil(position)
    return values[low] if low == high else values[low] * (high-position) + values[high] * (position-low)


def aggregate(rows):
    regrets = [row["regret"] for row in rows]
    return {"positions": len(rows), "mean_regret": sum(regrets)/len(regrets),
            "median_regret": percentile(regrets,.5), "p75_regret": percentile(regrets,.75),
            "p90_regret": percentile(regrets,.90), "p95_regret": percentile(regrets,.95),
            "zero_regret_fraction": sum(value <= 1e-12 for value in regrets)/len(regrets),
            "reference_move_agreement": sum(row["howl_move"] == row["reference_move"] for row in rows)/len(rows),
            "mean_reference_expected_score_loss": sum(regrets)/len(regrets)}


def build_baseline(args):
    split_rows = load_tsv(args.split)
    howl_rows = load_tsv(args.howl_results)
    if len(split_rows) != 1000 or len(howl_rows) != 1000:
        raise RuntimeError("split and Howl results must each contain 1000 positions")
    howl = {row["position_id"]: row for row in howl_rows}
    reference = json.loads(Path(args.reference_cache).read_text(encoding="utf-8"))
    identity_engine = StockfishUCI(args.stockfish, 1, args.hash)
    identity = identity_engine.identity; identity_engine.close()
    metadata = {"stockfish_identity": identity, "stockfish_path": str(Path(args.stockfish).resolve()),
                "nodes_per_move": args.nodes, "threads": 1, "hash_mb": args.hash,
                "quality": "wdl_expected_score", "mode": "constrained_searchmoves"}
    cache_path = Path(args.constrained_cache)
    if cache_path.exists():
        cache = json.loads(cache_path.read_text(encoding="utf-8"))
        if cache["metadata"] != metadata:
            raise RuntimeError("constrained cache metadata mismatch")
    else:
        cache = {"metadata": metadata, "positions": {}}
    tasks = []
    for row in split_rows:
        pid = row["position_id"]
        ref_position = reference["positions"][pid]
        reference_move = ref_position["best_move"]
        howl_move = howl[pid]["best_move"]
        position = cache["positions"].setdefault(pid, {"fen": row["fen"], "moves": {}})
        required = [reference_move] + ([] if howl_move == reference_move else [howl_move])
        for move in required:
            if move not in position["moves"]:
                tasks.append((pid, row["fen"], move))
    atomic_json(cache_path, cache)
    consumed = 0
    if tasks:
        with concurrent.futures.ProcessPoolExecutor(max_workers=args.workers, initializer=init_worker,
                initargs=(args.stockfish, args.nodes, args.hash)) as executor:
            futures = [executor.submit(analyze_move, task) for task in tasks]
            for future in concurrent.futures.as_completed(futures):
                pid, move, record, nodes = future.result()
                cache["positions"][pid]["moves"][move] = record
                consumed += nodes
                atomic_json(cache_path, cache)

    baseline = []
    for row in split_rows:
        pid = row["position_id"]
        reference_move = reference["positions"][pid]["best_move"]
        howl_move = howl[pid]["best_move"]
        reference_record = cache["positions"][pid]["moves"][reference_move]
        howl_record = reference_record if howl_move == reference_move else cache["positions"][pid]["moves"][howl_move]
        raw = reference_record["quality"] - howl_record["quality"]
        baseline.append({"position_id": pid, "split": row["split"], "fen": row["fen"],
            "howl_move": howl_move, "reference_move": reference_move,
            "howl_quality": howl_record["quality"], "reference_quality": reference_record["quality"],
            "raw_regret": raw, "regret": max(0.0, raw),
            "howl_searched_score": howl[pid]["searched_score"], "howl_nodes": howl[pid]["nodes"]})
    fields = list(baseline[0])
    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    with open(args.output, "w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fields, delimiter="\t"); writer.writeheader(); writer.writerows(baseline)
    summary = {"all": aggregate(baseline),
               "train": aggregate([row for row in baseline if row["split"] == "train"]),
               "validation": aggregate([row for row in baseline if row["split"] == "validation"]),
               "additional_stockfish_nodes": consumed,
               "total_cached_move_analyses": sum(len(position["moves"]) for position in cache["positions"].values())}
    atomic_json(args.summary, summary)
    print(json.dumps(summary, indent=2))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=("prepare", "baseline"))
    parser.add_argument("--corpus", default="move-quality-reference/corpus-1000.tsv")
    parser.add_argument("--split", default="move-quality-reference/frozen-split-800-200.tsv")
    parser.add_argument("--determinism-sample", default="move-quality-reference/determinism-20.tsv")
    parser.add_argument("--howl-results", default="move-quality-reference/howl-baseline-search.tsv")
    parser.add_argument("--reference-cache", default="move-quality-reference/reference-cache-5m-multipv8.json")
    parser.add_argument("--constrained-cache", default="move-quality-reference/constrained-move-cache-5m.json")
    parser.add_argument("--output", default="move-quality-reference/frozen-howl-baseline.tsv")
    parser.add_argument("--summary", default="move-quality-reference/frozen-howl-baseline-summary.json")
    parser.add_argument("--stockfish", default="/home/masoud/Downloads/stockfish/stockfish-linux-x86-64-universal")
    parser.add_argument("--nodes", type=int, default=5000000)
    parser.add_argument("--workers", type=int, default=10)
    parser.add_argument("--hash", type=int, default=64)
    args = parser.parse_args()
    if args.mode == "prepare": prepare_split(args.corpus, args.split, args.determinism_sample)
    else: build_baseline(args)


if __name__ == "__main__":
    main()
