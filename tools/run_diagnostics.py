#!/usr/bin/env python3
"""Runner for Howl chess engine diagnostic position suite."""

import argparse
import csv
from datetime import datetime
import os
from pathlib import Path
import subprocess
import sys
import time
from typing import Dict, List, Optional


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
            score_type = tokens[i + 1]
            if score_type in ("cp", "mate") and i + 2 < len(tokens):
                parsed["score"] = f"{score_type} {tokens[i + 2]}"
                i += 3
            else:
                parsed["score"] = tokens[i + 1]
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


class UCIEngine:
    def __init__(self, binary_path: str, timeout: float = 30.0):
        self.binary_path = binary_path
        self.timeout = timeout
        self.proc: Optional[subprocess.Popen] = None

    def start(self) -> None:
        if not os.path.isfile(self.binary_path):
            raise FileNotFoundError(f"Engine binary not found: {self.binary_path}")
        if not os.access(self.binary_path, os.X_OK):
            raise PermissionError(f"Engine binary is not executable: {self.binary_path}")

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
        self._send("isready")
        self._wait_for("readyok")

    def _send(self, command: str) -> None:
        if not self.proc or self.proc.stdin is None:
            raise RuntimeError("Engine process is not running")
        self.proc.stdin.write(f"{command}\n")
        self.proc.stdin.flush()

    def _wait_for(self, expected: str) -> None:
        if not self.proc or self.proc.stdout is None:
            raise RuntimeError("Engine process is not running")
        start_time = time.monotonic()
        while True:
            if time.monotonic() - start_time > self.timeout:
                raise TimeoutError(f"Timed out waiting for '{expected}' from engine")
            line = self.proc.stdout.readline()
            if not line:
                raise RuntimeError("Engine process closed stdout unexpectedly")
            if line.strip() == expected:
                break

    def search_position(self, fen: str, budget: int) -> Dict[str, str]:
        if not self.proc or self.proc.stdout is None:
            raise RuntimeError("Engine process is not running")

        self._send("ucinewgame")
        self._send("isready")
        self._wait_for("readyok")

        self._send(f"position fen {fen}")
        self._send(f"go nodes {budget}")

        completed_depth = ""
        nodes = ""
        score = ""
        pv = ""
        best_move = ""

        start_time = time.monotonic()
        while True:
            if time.monotonic() - start_time > self.timeout:
                raise TimeoutError(f"Timed out during search for FEN: {fen}")

            line = self.proc.stdout.readline()
            if not line:
                raise RuntimeError("Engine process closed stdout unexpectedly during search")

            line_str = line.strip()
            if line_str.startswith("info"):
                info = parse_info_line(line_str)
                if "depth" in info:
                    completed_depth = info["depth"]
                if "nodes" in info:
                    nodes = info["nodes"]
                if "score" in info:
                    score = info["score"]
                if "pv" in info:
                    pv = info["pv"]
            elif line_str.startswith("bestmove"):
                tokens = line_str.split()
                if len(tokens) >= 2:
                    best_move = tokens[1]
                else:
                    best_move = "(none)"
                break

        return {
            "completed_depth": completed_depth,
            "nodes": nodes,
            "score": score,
            "best_move": best_move,
            "pv": pv,
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


def run_diagnostics(
    positions_file: str,
    engine_bin: str,
    budget: int,
    output_file: Optional[str] = None,
    timeout: float = 30.0,
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

    if not output_file:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        results_dir = Path("diagnostics/results")
        results_dir.mkdir(parents=True, exist_ok=True)
        out_path = results_dir / f"results_{timestamp}.tsv"
    else:
        out_path = Path(output_file)
        out_path.parent.mkdir(parents=True, exist_ok=True)

    engine = UCIEngine(binary_path=engine_bin, timeout=timeout)
    engine.start()

    fieldnames = [
        "id",
        "fen",
        "category",
        "reference_move",
        "budget",
        "completed_depth",
        "nodes",
        "score",
        "best_move",
        "pv",
        "match",
    ]

    results: List[Dict[str, str]] = []
    try:
        for pos in positions:
            pos_id = pos.get("id", "")
            fen = pos.get("fen", "")
            category = pos.get("category", "")
            reference_move = pos.get("reference_move", "")

            search_result = engine.search_position(fen=fen, budget=budget)
            best_move = search_result["best_move"]
            is_match = "true" if best_move == reference_move else "false"

            record = {
                "id": pos_id,
                "fen": fen,
                "category": category,
                "reference_move": reference_move,
                "budget": str(budget),
                "completed_depth": search_result["completed_depth"],
                "nodes": search_result["nodes"],
                "score": search_result["score"],
                "best_move": best_move,
                "pv": search_result["pv"],
                "match": is_match,
            }
            results.append(record)
    finally:
        engine.close()

    with open(out_path, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(results)

    return str(out_path)


def main() -> None:
    parser = argparse.ArgumentParser(description="Run Howl diagnostic position suite.")
    parser.add_argument(
        "--positions",
        default="diagnostics/positions.tsv",
        help="Path to TSV position file (default: diagnostics/positions.tsv)",
    )
    parser.add_argument(
        "--engine",
        default="./build/howl",
        help="Path to engine binary (default: ./build/howl)",
    )
    parser.add_argument(
        "--nodes",
        "--budget",
        dest="budget",
        type=int,
        default=1000,
        help="Fixed node budget per position (default: 1000)",
    )
    parser.add_argument(
        "--output",
        default=None,
        help="Output TSV file path (default: diagnostics/results/results_<timestamp>.tsv)",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=30.0,
        help="Timeout in seconds per position (default: 30.0)",
    )

    args = parser.parse_args()

    out_file = run_diagnostics(
        positions_file=args.positions,
        engine_bin=args.engine,
        budget=args.budget,
        output_file=args.output,
        timeout=args.timeout,
    )

    print(f"Diagnostics completed successfully.")
    print(f"Results written to: {out_file}")


if __name__ == "__main__":
    main()
