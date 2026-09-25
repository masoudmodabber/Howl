#!/usr/bin/env python3
"""Persistent Stockfish WDL reference cache and Howl move-regret pilot."""

import argparse
import atexit
import concurrent.futures
import csv
import json
import math
import os
from pathlib import Path
import subprocess
import tempfile

import chess

ENGINE = None
ENGINE_PATH = ""
NODE_BUDGET = 0
HASH_MB = 0


class StockfishUCI:
    def __init__(self, path, threads=1, hash_mb=64):
        self.process = subprocess.Popen([path], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE, text=True, bufsize=1)
        self.identity = {}
        self.options = set()
        self.send("uci")
        while True:
            line = self.readline()
            if line.startswith("id name "):
                self.identity["name"] = line[8:]
            elif line.startswith("id author "):
                self.identity["author"] = line[10:]
            elif line.startswith("option name "):
                self.options.add(line[12:].split(" type ", 1)[0])
            elif line == "uciok":
                break
        self.send(f"setoption name Threads value {threads}")
        self.send(f"setoption name Hash value {hash_mb}")
        self.send("setoption name UCI_ShowWDL value true")
        self.send("isready")
        self.wait("readyok")

    def send(self, command):
        self.process.stdin.write(command + "\n")
        self.process.stdin.flush()

    def readline(self):
        line = self.process.stdout.readline()
        if not line:
            raise RuntimeError("Stockfish stdout closed")
        return line.strip()

    def wait(self, expected):
        while self.readline() != expected:
            pass

    def analyze(self, fen, nodes, multipv, requested_move=None):
        self.send("setoption name Clear Hash")
        self.send(f"setoption name MultiPV value {multipv}")
        self.send("isready")
        self.wait("readyok")
        self.send("ucinewgame")
        self.send(f"position fen {fen}")
        command = f"go nodes {nodes}"
        if requested_move:
            command += f" searchmoves {requested_move}"
        self.send(command)
        latest = {}
        overall_nodes = 0
        while True:
            line = self.readline()
            if line.startswith("bestmove"):
                break
            if not line.startswith("info ") or " pv " not in line:
                continue
            tokens = line.split()
            record = {"depth": None, "seldepth": None, "nodes": None,
                      "score_type": None, "score_value": None,
                      "wdl_wins": None, "wdl_draws": None, "wdl_losses": None, "pv": []}
            mp = 1
            index = 1
            while index < len(tokens):
                token = tokens[index]
                if token in ("depth", "seldepth", "nodes", "multipv") and index + 1 < len(tokens):
                    value = int(tokens[index + 1])
                    if token == "multipv": mp = value
                    else: record[token] = value
                    index += 2
                elif token == "score" and index + 2 < len(tokens):
                    record["score_type"] = tokens[index + 1]
                    record["score_value"] = int(tokens[index + 2])
                    index += 3
                elif token == "wdl" and index + 3 < len(tokens):
                    record["wdl_wins"], record["wdl_draws"], record["wdl_losses"] = map(
                        int, tokens[index + 1:index + 4])
                    index += 4
                elif token == "pv":
                    record["pv"] = tokens[index + 1:]
                    break
                else:
                    index += 1
            if record["nodes"] is not None:
                overall_nodes = max(overall_nodes, record["nodes"])
            if record["pv"] and record["score_type"]:
                latest[mp] = record
        if not latest:
            raise RuntimeError("Stockfish returned no principal variation")
        return [latest[key] for key in sorted(latest)], overall_nodes

    def close(self):
        if self.process.poll() is None:
            self.send("quit")
            try: self.process.wait(timeout=5)
            except subprocess.TimeoutExpired: self.process.kill()


def atomic_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", dir=path.parent, delete=False, encoding="utf-8") as stream:
        json.dump(value, stream, indent=2, sort_keys=True)
        stream.write("\n")
        temporary = stream.name
    os.replace(temporary, path)


def init_worker(engine_path, nodes, hash_mb):
    global ENGINE, ENGINE_PATH, NODE_BUDGET, HASH_MB
    ENGINE_PATH, NODE_BUDGET, HASH_MB = engine_path, nodes, hash_mb
    ENGINE = StockfishUCI(engine_path, threads=1, hash_mb=hash_mb)
    atexit.register(ENGINE.close)


def quality_record(info):
    record = {
        "move": info["pv"][0],
        "depth": info.get("depth"),
        "seldepth": info.get("seldepth"),
        "nodes": info.get("nodes"),
        "pv": info.get("pv", []),
        "score_type": info["score_type"],
        "score_value": info["score_value"],
    }
    if info["wdl_wins"] is not None:
        wins, draws, losses = info["wdl_wins"], info["wdl_draws"], info["wdl_losses"]
        total = wins + draws + losses
        record.update({"wdl_wins": wins, "wdl_draws": draws, "wdl_losses": losses,
                       "quality": (wins + 0.5 * draws) / total})
    else:
        record.update({"wdl_wins": None, "wdl_draws": None, "wdl_losses": None,
                       "quality": None})
    return record


def analyze_task(task):
    position_id, fen, requested_move, multipv = task
    board = chess.Board(fen)
    if requested_move:
        move = chess.Move.from_uci(requested_move)
        if move not in board.legal_moves:
            raise RuntimeError(f"illegal requested move {requested_move} at {position_id}")
    count = 1 if requested_move else min(multipv, board.legal_moves.count())
    infos, used_nodes = ENGINE.analyze(fen, NODE_BUDGET, count, requested_move)
    records = [quality_record(info) for info in infos]
    return position_id, records, used_nodes


def load_rows(path):
    with open(path, newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def engine_identity(path):
    engine = StockfishUCI(path, threads=1, hash_mb=16)
    try:
        return engine.identity, engine.options
    finally:
        engine.close()


def ensure_cache(path, engine_path, identity, nodes, multipv, hash_mb):
    metadata = {"engine_path": str(Path(engine_path).resolve()), "engine_identity": identity,
                "nodes_per_analysis": nodes, "multipv": multipv, "threads": 1,
                "hash_mb": hash_mb, "quality": "wdl_expected_score"}
    if Path(path).exists():
        cache = json.loads(Path(path).read_text(encoding="utf-8"))
        if cache.get("metadata") != metadata:
            raise RuntimeError("existing reference cache metadata is incompatible")
        return cache
    return {"metadata": metadata, "positions": {}}


def run_tasks(tasks, cache, cache_path, workers, engine_path, nodes, hash_mb, multipv_mode):
    total_nodes = 0
    if not tasks:
        return total_nodes
    with concurrent.futures.ProcessPoolExecutor(max_workers=workers, initializer=init_worker,
            initargs=(engine_path, nodes, hash_mb)) as executor:
        futures = {executor.submit(analyze_task, task): task for task in tasks}
        for future in concurrent.futures.as_completed(futures):
            position_id, records, used_nodes = future.result()
            position = cache["positions"][position_id]
            if multipv_mode:
                position["multipv_complete"] = True
                position["best_move"] = records[0]["move"]
                position["best_quality"] = records[0]["quality"]
                position["multipv_moves"] = [record["move"] for record in records]
            for record in records:
                position["candidates"][record["move"]] = record
            total_nodes += used_nodes
            atomic_json(cache_path, cache)
    return total_nodes


def percentile(values, fraction):
    values = sorted(values)
    position = (len(values) - 1) * fraction
    low, high = math.floor(position), math.ceil(position)
    return values[low] if low == high else values[low] * (high - position) + values[high] * (position - low)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=("reference", "complete"))
    parser.add_argument("--sample", default="move-quality-pilot/positions-200.tsv")
    parser.add_argument("--stockfish", default="/home/masoud/Downloads/stockfish/stockfish-linux-x86-64-universal")
    parser.add_argument("--cache", default="move-quality-pilot/reference-cache.json")
    parser.add_argument("--howl-results", default="move-quality-pilot/howl-results.tsv")
    parser.add_argument("--output", default="move-quality-pilot/regret-results.tsv")
    parser.add_argument("--summary", default="move-quality-pilot/summary.json")
    parser.add_argument("--workers", type=int, default=10)
    parser.add_argument("--nodes", type=int, default=5000000)
    parser.add_argument("--multipv", type=int, default=8)
    parser.add_argument("--hash", type=int, default=64)
    parser.add_argument("--expected-positions", type=int, default=200)
    args = parser.parse_args()
    if not 1 <= args.workers <= 10:
        parser.error("--workers must be from 1 through 10")

    rows = load_rows(args.sample)
    if (len(rows) != args.expected_positions or
            len({row["position_id"] for row in rows}) != args.expected_positions):
        raise RuntimeError(f"sample must contain {args.expected_positions} unique positions")
    identity, options = engine_identity(args.stockfish)
    if "UCI_ShowWDL" not in options:
        raise RuntimeError("reference engine does not expose UCI_ShowWDL")
    cache = ensure_cache(args.cache, args.stockfish, identity, args.nodes, args.multipv, args.hash)
    for row in rows:
        cache["positions"].setdefault(row["position_id"], {
            "fen": row["fen"], "multipv_complete": False, "best_move": None,
            "best_quality": None, "candidates": {}})
    atomic_json(args.cache, cache)

    if args.mode == "reference":
        tasks = [(row["position_id"], row["fen"], None, args.multipv) for row in rows
                 if not cache["positions"][row["position_id"]]["multipv_complete"]]
        nodes = run_tasks(tasks, cache, args.cache, args.workers, args.stockfish,
                          args.nodes, args.hash, True)
        print(f"reference_positions_analyzed: {len(tasks)}")
        print(f"reference_nodes_consumed: {nodes}")
        print(f"engine_identity: {identity}")
        return

    howl_rows = load_rows(args.howl_results)
    if len(howl_rows) != 200:
        raise RuntimeError("Howl results must contain 200 positions")
    howl = {row["position_id"]: row for row in howl_rows}
    tasks = []
    for row in rows:
        position = cache["positions"][row["position_id"]]
        if not position["multipv_complete"]:
            raise RuntimeError(f'incomplete MultiPV cache for {row["position_id"]}')
        if "multipv_moves" not in position:
            position["multipv_moves"] = [item["move"] for item in sorted(
                position["candidates"].values(), key=lambda item: item["quality"], reverse=True)]
        move = howl[row["position_id"]]["best_move"]
        if move not in position["candidates"]:
            tasks.append((row["position_id"], row["fen"], move, 1))
    nodes = run_tasks(tasks, cache, args.cache, args.workers, args.stockfish,
                      args.nodes, args.hash, False)

    output_rows = []
    regrets = []
    top1 = top3 = zero = 0
    for row in rows:
        position = cache["positions"][row["position_id"]]
        move = howl[row["position_id"]]["best_move"]
        candidate = position["candidates"][move]
        reference_top = position["multipv_moves"]
        regret = position["best_quality"] - candidate["quality"]
        regrets.append(regret)
        zero += regret <= 1e-12
        top1 += move == position["best_move"]
        top3 += move in reference_top[:3]
        output_rows.append({"position_id": row["position_id"], "fen": row["fen"],
            "howl_move": move, "howl_score": howl[row["position_id"]]["searched_score"],
            "howl_nodes": howl[row["position_id"]]["nodes"], "reference_best_move": position["best_move"],
            "reference_best_quality": position["best_quality"], "howl_move_quality": candidate["quality"],
            "regret": regret, "reference_rank": reference_top.index(move) + 1 if move in reference_top else "outside_top8",
            "on_demand": int(any(task[0] == row["position_id"] for task in tasks))})
    output_path = Path(args.output); output_path.parent.mkdir(parents=True, exist_ok=True)
    fields = list(output_rows[0])
    with output_path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t")
        writer.writeheader(); writer.writerows(output_rows)
    summary = {"positions": len(rows), "mean_regret": sum(regrets)/len(regrets),
        "median_regret": percentile(regrets,.5), "p75_regret": percentile(regrets,.75),
        "p90_regret": percentile(regrets,.90), "p95_regret": percentile(regrets,.95),
        "effectively_zero_fraction": zero/len(rows), "top_move_fraction": top1/len(rows),
        "top3_fraction": top3/len(rows), "on_demand_count": len(tasks),
        "on_demand_nodes": nodes,
        "highest_regret": sorted(output_rows, key=lambda item: item["regret"], reverse=True)[:20]}
    atomic_json(args.summary, summary)
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
