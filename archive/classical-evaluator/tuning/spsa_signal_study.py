#!/usr/bin/env python3
"""Bounded fixed-node signal/throughput study for Howl classic SPSA."""

from __future__ import annotations

import argparse
from concurrent.futures import ProcessPoolExecutor, as_completed
import csv
import io
import json
import math
from pathlib import Path
import random
import statistics
import sys
import time

import chess
import chess.pgn

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from fishtest_spsa import (ClassicSPSA, atomic_json, build_variant, read_manifest,
                           stochastic_round)  # noqa: E402
from spsa_tune import STARTING_POSITIONS, SingleGameTask, _run_single_game_worker  # noqa: E402

SCALES = (0.5, 1.0, 2.0)
PREFIXES = (8, 16, 32, 64)


def write_tsv(path: Path, rows: list[dict]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if not rows:
        return
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]), delimiter="\t")
        writer.writeheader(); writer.writerows(rows)


def read_tsv(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        return []
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def opening_tasks(pairs: int, concurrency: int, nodes: int, plus: Path, minus: Path,
                  syzygy_path: str, opening_offset: int = 0) -> list[SingleGameTask]:
    tasks = []
    options = {"SyzygyPath": syzygy_path, "SyzygyProbeLimit": 5}
    for pair in range(pairs):
        name, fen = STARTING_POSITIONS[(opening_offset + pair) % len(STARTING_POSITIONS)]
        for color in range(2):
            game = pair * 2 + color
            tasks.append(SingleGameTask(
                game_index=game, pos_name=name, starting_fen=fen,
                white_role="plus" if color == 0 else "minus",
                black_role="minus" if color == 0 else "plus",
                plus_engine_path=str(plus), minus_engine_path=str(minus),
                base_time_sec=1.0, inc_sec=0.0, cpu_core=game % concurrency,
                uci_options=options, nodes_per_move=nodes,
            ))
    return tasks


def play(tasks: list[SingleGameTask], concurrency: int, allow_invalid: bool = False):
    results = [None] * len(tasks)
    start = time.monotonic()
    with ProcessPoolExecutor(max_workers=concurrency) as pool:
        futures = {pool.submit(_run_single_game_worker, task): task.game_index for task in tasks}
        for future in as_completed(futures):
            results[futures[future]] = future.result()
    elapsed = time.monotonic() - start
    failures = [f"game {index}: {getattr(result, 'error_message', 'missing result')}"
                for index, result in enumerate(results)
                if result is None or (not result.is_valid and not allow_invalid)]
    if failures:
        raise RuntimeError("game failure in signal study: " + "; ".join(failures))
    return results, elapsed


def ply_count(result) -> int:
    game = chess.pgn.read_game(io.StringIO(result.pgn_str))
    return sum(1 for _ in game.mainline_moves())


def first_move(result) -> str:
    game = chess.pgn.read_game(io.StringIO(result.pgn_str))
    move = next(iter(game.mainline_moves()), None)
    return move.uci() if move else ""


def pair_metrics(results) -> list[dict]:
    rows = []
    for index in range(0, len(results), 2):
        first, second = results[index], results[index + 1]
        plus_score = first.plus_score + second.plus_score
        rows.append({"pair": index // 2, "plus_score": plus_score,
                     "D": plus_score - (2.0 - plus_score),
                     "root_disagreement": int(first_move(first) != first_move(second)),
                     "results": first.result + "," + second.result})
    return rows


def sign(value: float) -> int:
    return (value > 0) - (value < 0)


def bootstrap_ci(values: list[float], seed: int, replicates: int = 10000) -> tuple[float, float]:
    rng = random.Random(seed)
    samples = sorted(sum(values[rng.randrange(len(values))] for _ in values)
                     for _ in range(replicates))
    return samples[int(0.025 * replicates)], samples[int(0.975 * replicates)]


def perturb(manifest, theta, direction: int, scale: float, num_iter: int,
            A: int, alpha: float, gamma: float, seed: int):
    rng = random.Random(seed + direction * 1_000_003)
    plus, minus = {}, {}
    flips = []
    for row in manifest:
        flip = rng.choice((-1, 1)); flips.append(flip)
        c_end = float(row["natural_scale"])
        c = c_end * num_iter ** gamma * scale
        value = float(theta[row["name"]]); low=float(row["minimum"]); high=float(row["maximum"])
        plus[row["name"]] = stochastic_round(min(max(value + c * flip, low), high), rng)
        minus[row["name"]] = stochastic_round(min(max(value - c * flip, low), high), rng)
    return plus, minus, flips


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--build-dir", default="build")
    parser.add_argument("--manifest", default="move-quality-tuning/parameter-manifest.tsv")
    parser.add_argument("--output", default="move-quality-tuning/spsa-signal-study")
    parser.add_argument("--nodes-per-move", type=int, default=1000)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--syzygy-path", default="")
    args = parser.parse_args()
    repo=Path(args.repo_root).resolve(); build=(repo/args.build_dir).resolve(); out=(repo/args.output).resolve()
    out.mkdir(parents=True, exist_ok=True)
    manifest=read_manifest((repo/args.manifest).resolve())
    theta={row["name"]:int(row["current_value"]) for row in manifest}
    production=build_variant(repo,build,out,theta,"production")

    throughput=[{key:(float(value) if key in {"seconds","games_per_second","nodes_per_second"} else int(value))
                 for key,value in row.items()} for row in read_tsv(out/"throughput.tsv")]
    throughput_games=16
    measured={int(row["concurrency"]) for row in throughput}
    for concurrency in (1,4,8,12,16):
        if concurrency in measured:
            continue
        results, elapsed=play(opening_tasks(throughput_games//2,concurrency,args.nodes_per_move,
                                            production,production,args.syzygy_path),concurrency)
        nodes=sum(ply_count(result) for result in results)*args.nodes_per_move
        throughput.append({"concurrency":concurrency,"games":len(results),"nodes":nodes,
                           "seconds":elapsed,"games_per_second":len(results)/elapsed,
                           "nodes_per_second":nodes/elapsed})
        write_tsv(out/"throughput.tsv",throughput)
    best=max(throughput,key=lambda row:row["games_per_second"])
    selected=int(best["concurrency"])

    # Fishtest schedule implied by the runner defaults: 1000 batches x 4 pairs.
    num_iter=4000; A=int(0.1*num_iter); alpha=0.602; gamma=0.101
    perturb_rows=read_tsv(out/"perturbations.tsv")
    for row in perturb_rows:
        row.setdefault("invalid_games", "0")
    signal_rows=read_tsv(out/"signal-results.tsv")
    all_pair_rows=read_tsv(out/"pair-results.tsv")
    completed={(float(row["scale"]),int(row["direction"])) for row in perturb_rows}
    for scale in SCALES:
        for direction in range(20):
            if (scale,direction) in completed:
                continue
            plus_vector,minus_vector,flips=perturb(manifest,theta,direction,scale,num_iter,A,alpha,gamma,args.seed)
            plus=build_variant(repo,build,out,plus_vector,f"s{scale}-d{direction}-plus")
            minus=build_variant(repo,build,out,minus_vector,f"s{scale}-d{direction}-minus")
            results,elapsed=play(opening_tasks(64,selected,args.nodes_per_move,plus,minus,args.syzygy_path),
                                 selected,allow_invalid=True)
            pairs=pair_metrics(results)
            disagreement=sum(row["root_disagreement"] for row in pairs)/64
            perturb_rows.append({"scale":scale,"direction":direction,"plus_key":plus.name,
                                 "minus_key":minus.name,"root_disagreement":disagreement,"seconds":elapsed,
                                 "invalid_games":sum(not result.is_valid for result in results)})
            for row in pairs:
                all_pair_rows.append({"scale":scale,"direction":direction,**row})
            d64=sum(row["D"] for row in pairs)
            ci=bootstrap_ci([row["D"] for row in pairs],args.seed+direction+int(scale*1000))
            w=sum(result.winner=="plus" for result in results); l=sum(result.winner=="minus" for result in results)
            draws=len(results)-w-l
            penta=[0]*5
            for row in pairs:penta[int(round((row["D"]+2)))] += 1
            for prefix in PREFIXES:
                D=sum(row["D"] for row in pairs[:prefix])
                signal_rows.append({"scale":scale,"direction":direction,"pairs":prefix,
                                    "wins":w if prefix==64 else "","draws":draws if prefix==64 else "",
                                    "losses":l if prefix==64 else "","pentanomial":json.dumps(penta) if prefix==64 else "",
                                    "D":D,"sign":sign(D),"reference_D":d64,"reference_sign":sign(d64),
                                    "sign_agreement":int(sign(D)==sign(d64)),
                                    "bootstrap_95_low":ci[0] if prefix==64 else "",
                                    "bootstrap_95_high":ci[1] if prefix==64 else "",
                                    "root_disagreement":disagreement})
            write_tsv(out/"perturbations.tsv",perturb_rows)
            write_tsv(out/"signal-results.tsv",signal_rows)
            write_tsv(out/"pair-results.tsv",all_pair_rows)

    aggregate=[]
    for scale in SCALES:
        for prefix in PREFIXES:
            rows=[row for row in signal_rows if float(row["scale"])==scale and int(row["pairs"])==prefix]
            absolute=[abs(float(row["D"])) for row in rows]
            aggregate.append({"scale":scale,"pairs":prefix,
                              "sign_agreement":sum(int(row["sign_agreement"]) for row in rows)/len(rows),
                              "mean_absolute_D":statistics.mean(absolute),
                              "median_absolute_D":statistics.median(absolute),
                              "zero_fraction":sum(value==0 for value in absolute)/len(absolute),
                              "mean_root_disagreement":statistics.mean(float(row["root_disagreement"]) for row in rows)})
    write_tsv(out/"c-scale-results.tsv",aggregate)
    # Pick the smallest scale with nonzero behavior, then the smallest prefix with >=75% sign agreement.
    scale_summary={scale:next(row for row in aggregate if row["scale"]==scale and row["pairs"]==64) for scale in SCALES}
    recommended_scale=next((scale for scale in SCALES if scale_summary[scale]["mean_root_disagreement"]>0.01
                            and scale_summary[scale]["zero_fraction"]<0.5),2.0)
    recommended_pairs=next((p for p in (8,16,32,64) if next(row for row in aggregate
                            if row["scale"]==recommended_scale and row["pairs"]==p)["sign_agreement"]>=0.75),64)
    gps=float(best["games_per_second"])
    estimates={str(updates):{"pairs_per_update":recommended_pairs,"games":updates*recommended_pairs*2,
                             "seconds":updates*recommended_pairs*2/gps} for updates in (500,1000,2000)}
    summary={"nodes_per_move":args.nodes_per_move,"selected_concurrency":selected,
             "throughput":best,"aggregates":aggregate,"recommended_c_scale":recommended_scale,
             "recommended_pairs":recommended_pairs,"campaign_estimates":estimates,
             "parameters":len(manifest)}
    atomic_json(out/"summary.json",summary)
    (out/"report.md").write_text("# Howl classic SPSA signal study\n\n```json\n"+
                                  json.dumps(summary,indent=2,sort_keys=True)+"\n```\n",encoding="utf-8")
    print(json.dumps(summary,sort_keys=True))
    return 0


if __name__ == "__main__": raise SystemExit(main())
