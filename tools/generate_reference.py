#!/usr/bin/env python3
"""Generate cached reference analysis using Stockfish via UCI."""

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


class StockfishUCI:
    def __init__(
        self,
        binary_path: str,
        threads: int = 12,
        hash_mb: int = 512,
        multipv: int = 3,
        timeout: float = 60.0,
    ):
        self.binary_path = binary_path
        self.threads = threads
        self.hash_mb = hash_mb
        self.multipv = multipv
        self.timeout = timeout
        self.proc: Optional[subprocess.Popen] = None
        self.confirmed_threads: Optional[int] = None
        self.confirmed_hash: Optional[int] = None
        self.confirmed_multipv: Optional[int] = None

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

        self.confirmed_hash = self.hash_mb
        self.confirmed_multipv = self.multipv

        # Check for confirmation lines before readyok
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

    def analyze_position(self, fen: str, movetime_ms: int = 10000) -> Dict[str, str]:
        if not self.proc or self.proc.stdout is None:
            raise RuntimeError("Stockfish process is not running")

        self._send("ucinewgame")
        self._send("isready")
        self._wait_for("readyok")

        self._send(f"position fen {fen}")
        self._send(f"go movetime {movetime_ms}")

        multipv_data: Dict[int, Dict[str, str]] = {
            1: {"move": "", "score_type": "", "score_value": "", "pv": "", "depth": ""},
            2: {"move": "", "score_type": "", "score_value": "", "pv": "", "depth": ""},
            3: {"move": "", "score_type": "", "score_value": "", "pv": "", "depth": ""},
        }
        overall_nodes = ""
        overall_depth = ""

        # Allow extra buffer beyond movetime_ms for safety
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
                curr_depth = ""
                curr_nodes = ""
                curr_mp = 1
                curr_score_type = ""
                curr_score_val = ""
                curr_pv = ""

                while i < len(tokens):
                    t = tokens[i]
                    if t == "depth" and i + 1 < len(tokens):
                        curr_depth = tokens[i + 1]
                        i += 2
                    elif t == "nodes" and i + 1 < len(tokens):
                        curr_nodes = tokens[i + 1]
                        i += 2
                    elif t == "multipv" and i + 1 < len(tokens):
                        try:
                            curr_mp = int(tokens[i + 1])
                        except ValueError:
                            curr_mp = 1
                        i += 2
                    elif t == "score" and i + 1 < len(tokens):
                        st = tokens[i + 1]
                        if st in ("cp", "mate") and i + 2 < len(tokens):
                            curr_score_type = st
                            curr_score_val = tokens[i + 2]
                            i += 3
                        else:
                            curr_score_type = st
                            i += 2
                    elif t == "pv":
                        pv_tokens = []
                        i += 1
                        while i < len(tokens) and tokens[i] not in KEYWORDS:
                            pv_tokens.append(tokens[i])
                            i += 1
                        curr_pv = " ".join(pv_tokens)
                    else:
                        i += 1

                if curr_nodes:
                    overall_nodes = curr_nodes
                if curr_depth:
                    overall_depth = curr_depth

                if 1 <= curr_mp <= 3:
                    if curr_depth:
                        multipv_data[curr_mp]["depth"] = curr_depth
                    if curr_score_type:
                        multipv_data[curr_mp]["score_type"] = curr_score_type
                    if curr_score_val:
                        multipv_data[curr_mp]["score_value"] = curr_score_val
                    if curr_pv:
                        multipv_data[curr_mp]["pv"] = curr_pv
                        multipv_data[curr_mp]["move"] = curr_pv.split()[0]

            elif line_str.startswith("bestmove"):
                break

        stockfish_depth = multipv_data[1]["depth"] or overall_depth
        stockfish_nodes = overall_nodes

        return {
            "stockfish_depth": stockfish_depth,
            "stockfish_nodes": stockfish_nodes,
            "stockfish_move_1": multipv_data[1]["move"],
            "stockfish_score_1_type": multipv_data[1]["score_type"],
            "stockfish_score_1_value": multipv_data[1]["score_value"],
            "stockfish_pv_1": multipv_data[1]["pv"],
            "stockfish_move_2": multipv_data[2]["move"],
            "stockfish_score_2_type": multipv_data[2]["score_type"],
            "stockfish_score_2_value": multipv_data[2]["score_value"],
            "stockfish_pv_2": multipv_data[2]["pv"],
            "stockfish_move_3": multipv_data[3]["move"],
            "stockfish_score_3_type": multipv_data[3]["score_type"],
            "stockfish_score_3_value": multipv_data[3]["score_value"],
            "stockfish_pv_3": multipv_data[3]["pv"],
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


def generate_reference(
    positions_file: str,
    stockfish_bin: str,
    output_file: str,
    threads: int = 12,
    hash_mb: int = 512,
    multipv: int = 3,
    movetime_ms: int = 10000,
) -> Tuple[str, bool, bool, bool, int, float, int]:
    pos_path = Path(positions_file)
    if not pos_path.exists():
        raise FileNotFoundError(f"Positions file not found: {positions_file}")

    positions: List[Dict[str, str]] = []
    with open(pos_path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            positions.append(row)

    if not positions:
        raise ValueError(f"No positions found in {positions_file}")

    out_path = Path(output_file)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    t_start = time.monotonic()

    sf = StockfishUCI(
        binary_path=stockfish_bin,
        threads=threads,
        hash_mb=hash_mb,
        multipv=multipv,
    )
    sf.start()

    fieldnames = [
        "id",
        "fen",
        "stockfish_depth",
        "stockfish_nodes",
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
    ]

    records: List[Dict[str, str]] = []
    completed_count = 0
    fewer_than_3_pvs_count = 0

    total_positions = len(positions)
    try:
        for idx, pos in enumerate(positions, 1):
            pos_id = pos.get("id", "")
            fen = pos.get("fen", "")

            pos_t0 = time.monotonic()
            analysis = sf.analyze_position(fen=fen, movetime_ms=movetime_ms)
            pos_elapsed = time.monotonic() - pos_t0

            has_pv1 = bool(analysis["stockfish_move_1"])
            has_pv2 = bool(analysis["stockfish_move_2"])
            has_pv3 = bool(analysis["stockfish_move_3"])
            if not (has_pv1 and has_pv2 and has_pv3):
                fewer_than_3_pvs_count += 1

            record = {
                "id": pos_id,
                "fen": fen,
                "stockfish_depth": analysis["stockfish_depth"],
                "stockfish_nodes": analysis["stockfish_nodes"],
                "stockfish_move_1": analysis["stockfish_move_1"],
                "stockfish_score_1_type": analysis["stockfish_score_1_type"],
                "stockfish_score_1_value": analysis["stockfish_score_1_value"],
                "stockfish_pv_1": analysis["stockfish_pv_1"],
                "stockfish_move_2": analysis["stockfish_move_2"],
                "stockfish_score_2_type": analysis["stockfish_score_2_type"],
                "stockfish_score_2_value": analysis["stockfish_score_2_value"],
                "stockfish_pv_2": analysis["stockfish_pv_2"],
                "stockfish_move_3": analysis["stockfish_move_3"],
                "stockfish_score_3_type": analysis["stockfish_score_3_type"],
                "stockfish_score_3_value": analysis["stockfish_score_3_value"],
                "stockfish_pv_3": analysis["stockfish_pv_3"],
            }
            records.append(record)
            completed_count += 1
            print(
                f"[{idx:02d}/{total_positions:02d}] {pos_id} analyzed in {pos_elapsed:.1f}s "
                f"(depth={analysis['stockfish_depth']}, nodes={analysis['stockfish_nodes']}, "
                f"best={analysis['stockfish_move_1']})",
                flush=True,
            )
    finally:
        sf.close()

    with open(out_path, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(records)

    total_elapsed = time.monotonic() - t_start
    used_expected_threads = (sf.confirmed_threads == threads)
    used_expected_hash = (sf.confirmed_hash == hash_mb)
    used_expected_multipv = (sf.confirmed_multipv == multipv)

    return (
        str(out_path),
        used_expected_threads,
        used_expected_hash,
        used_expected_multipv,
        completed_count,
        total_elapsed,
        fewer_than_3_pvs_count,
    )


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate Stockfish reference analysis.")
    parser.add_argument(
        "--positions",
        default="diagnostics/positions.tsv",
        help="Path to positions TSV (default: diagnostics/positions.tsv)",
    )
    parser.add_argument(
        "--stockfish",
        default=os.path.expanduser("~/Downloads/stockfish/stockfish-linux-x86-64-universal"),
        help="Path to Stockfish binary",
    )
    parser.add_argument(
        "--output",
        default="diagnostics/reference.tsv",
        help="Output reference TSV path (default: diagnostics/reference.tsv)",
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
    parser.add_argument(
        "--multipv",
        type=int,
        default=3,
        help="MultiPV for Stockfish (default: 3)",
    )
    parser.add_argument(
        "--movetime",
        type=int,
        default=10000,
        help="Movetime in milliseconds per position (default: 10000)",
    )

    args = parser.parse_args()

    (
        out_file,
        threads_ok,
        hash_ok,
        multipv_ok,
        count,
        total_elapsed,
        fewer_3_pv,
    ) = generate_reference(
        positions_file=args.positions,
        stockfish_bin=args.stockfish,
        output_file=args.output,
        threads=args.threads,
        hash_mb=args.hash_mb,
        multipv=args.multipv,
        movetime_ms=args.movetime,
    )

    print(f"Reference generation completed.")
    print(f"Positions completed: {count}")
    print(f"Stockfish used {args.threads} threads: {threads_ok}")
    print(f"Stockfish Hash set to {args.hash_mb} MB: {hash_ok}")
    print(f"Stockfish MultiPV set to {args.multipv}: {multipv_ok}")
    print(f"Total elapsed time: {total_elapsed:.2f}s ({total_elapsed/60:.2f}m)")
    print(f"Positions with fewer than 3 PVs: {fewer_3_pv}")
    print(f"Output saved to: {out_file}")


if __name__ == "__main__":
    main()
