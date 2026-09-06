#!/usr/bin/env python3
"""
Targeted Stockfish evaluation of positions where Howl's move was outside the top 3.
Evaluates both the cached Stockfish best move and Howl's chosen move in the same
Stockfish search using UCI searchmoves and MultiPV = 2.
"""

import argparse
import csv
import os
from pathlib import Path
import subprocess
import sys
import time
from typing import Dict, List, Optional, Tuple


KEYWORDS = {
    "depth", "seldepth", "time", "nodes", "pv", "multipv",
    "score", "currmove", "currmovenumber", "hashfull", "nps",
    "tbhits", "sbhits", "cpuload", "string", "refutation", "currline"
}


class TargetedStockfishUCI:
    def __init__(
        self,
        binary_path: str,
        threads: int = 12,
        hash_mb: int = 512,
        multipv: int = 2,
        timeout: float = 30.0,
    ):
        self.binary_path = binary_path
        self.threads = threads
        self.hash_mb = hash_mb
        self.multipv = multipv
        self.timeout = timeout
        self.proc: Optional[subprocess.Popen] = None
        self.confirmed_threads: Optional[int] = None

    def start(self) -> None:
        if not os.path.isfile(self.binary_path):
            raise FileNotFoundError(f"Stockfish binary not found: {self.binary_path}")
        if not os.access(self.binary_path, os.X_OK):
            raise PermissionError(f"Stockfish binary is not executable: {self.binary_path}")

        self.proc = subprocess.Popen(
            [self.binary_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )

        self._send("uci")
        self._wait_for("uciok")

        self._send(f"setoption name Threads value {self.threads}")
        self._send(f"setoption name Hash value {self.hash_mb}")
        self._send(f"setoption name MultiPV value {self.multipv}")
        self._send("isready")

        start_time = time.monotonic()
        while True:
            if time.monotonic() - start_time > self.timeout:
                raise TimeoutError("Timed out waiting for readyok after setting options")
            assert self.proc and self.proc.stdout is not None
            line = self.proc.stdout.readline()
            if not line:
                raise RuntimeError("Stockfish stdout closed unexpectedly during startup")
            line_str = line.strip()
            if "Using" in line_str and "threads" in line_str:
                tokens = line_str.split()
                for idx, tok in enumerate(tokens):
                    if tok == "Using" and idx + 1 < len(tokens):
                        try:
                            self.confirmed_threads = int(tokens[idx + 1])
                        except ValueError:
                            pass
            if line_str == "readyok":
                break

    def _send(self, command: str) -> None:
        if not self.proc or self.proc.stdin is None:
            raise RuntimeError("Stockfish process is not running")
        self.proc.stdin.write(f"{command}\n")
        self.proc.stdin.flush()

    def _wait_for(self, expected: str) -> None:
        if not self.proc or self.proc.stdout is None:
            raise RuntimeError("Stockfish process is not running")
        start_time = time.monotonic()
        while True:
            if time.monotonic() - start_time > self.timeout:
                raise TimeoutError(f"Timed out waiting for '{expected}' from Stockfish")
            line = self.proc.stdout.readline()
            if not line:
                raise RuntimeError("Stockfish stdout closed unexpectedly")
            if line.strip() == expected:
                break

    def analyze_two_moves(
        self,
        fen: str,
        move_a: str,
        move_b: str,
        movetime_ms: int = 10000,
    ) -> Dict[str, any]:
        if not self.proc or self.proc.stdout is None:
            raise RuntimeError("Stockfish process is not running")

        self._send("ucinewgame")
        self._send("isready")
        self._wait_for("readyok")

        self._send(f"position fen {fen}")
        self._send(f"go movetime {movetime_ms} searchmoves {move_a} {move_b}")

        multipv_by_move: Dict[str, Dict[str, str]] = {}
        overall_depth = ""
        overall_nodes = ""

        timeout_sec = (movetime_ms / 1000.0) + self.timeout
        start_time = time.monotonic()

        while True:
            if time.monotonic() - start_time > timeout_sec:
                raise TimeoutError(f"Timed out analyzing FEN: {fen}")

            line = self.proc.stdout.readline()
            if not line:
                raise RuntimeError("Stockfish closed stdout unexpectedly during analysis")

            line_str = line.strip()
            if line_str.startswith("info"):
                tokens = line_str.split()
                i = 0
                depth = ""
                nodes = ""
                mp = None
                score_type = ""
                score_val = ""
                pv_tokens: List[str] = []

                while i < len(tokens):
                    t = tokens[i]
                    if t == "depth" and i + 1 < len(tokens):
                        depth = tokens[i + 1]
                        i += 2
                    elif t == "nodes" and i + 1 < len(tokens):
                        nodes = tokens[i + 1]
                        i += 2
                    elif t == "multipv" and i + 1 < len(tokens):
                        try:
                            mp = int(tokens[i + 1])
                        except ValueError:
                            mp = None
                        i += 2
                    elif t == "score" and i + 1 < len(tokens):
                        st = tokens[i + 1]
                        if st in ("cp", "mate") and i + 2 < len(tokens):
                            score_type = st
                            score_val = tokens[i + 2]
                            i += 3
                        else:
                            score_type = st
                            i += 2
                    elif t == "pv":
                        i += 1
                        while i < len(tokens) and tokens[i] not in KEYWORDS:
                            pv_tokens.append(tokens[i])
                            i += 1
                    else:
                        i += 1

                if depth:
                    overall_depth = depth
                if nodes:
                    overall_nodes = nodes

                if mp is not None and pv_tokens:
                    root_move = pv_tokens[0]
                    multipv_by_move[root_move] = {
                        "score_type": score_type,
                        "score_value": score_val,
                        "pv": " ".join(pv_tokens),
                        "depth": depth,
                    }

            elif line_str.startswith("bestmove"):
                break

        return {
            "by_move": multipv_by_move,
            "depth": overall_depth,
            "nodes": overall_nodes,
        }

    def close(self) -> None:
        if self.proc:
            try:
                if self.proc.poll() is None:
                    self._send("quit")
                    try:
                        self.proc.wait(timeout=2.0)
                    except subprocess.TimeoutExpired:
                        self.proc.kill()
            except Exception:
                try:
                    self.proc.kill()
                except Exception:
                    pass
            finally:
                self.proc = None


def run_disagreement_analysis(
    input_file: str = "diagnostics/results/howl_diagnostics_20s.tsv",
    output_file: str = "diagnostics/results/stockfish_howl_disagreements.tsv",
    stockfish_bin: str = os.path.expanduser("~/Downloads/stockfish/stockfish-linux-x86-64-universal"),
    movetime_ms: int = 10000,
    threads: int = 12,
    hash_mb: int = 512,
) -> Tuple[str, int, float, List[Dict[str, str]], List[str]]:
    in_path = Path(input_file)
    if not in_path.exists():
        raise FileNotFoundError(f"Input diagnostic results not found: {input_file}")

    selected_positions: List[Dict[str, str]] = []
    with open(in_path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            if row.get("stockfish_rank_of_howl_move") == "outside_top_3":
                selected_positions.append(row)

    if not selected_positions:
        raise ValueError("No positions found with stockfish_rank_of_howl_move == 'outside_top_3'")

    out_path = Path(output_file)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    fieldnames = [
        "id",
        "fen",
        "howl_move",
        "cached_stockfish_best_move",
        "targeted_stockfish_best_move",
        "targeted_stockfish_best_score_type",
        "targeted_stockfish_best_score_value",
        "targeted_stockfish_best_pv",
        "targeted_howl_move",
        "targeted_howl_score_type",
        "targeted_howl_score_value",
        "targeted_howl_pv",
        "depth",
        "nodes",
    ]

    sf = TargetedStockfishUCI(
        binary_path=stockfish_bin,
        threads=threads,
        hash_mb=hash_mb,
        multipv=2,
    )
    sf.start()

    t_start = time.monotonic()
    records: List[Dict[str, str]] = []
    mate_scores: List[str] = []

    try:
        total = len(selected_positions)
        for idx, pos in enumerate(selected_positions, 1):
            pos_id = pos["id"]
            fen = pos["fen"]
            howl_move = pos["best_move"]
            sf_best = pos["stockfish_move_1"]

            t0 = time.monotonic()
            res = sf.analyze_two_moves(
                fen=fen,
                move_a=sf_best,
                move_b=howl_move,
                movetime_ms=movetime_ms,
            )
            elapsed = time.monotonic() - t0

            by_move = res["by_move"]
            sf_data = by_move.get(sf_best, {})
            howl_data = by_move.get(howl_move, {})

            sf_st = sf_data.get("score_type", "")
            sf_sv = sf_data.get("score_value", "")
            howl_st = howl_data.get("score_type", "")
            howl_sv = howl_data.get("score_value", "")

            if sf_st == "mate":
                mate_scores.append(f"{pos_id} (SF best move {sf_best}: mate {sf_sv})")
            if howl_st == "mate":
                mate_scores.append(f"{pos_id} (Howl move {howl_move}: mate {howl_sv})")

            rec = {
                "id": pos_id,
                "fen": fen,
                "howl_move": howl_move,
                "cached_stockfish_best_move": sf_best,
                "targeted_stockfish_best_move": sf_best,
                "targeted_stockfish_best_score_type": sf_st,
                "targeted_stockfish_best_score_value": sf_sv,
                "targeted_stockfish_best_pv": sf_data.get("pv", ""),
                "targeted_howl_move": howl_move,
                "targeted_howl_score_type": howl_st,
                "targeted_howl_score_value": howl_sv,
                "targeted_howl_pv": howl_data.get("pv", ""),
                "depth": res["depth"],
                "nodes": res["nodes"],
            }
            records.append(rec)
            print(
                f"[{idx:02d}/{total:02d}] {pos_id} analyzed in {elapsed:.1f}s | "
                f"SF {sf_best}: {sf_st} {sf_sv} vs Howl {howl_move}: {howl_st} {howl_sv}",
                flush=True,
            )
    finally:
        sf.close()

    with open(out_path, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(records)

    total_time = time.monotonic() - t_start
    return str(out_path), len(records), total_time, records, mate_scores


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Targeted Stockfish comparison of Howl disagreement moves."
    )
    parser.add_argument(
        "--input",
        default="diagnostics/results/howl_diagnostics_20s.tsv",
        help="Input diagnostic results TSV (default: diagnostics/results/howl_diagnostics_20s.tsv)",
    )
    parser.add_argument(
        "--output",
        default="diagnostics/results/stockfish_howl_disagreements.tsv",
        help="Output comparison TSV path (default: diagnostics/results/stockfish_howl_disagreements.tsv)",
    )
    parser.add_argument(
        "--stockfish",
        default=os.path.expanduser("~/Downloads/stockfish/stockfish-linux-x86-64-universal"),
        help="Path to Stockfish binary",
    )
    parser.add_argument(
        "--movetime",
        type=int,
        default=10000,
        help="Movetime in milliseconds per position (default: 10000)",
    )
    parser.add_argument(
        "--threads",
        type=int,
        default=12,
        help="Threads for Stockfish (default: 12)",
    )
    parser.add_argument(
        "--hash",
        dest="hash_mb",
        type=int,
        default=512,
        help="Hash in MB for Stockfish (default: 512)",
    )

    args = parser.parse_args()

    out_file, count, total_time, records, mate_scores = run_disagreement_analysis(
        input_file=args.input,
        output_file=args.output,
        stockfish_bin=args.stockfish,
        movetime_ms=args.movetime,
        threads=args.threads,
        hash_mb=args.hash_mb,
    )

    print(f"\nDisagreement analysis completed.")
    print(f"Analyzed {count} positions in {total_time:.1f}s.")
    print(f"Results saved to: {out_file}")


if __name__ == "__main__":
    main()
