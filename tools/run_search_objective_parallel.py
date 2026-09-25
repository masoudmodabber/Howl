#!/usr/bin/env python3
"""Run frozen search-objective positions in isolated deterministic processes."""

import argparse
import csv
import json
import math
from pathlib import Path
import subprocess
import tempfile


FIELDS = ("position_id", "searched_score", "best_move", "actual_result",
          "squared_error", "weight", "nodes")


def read_tsv(path):
    with open(path, newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def write_sample(path, rows):
    with open(path, "w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys(), delimiter="\t")
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sample", default="tuner-samples/knight-outpost-weighted-500.tsv")
    parser.add_argument("--engine", default="build/howl_search_tuner_objective")
    parser.add_argument("--workers", type=int, default=1)
    parser.add_argument("--nodes", type=int, default=500000)
    parser.add_argument("--k", type=float, default=1.0)
    parser.add_argument("--score-clamp", type=int, default=2000)
    parser.add_argument("--set-knight-outpost", nargs=2, metavar=("NAME", "VALUE"))
    parser.add_argument("--set-rook-file", nargs=2, action="append", metavar=("NAME", "VALUE"))
    parser.add_argument("--set-isolated-pawn", nargs=2, action="append", metavar=("NAME", "VALUE"))
    parser.add_argument("--results-out")
    parser.add_argument("--summary-out")
    args = parser.parse_args()
    if args.workers < 1 or args.workers > 12:
        parser.error("--workers must be between 1 and 12")
    if args.nodes < 1:
        parser.error("--nodes must be positive")

    sample = read_tsv(args.sample)
    if not sample:
        raise RuntimeError("sample is empty")
    ids = [row["position_id"] for row in sample]
    if len(ids) != len(set(ids)):
        raise RuntimeError("sample position identifiers are not unique")

    with tempfile.TemporaryDirectory(prefix="howl-search-objective-") as directory:
        directory = Path(directory)
        processes = []
        result_paths = []
        for worker in range(args.workers):
            worker_rows = [row for index, row in enumerate(sample) if index % args.workers == worker]
            if not worker_rows:
                continue
            sample_path = directory / f"sample-{worker}.tsv"
            result_path = directory / f"results-{worker}.tsv"
            write_sample(sample_path, worker_rows)
            command = [args.engine, "--sample", str(sample_path), "--nodes", str(args.nodes),
                       "--k", str(args.k), "--score-clamp", str(args.score_clamp),
                       "--results-out", str(result_path)]
            if args.set_knight_outpost:
                command.extend(("--set-knight-outpost", *args.set_knight_outpost))
            if args.set_rook_file:
                for override_value in args.set_rook_file:
                    command.extend(("--set-rook-file", *override_value))
            if args.set_isolated_pawn:
                for override_value in args.set_isolated_pawn:
                    command.extend(("--set-isolated-pawn", *override_value))
            processes.append((worker, command, subprocess.Popen(command, stdout=subprocess.PIPE,
                                                                stderr=subprocess.PIPE, text=True)))
            result_paths.append(result_path)
        for worker, command, process in processes:
            stdout, stderr = process.communicate()
            if process.returncode:
                raise RuntimeError(f"worker {worker} failed ({' '.join(command)}):\n{stdout}{stderr}")

        results = {}
        for path in result_paths:
            for row in read_tsv(path):
                if row["position_id"] in results:
                    raise RuntimeError(f'duplicate worker result: {row["position_id"]}')
                results[row["position_id"]] = row
        missing = [position_id for position_id in ids if position_id not in results]
        if missing or len(results) != len(sample):
            raise RuntimeError(f"worker result mismatch: missing={len(missing)} extra={len(results)-len(sample)}")
        ordered = [results[position_id] for position_id in ids]

    weighted_error = 0.0
    weight_sum = 0.0
    total_nodes = 0
    for row in ordered:
        weight = float(row["weight"])
        weighted_error += weight * float(row["squared_error"])
        weight_sum += weight
        total_nodes += int(row["nodes"])
    mse = weighted_error / weight_sum

    if args.results_out:
        with open(args.results_out, "w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=FIELDS, delimiter="\t")
            writer.writeheader()
            writer.writerows(ordered)
    summary = {
        "positions_processed": len(ordered),
        "workers": args.workers,
        "node_budget": args.nodes,
        "total_nodes": total_nodes,
        "weight_sum": weight_sum,
        "weighted_mse": mse,
        "weighted_rmse": math.sqrt(mse),
    }
    if args.summary_out:
        Path(args.summary_out).write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n",
                                          encoding="utf-8")
    print(f"positions_processed: {len(ordered)}")
    print(f"workers: {args.workers}")
    print(f"node_budget: {args.nodes}")
    print(f"total_nodes: {total_nodes}")
    print(f"weight_sum: {weight_sum:.17g}")
    print(f"weighted_mse: {mse:.17g}")
    print(f"weighted_rmse: {math.sqrt(mse):.17g}")


if __name__ == "__main__":
    main()
