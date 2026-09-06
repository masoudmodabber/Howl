#!/usr/bin/env python3
"""Runner for Howl chess engine diagnostic position suite."""

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import csv
from datetime import datetime
import os
from pathlib import Path
import subprocess
import sys
import time
from typing import Any, Dict, List, Optional, Tuple


def parse_info_line(line: str) -> Dict[str, str]:
    """Parse tokens from a UCI 'info' line."""
    tokens = line.strip().split()
    parsed: Dict[str, str] = {}
    i = 0
    keywords = {
        "depth", "seldepth", "time", "nodes", "pv", "multipv",
        "score", "currmove", "currmovenumber", "hashfull", "nps",
        "tbhits", "sbhits", "cpuload", "string", "refutation", "currline"
    }

    while i < len(tokens):
        token = tokens[i]
        if token == "depth" and i + 1 < len(tokens):
            parsed["depth"] = tokens[i + 1]
            i += 2
        elif token == "nodes" and i + 1 < len(tokens):
            parsed["nodes"] = tokens[i + 1]
            i += 2
        elif token == "score" and i + 1 < len(tokens):
            st = tokens[i + 1]
            if st in ("cp", "mate") and i + 2 < len(tokens):
                parsed["score_type"] = st
                parsed["score_value"] = tokens[i + 2]
                parsed["score"] = f"{st} {tokens[i + 2]}"
                i += 3
            else:
                parsed["score_type"] = "cp"
                parsed["score_value"] = st
                parsed["score"] = st
                i += 2
        elif token == "pv":
            pv_tokens = []
            i += 1
            while i < len(tokens) and tokens[i] not in keywords:
                pv_tokens.append(tokens[i])
                i += 1
            parsed["pv"] = " ".join(pv_tokens)
        else:
            i += 1

    return parsed


def run_single_position(
    engine_bin: str,
    fen: str,
    movetime_ms: int = 20000,
    nodes_budget: Optional[int] = None,
    timeout_sec: float = 35.0,
) -> Dict[str, str]:
    """Execute a single-threaded Howl search on one position."""
    if not os.path.isfile(engine_bin):
        raise FileNotFoundError(f"Engine binary not found: {engine_bin}")
    if not os.access(engine_bin, os.X_OK):
        raise PermissionError(f"Engine binary is not executable: {engine_bin}")

    proc = subprocess.Popen(
        [engine_bin],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
    )

    try:
        def send(cmd: str) -> None:
            proc.stdin.write(f"{cmd}\n")
            proc.stdin.flush()

        send("uci")
        while True:
            line = proc.stdout.readline()
            if not line:
                raise RuntimeError("Howl stdout closed unexpectedly during uci handshake")
            if line.strip() == "uciok":
                break

        send("isready")
        while True:
            line = proc.stdout.readline()
            if not line:
                raise RuntimeError("Howl stdout closed unexpectedly during isready")
            if line.strip() == "readyok":
                break

        send("ucinewgame")
        send("isready")
        while True:
            line = proc.stdout.readline()
            if not line:
                raise RuntimeError("Howl stdout closed unexpectedly during ucinewgame")
            if line.strip() == "readyok":
                break

        send(f"position fen {fen}")
        if nodes_budget is not None and nodes_budget > 0:
            send(f"go nodes {nodes_budget}")
        else:
            send(f"go movetime {movetime_ms}")

        completed_depth = ""
        nodes = ""
        score_type = ""
        score_value = ""
        pv = ""
        best_move = ""

        start_time = time.monotonic()
        while True:
            if time.monotonic() - start_time > timeout_sec:
                raise TimeoutError(f"Timed out during search for FEN: {fen}")

            line = proc.stdout.readline()
            if not line:
                raise RuntimeError("Howl stdout closed unexpectedly during search")

            line_str = line.strip()
            if line_str.startswith("info"):
                info = parse_info_line(line_str)
                if "depth" in info:
                    completed_depth = info["depth"]
                if "nodes" in info:
                    nodes = info["nodes"]
                if "score_type" in info:
                    score_type = info["score_type"]
                if "score_value" in info:
                    score_value = info["score_value"]
                if "pv" in info:
                    pv = info["pv"]
            elif line_str.startswith("bestmove"):
                tokens = line_str.split()
                if len(tokens) >= 2:
                    best_move = tokens[1]
                else:
                    best_move = "(none)"
                break

        send("quit")
        try:
            proc.wait(timeout=2.0)
        except subprocess.TimeoutExpired:
            proc.kill()

        return {
            "completed_depth": completed_depth,
            "nodes": nodes,
            "score_type": score_type,
            "score_value": score_value,
            "best_move": best_move,
            "pv": pv,
        }
    finally:
        if proc.poll() is None:
            try:
                proc.kill()
            except Exception:
                pass


def load_tsv_dict(filepath: Path, key_col: str = "id") -> Dict[str, Dict[str, str]]:
    """Load TSV file as a dictionary keyed by key_col."""
    if not filepath.exists():
        return {}
    data: Dict[str, Dict[str, str]] = {}
    with open(filepath, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            k = row.get(key_col, "")
            if k:
                data[k] = row
    return data


def run_diagnostics(
    positions_file: str,
    engine_bin: str,
    movetime_ms: int = 20000,
    nodes_budget: Optional[int] = None,
    concurrency: int = 12,
    reference_file: str = "diagnostics/reference.tsv",
    output_file: Optional[str] = None,
    timeout_buffer: float = 15.0,
) -> str:
    positions_path = Path(positions_file)
    if not positions_path.exists():
        raise FileNotFoundError(f"Positions file not found: {positions_file}")

    positions: List[Dict[str, str]] = []
    with open(positions_path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            positions.append(row)

    if not positions:
        raise ValueError(f"No positions found in {positions_file}")

    ref_data = load_tsv_dict(Path(reference_file), key_col="id")

    if not output_file:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        results_dir = Path("diagnostics/results")
        results_dir.mkdir(parents=True, exist_ok=True)
        out_path = results_dir / f"results_{timestamp}.tsv"
    else:
        out_path = Path(output_file)
        out_path.parent.mkdir(parents=True, exist_ok=True)

    fieldnames = [
        "id",
        "fen",
        "game_phase",
        "movetime_ms",
        "completed_depth",
        "nodes",
        "score_type",
        "score_value",
        "best_move",
        "pv",
        "stockfish_move_1",
        "stockfish_score_1_type",
        "stockfish_score_1_value",
        "stockfish_pv_1",
        "stockfish_move_2",
        "stockfish_score_2_type",
        "stockfish_score_2_value",
        "stockfish_pv_2",
        "stockfish_move_3",
        "stockfish_score_3_type",
        "stockfish_score_3_value",
        "stockfish_pv_3",
        "stockfish_rank_of_howl_move",
    ]

    total_timeout = (movetime_ms / 1000.0) + timeout_buffer if movetime_ms > 0 else 60.0

    print(
        f"Running diagnostics for {len(positions)} positions using {concurrency} concurrent Howl processes..."
    )
    t_start = time.monotonic()

    results_by_index: Dict[int, Dict[str, str]] = {}

    def worker(idx: int, pos: Dict[str, str]) -> Tuple[int, Dict[str, str]]:
        pos_id = pos.get("id", "")
        fen = pos.get("fen", "")
        game_phase = pos.get("game_phase", "")

        t0 = time.monotonic()
        search_res = run_single_position(
            engine_bin=engine_bin,
            fen=fen,
            movetime_ms=movetime_ms,
            nodes_budget=nodes_budget,
            timeout_sec=total_timeout,
        )
        elapsed = time.monotonic() - t0

        ref = ref_data.get(pos_id, {})
        sf_m1 = ref.get("stockfish_move_1", "")
        sf_m2 = ref.get("stockfish_move_2", "")
        sf_m3 = ref.get("stockfish_move_3", "")

        best_move = search_res["best_move"]
        if best_move and best_move != "(none)":
            if best_move == sf_m1:
                rank = "1"
            elif best_move == sf_m2:
                rank = "2"
            elif best_move == sf_m3:
                rank = "3"
            else:
                rank = "outside_top_3"
        else:
            rank = "outside_top_3"

        row = {
            "id": pos_id,
            "fen": fen,
            "game_phase": game_phase,
            "movetime_ms": str(movetime_ms),
            "completed_depth": search_res["completed_depth"],
            "nodes": search_res["nodes"],
            "score_type": search_res["score_type"],
            "score_value": search_res["score_value"],
            "best_move": best_move,
            "pv": search_res["pv"],
            "stockfish_move_1": sf_m1,
            "stockfish_score_1_type": ref.get("stockfish_score_1_type", ""),
            "stockfish_score_1_value": ref.get("stockfish_score_1_value", ""),
            "stockfish_pv_1": ref.get("stockfish_pv_1", ""),
            "stockfish_move_2": sf_m2,
            "stockfish_score_2_type": ref.get("stockfish_score_2_type", ""),
            "stockfish_score_2_value": ref.get("stockfish_score_2_value", ""),
            "stockfish_pv_2": ref.get("stockfish_pv_2", ""),
            "stockfish_move_3": sf_m3,
            "stockfish_score_3_type": ref.get("stockfish_score_3_type", ""),
            "stockfish_score_3_value": ref.get("stockfish_score_3_value", ""),
            "stockfish_pv_3": ref.get("stockfish_pv_3", ""),
            "stockfish_rank_of_howl_move": rank,
        }
        print(
            f"[{pos_id}] finished in {elapsed:.1f}s | depth={search_res['completed_depth']} "
            f"best={best_move} (SF rank: {rank})",
            flush=True,
        )
        return idx, row

    with ThreadPoolExecutor(max_workers=concurrency) as executor:
        futures = [
            executor.submit(worker, idx, pos) for idx, pos in enumerate(positions)
        ]
        for f in as_completed(futures):
            idx, res_row = f.result()
            results_by_index[idx] = res_row

    ordered_results = [results_by_index[i] for i in range(len(positions))]

    with open(out_path, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(ordered_results)

    total_time = time.monotonic() - t_start
    print(f"\nAll {len(positions)} positions completed in {total_time:.1f}s.")
    print(f"Results written to: {out_path}")

    # Summary of Stockfish ranks
    rank_counts: Dict[str, int] = {}
    for r in ordered_results:
        rk = r["stockfish_rank_of_howl_move"]
        rank_counts[rk] = rank_counts.get(rk, 0) + 1
    print("Rank distribution of Howl moves:")
    for k in ["1", "2", "3", "outside_top_3"]:
        print(f"  {k}: {rank_counts.get(k, 0)}")

    return str(out_path)


def main() -> None:
    parser = argparse.ArgumentParser(description="Run Howl diagnostic position suite.")
    parser.add_argument(
        "--positions",
        default="diagnostics/positions.tsv",
        help="Path to TSV position file (default: diagnostics/positions.tsv)",
    )
    parser.add_argument(
        "--reference",
        default="diagnostics/reference.tsv",
        help="Path to Stockfish reference TSV (default: diagnostics/reference.tsv)",
    )
    parser.add_argument(
        "--engine",
        default="./build/howl",
        help="Path to engine binary (default: ./build/howl)",
    )
    parser.add_argument(
        "--movetime",
        type=int,
        default=20000,
        help="Movetime in milliseconds per position (default: 20000)",
    )
    parser.add_argument(
        "--nodes",
        "--budget",
        dest="budget",
        type=int,
        default=None,
        help="Fixed node budget per position (optional)",
    )
    parser.add_argument(
        "--concurrency",
        type=int,
        default=12,
        help="Maximum concurrent Howl instances (default: 12)",
    )
    parser.add_argument(
        "--output",
        default="diagnostics/results/howl_diagnostics_20s.tsv",
        help="Output TSV file path (default: diagnostics/results/howl_diagnostics_20s.tsv)",
    )

    args = parser.parse_args()

    run_diagnostics(
        positions_file=args.positions,
        engine_bin=args.engine,
        movetime_ms=args.movetime,
        nodes_budget=args.budget,
        concurrency=args.concurrency,
        reference_file=args.reference,
        output_file=args.output,
    )


if __name__ == "__main__":
    main()
