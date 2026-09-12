#!/usr/bin/env python3
"""
Standalone SPSA Evaluator Tuning Runner for Howl UCI Chess Engine.

Features:
- Standard symmetric SPSA (Simultaneous Perturbation Stochastic Approximation).
- SPSA schedules: a_k = a / (A + k)^alpha, c_k = c / k^gamma.
- Individual parameter numerical scaling, bounds, and integer preservation.
- Automatic isolated engine building for plus and minus candidates per iteration.
- 8 concurrent games per iteration across 8 CPU cores with CPU affinity pinning.
- 4 fixed balanced openings with colors swapped (8 games total).
- Syzygy tablebase options (SyzygyPath and SyzygyProbeLimit).
- Comprehensive checkpointing, state serialization, and resume capability.
- Robust failure handling: reruns iteration on engine crash/timeout rather than using corrupted scores.
- Dry-run, smoke-test, and parameter-listing modes.
"""

from __future__ import annotations

import argparse
import collections
from concurrent.futures import ProcessPoolExecutor, as_completed
from dataclasses import asdict, dataclass, field
import datetime
import json
import os
import random
import re
import shutil
import subprocess
import sys
import threading
import time
from typing import Any, Dict, List, Optional, Tuple

import chess
import chess.pgn

# Four balanced, shallow, varied named opening positions (reused from self_match.py)
STARTING_POSITIONS: List[Tuple[str, str]] = [
    ("Italian Game", "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3"),
    ("Sicilian Defense Open", "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq - 1 5"),
    ("Queen's Gambit Declined", "rnbqkb1r/ppp2ppp/4pn2/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 2 4"),
    ("French Defense Advance", "rnbqkbnr/ppp2ppp/4p3/3pP3/3P4/8/PPP2PPP/RNBQKBNR b KQkq - 0 3"),
]


@dataclass
class SPSAConfig:
    # SPSA Hyperparameters
    a: float = 10.0          # Step size numerator
    c: float = 1.0           # Perturbation numerator
    A: float = 10.0          # Stability constant
    alpha: float = 0.602     # Step size exponent
    gamma: float = 0.101     # Perturbation exponent

    # Match / Execution Settings
    base_time_sec: float = 120.0
    inc_sec: float = 1.0
    concurrency: int = 8
    syzygy_path: str = "/home/masoud/syzygy/3-4-5-wdl"
    syzygy_probe_limit: int = 5

    # Directory settings
    state_dir: str = "spsa_state"
    build_dir: str = "build"
    repo_root: str = "."
    max_iterations: int = 1000
    base_seed: int = 42


@dataclass
class Parameter:
    name: str
    current_value: float
    min_value: float
    max_value: float
    scale: float = 1.0
    enabled: bool = True
    is_integer: bool = True

    def clamp(self, val: float) -> float:
        return max(self.min_value, min(self.max_value, val))

    def format_value(self, val: float) -> Any:
        if self.is_integer:
            return int(round(self.clamp(val)))
        return float(self.clamp(val))

    def materialize_candidates(self, base_val: float, c_k: float, d: int) -> Tuple[Any, Any, float]:
        """
        Generates symmetric or bounded (plus, minus) candidate values for the engine.
        For integer parameters, prefers symmetric perturbation around rounded base.
        Near hard bounds where symmetric perturbation cannot fit, materializes the closest feasible
        distinct integer candidate pair inside [min_value, max_value] oriented by d.
        Returns (plus, minus, effective_h) where effective_h = abs(plus - minus) / 2.0.
        """
        if not self.enabled or d == 0:
            val = self.format_value(base_val)
            return val, val, 0.0

        if self.is_integer:
            base_int = int(round(self.clamp(base_val)))
            min_i = int(self.min_value)
            max_i = int(self.max_value)
            if min_i >= max_i:
                return min_i, min_i, 0.0

            requested_mag = max(1, int(round(c_k * self.scale)))
            available_mag = min(base_int - min_i, max_i - base_int)
            actual_mag = min(requested_mag, available_mag)

            if actual_mag >= 1:
                plus = base_int + d * actual_mag
                minus = base_int - d * actual_mag
                effective_h = float(actual_mag)
            else:
                span = min(requested_mag, max_i - min_i)
                span = max(1, span)
                if base_int - min_i < max_i - base_int:
                    low_val = min_i
                    high_val = min_i + span
                else:
                    high_val = max_i
                    low_val = max_i - span

                plus = high_val if d > 0 else low_val
                minus = low_val if d > 0 else high_val
                effective_h = abs(plus - minus) / 2.0

            return plus, minus, effective_h
        else:
            requested_mag = c_k * self.scale
            min_f = float(self.min_value)
            max_f = float(self.max_value)
            if min_f >= max_f:
                return min_f, min_f, 0.0

            available_mag = min(base_val - min_f, max_f - base_val)
            actual_mag = min(requested_mag, available_mag)
            if actual_mag > 1e-9:
                plus = float(self.clamp(base_val + d * actual_mag))
                minus = float(self.clamp(base_val - d * actual_mag))
                effective_h = abs(plus - minus) / 2.0
            else:
                span = min(requested_mag, max_f - min_f)
                if base_val - min_f < max_f - base_val:
                    low_val = min_f
                    high_val = min_f + span
                else:
                    high_val = max_f
                    low_val = max_f - span
                plus = high_val if d > 0 else low_val
                minus = low_val if d > 0 else high_val
                effective_h = abs(plus - minus) / 2.0

            return plus, minus, effective_h


class EngineCrashError(Exception):
    def __init__(
        self,
        message: str,
        returncode: Optional[int],
        last_uci_command: str,
        last_bestmove: str,
        stderr_lines: List[str],
    ):
        super().__init__(message)
        self.message = message
        self.returncode = returncode
        self.last_uci_command = last_uci_command
        self.last_bestmove = last_bestmove
        self.stderr_lines = stderr_lines


class UCIEngineProcess:
    def __init__(self, path: str, options: Optional[Dict[str, Any]] = None):
        self.path = os.path.abspath(path)
        if not os.path.isfile(self.path):
            raise FileNotFoundError(f"Engine executable not found at: {self.path}")

        self.options = options or {}
        self.last_uci_command: str = ""
        self.last_bestmove: str = "(none)"
        self._stderr_buffer: collections.deque[str] = collections.deque(maxlen=20)
        self._stderr_lock = threading.Lock()

        self.process = subprocess.Popen(
            [self.path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )

        self._stderr_thread = threading.Thread(target=self._capture_stderr, daemon=True)
        self._stderr_thread.start()

        self._init_uci()

    def _capture_stderr(self) -> None:
        if self.process.stderr is None:
            return
        for line in self.process.stderr:
            clean_line = line.rstrip("\r\n")
            with self._stderr_lock:
                self._stderr_buffer.append(clean_line)

    def get_recent_stderr(self) -> List[str]:
        with self._stderr_lock:
            return list(self._stderr_buffer)

    def _raise_crash(self, msg: str) -> None:
        try:
            self.process.wait(timeout=0.2)
        except Exception:
            pass
        returncode = self.process.poll()
        stderr_lines = self.get_recent_stderr()
        raise EngineCrashError(
            message=msg,
            returncode=returncode,
            last_uci_command=self.last_uci_command,
            last_bestmove=self.last_bestmove,
            stderr_lines=stderr_lines,
        )

    def _send(self, command: str) -> None:
        if self.process.poll() is not None:
            self._raise_crash("Engine process terminated unexpectedly before sending command")
        self.last_uci_command = command
        assert self.process.stdin is not None
        try:
            self.process.stdin.write(f"{command}\n")
            self.process.stdin.flush()
        except BrokenPipeError:
            self._raise_crash("Broken pipe while writing command to engine stdin")

    def _read_line(self) -> str:
        assert self.process.stdout is not None
        line = self.process.stdout.readline()
        if not line:
            self._raise_crash("Engine standard output closed unexpectedly")
        return line.strip()

    def _init_uci(self) -> None:
        self._send("uci")
        while True:
            line = self._read_line()
            if line == "uciok":
                break

        for opt_name, opt_val in self.options.items():
            self._send(f"setoption name {opt_name} value {opt_val}")

        self._send("isready")
        while True:
            line = self._read_line()
            if line == "readyok":
                break

    def new_game(self) -> None:
        self._send("ucinewgame")
        self._send("isready")
        while True:
            line = self._read_line()
            if line == "readyok":
                break

    def get_move(
        self,
        starting_fen: str,
        moves: List[str],
        wtime_ms: int,
        btime_ms: int,
        winc_ms: int,
        binc_ms: int,
        timeout_sec: float,
    ) -> Optional[str]:
        if starting_fen == chess.STARTING_FEN:
            pos_cmd = "position startpos"
        else:
            pos_cmd = f"position fen {starting_fen}"

        if moves:
            pos_cmd += " moves " + " ".join(moves)

        self._send(pos_cmd)
        go_cmd = f"go wtime {max(1, wtime_ms)} btime {max(1, btime_ms)} winc {winc_ms} binc {binc_ms}"
        self._send(go_cmd)

        start_time = time.monotonic()
        while True:
            if time.monotonic() - start_time > timeout_sec:
                self.close()
                return None

            if self.process.poll() is not None:
                self._raise_crash("Engine process crashed during search")

            line = self._read_line()
            if line.startswith("bestmove"):
                tokens = line.split()
                if len(tokens) >= 2 and tokens[1] != "(none)":
                    self.last_bestmove = tokens[1]
                    return tokens[1]
                self.last_bestmove = "(none)"
                return None

    def close(self) -> None:
        try:
            if self.process.poll() is None:
                self._send("quit")
                try:
                    self.process.wait(timeout=1.0)
                except subprocess.TimeoutExpired:
                    self.process.kill()
        except Exception:
            try:
                self.process.kill()
            except Exception:
                pass


@dataclass
class SingleGameTask:
    game_index: int
    pos_name: str
    starting_fen: str
    white_role: str   # "plus" or "minus"
    black_role: str   # "minus" or "plus"
    plus_engine_path: str
    minus_engine_path: str
    base_time_sec: float
    inc_sec: float
    cpu_core: int
    uci_options: Dict[str, Any]


@dataclass
class SingleGameOutput:
    game_index: int
    pos_name: str
    starting_fen: str
    white_role: str
    black_role: str
    result: str  # "1-0", "0-1", "1/2-1/2"
    winner: Optional[str]  # "plus", "minus", or None
    termination: str
    plus_score: float
    minus_score: float
    pgn_str: str
    is_valid: bool
    error_message: Optional[str] = None


def _run_single_game_worker(task: SingleGameTask) -> SingleGameOutput:
    """Worker function executed in separate process with affinity pinned to a specific CPU core."""
    if hasattr(os, "sched_setaffinity"):
        try:
            os.sched_setaffinity(0, {task.cpu_core})
        except Exception as e:
            # Fallback or permission warning
            pass

    white_path = task.plus_engine_path if task.white_role == "plus" else task.minus_engine_path
    black_path = task.plus_engine_path if task.black_role == "plus" else task.minus_engine_path
    white_name = f"Howl-{task.white_role}"
    black_name = f"Howl-{task.black_role}"

    engine_procs = {}
    try:
        engine_procs[chess.WHITE] = UCIEngineProcess(white_path, task.uci_options)
    except Exception as e:
        return SingleGameOutput(
            game_index=task.game_index,
            pos_name=task.pos_name,
            starting_fen=task.starting_fen,
            white_role=task.white_role,
            black_role=task.black_role,
            result="0-1",
            winner=task.black_role,
            termination=f"White failed to launch: {e}",
            plus_score=0.0 if task.white_role == "plus" else 1.0,
            minus_score=1.0 if task.white_role == "plus" else 0.0,
            pgn_str="",
            is_valid=False,
            error_message=f"White failed to launch: {e}",
        )

    try:
        engine_procs[chess.BLACK] = UCIEngineProcess(black_path, task.uci_options)
    except Exception as e:
        engine_procs[chess.WHITE].close()
        return SingleGameOutput(
            game_index=task.game_index,
            pos_name=task.pos_name,
            starting_fen=task.starting_fen,
            white_role=task.white_role,
            black_role=task.black_role,
            result="1-0",
            winner=task.white_role,
            termination=f"Black failed to launch: {e}",
            plus_score=1.0 if task.white_role == "plus" else 0.0,
            minus_score=0.0 if task.white_role == "plus" else 1.0,
            pgn_str="",
            is_valid=False,
            error_message=f"Black failed to launch: {e}",
        )

    board = chess.Board(task.starting_fen)
    played_moves: List[str] = []

    wtime_ms = int(task.base_time_sec * 1000)
    btime_ms = int(task.base_time_sec * 1000)
    winc_ms = int(task.inc_sec * 1000)
    binc_ms = int(task.inc_sec * 1000)

    game_pgn = chess.pgn.Game()
    game_pgn.headers["Event"] = "Howl SPSA Tuning Match"
    game_pgn.headers["Site"] = f"CPU Core {task.cpu_core}"
    game_pgn.headers["Date"] = datetime.datetime.now().strftime("%Y.%m.%d")
    game_pgn.headers["Round"] = str(task.game_index + 1)
    game_pgn.headers["White"] = white_name
    game_pgn.headers["Black"] = black_name
    game_pgn.headers["SetUp"] = "1" if task.starting_fen != chess.STARTING_FEN else "0"
    if task.starting_fen != chess.STARTING_FEN:
        game_pgn.headers["FEN"] = task.starting_fen
    game_pgn.headers["Opening"] = task.pos_name

    current_node = game_pgn
    winner: Optional[str] = None
    result_str = "*"
    termination = ""
    is_valid = True
    error_msg: Optional[str] = None

    try:
        while not board.is_game_over(claim_draw=True):
            side = board.turn
            engine = engine_procs[side]
            role = task.white_role if side == chess.WHITE else task.black_role
            opp_role = task.black_role if side == chess.WHITE else task.white_role

            curr_clock_ms = wtime_ms if side == chess.WHITE else btime_ms
            timeout_sec = (curr_clock_ms / 1000.0) + 15.0

            t0 = time.monotonic()
            move_str = None
            try:
                move_str = engine.get_move(
                    starting_fen=task.starting_fen,
                    moves=played_moves,
                    wtime_ms=wtime_ms,
                    btime_ms=btime_ms,
                    winc_ms=winc_ms,
                    binc_ms=binc_ms,
                    timeout_sec=timeout_sec,
                )
            except EngineCrashError as e:
                result_str = "0-1" if side == chess.WHITE else "1-0"
                winner = opp_role
                termination = f"Engine crash ({role}): {e.message} (rc={e.returncode})"
                is_valid = False
                error_msg = termination
                break
            except Exception as e:
                result_str = "0-1" if side == chess.WHITE else "1-0"
                winner = opp_role
                termination = f"Engine error ({role}): {e}"
                is_valid = False
                error_msg = termination
                break

            elapsed_ms = int((time.monotonic() - t0) * 1000)
            if side == chess.WHITE:
                wtime_ms = max(0, wtime_ms - elapsed_ms + winc_ms)
            else:
                btime_ms = max(0, btime_ms - elapsed_ms + binc_ms)

            if move_str is None:
                result_str = "0-1" if side == chess.WHITE else "1-0"
                winner = opp_role
                termination = f"Timeout / No move returned ({role})"
                is_valid = False
                error_msg = termination
                break

            try:
                uci_move = chess.Move.from_uci(move_str)
            except ValueError:
                result_str = "0-1" if side == chess.WHITE else "1-0"
                winner = opp_role
                termination = f"Illegal move syntax '{move_str}' by {role}"
                is_valid = False
                error_msg = termination
                break

            if uci_move not in board.legal_moves:
                result_str = "0-1" if side == chess.WHITE else "1-0"
                winner = opp_role
                termination = f"Illegal move '{move_str}' played by {role}"
                is_valid = False
                error_msg = termination
                break

            current_node = current_node.add_variation(uci_move)
            board.push(uci_move)
            played_moves.append(move_str)

        if not termination:
            if board.is_checkmate():
                if board.turn == chess.WHITE:
                    result_str = "0-1"
                    winner = task.black_role
                    termination = f"Checkmate ({black_name} wins)"
                else:
                    result_str = "1-0"
                    winner = task.white_role
                    termination = f"Checkmate ({white_name} wins)"
            elif board.is_stalemate():
                result_str = "1/2-1/2"
                winner = None
                termination = "Stalemate"
            elif board.is_insufficient_material():
                result_str = "1/2-1/2"
                winner = None
                termination = "Draw by insufficient material"
            elif board.can_claim_threefold_repetition():
                result_str = "1/2-1/2"
                winner = None
                termination = "Draw by threefold repetition"
            elif board.can_claim_fifty_moves():
                result_str = "1/2-1/2"
                winner = None
                termination = "Draw by 50-move rule"
            else:
                result_str = "1/2-1/2"
                winner = None
                termination = "Draw"

    finally:
        for p in engine_procs.values():
            p.close()

    game_pgn.headers["Result"] = result_str
    game_pgn.headers["Termination"] = termination

    # Compute scores for plus and minus
    if result_str == "1-0":
        plus_sc = 1.0 if task.white_role == "plus" else 0.0
        minus_sc = 1.0 if task.white_role == "minus" else 0.0
    elif result_str == "0-1":
        plus_sc = 1.0 if task.black_role == "plus" else 0.0
        minus_sc = 1.0 if task.black_role == "minus" else 0.0
    else:
        plus_sc = 0.5
        minus_sc = 0.5

    exporter = chess.pgn.StringExporter(headers=True, variations=True, comments=False)
    pgn_text = game_pgn.accept(exporter)

    return SingleGameOutput(
        game_index=task.game_index,
        pos_name=task.pos_name,
        starting_fen=task.starting_fen,
        white_role=task.white_role,
        black_role=task.black_role,
        result=result_str,
        winner=winner,
        termination=termination,
        plus_score=plus_sc,
        minus_score=minus_sc,
        pgn_str=pgn_text,
        is_valid=is_valid,
        error_message=error_msg,
    )


# =============================================================================
# Parameter Discovery and Code Generation
# =============================================================================

def discover_parameters_from_option_cpp(option_cpp_path: str = "Option.cpp") -> Dict[str, Parameter]:
    """Extracts all tunable evaluation parameters from Option.cpp with sensible bounds and scales."""
    with open(option_cpp_path, "r", encoding="utf-8") as f:
        content = f.read()

    params: Dict[str, Parameter] = {}

    # 1. Piece Values
    piece_defaults = {
        "PawnValue": (140, 50, 300, 2.0),
        "KnightValue": (325, 150, 600, 2.0),
        "BishopValue": (365, 150, 600, 2.0),
        "RookValue": (515, 250, 900, 2.0),
        "QueenValue": (1020, 600, 1600, 5.0),
        "KingValue": (1200, 800, 2000, 5.0),
    }
    for name, (def_val, min_v, max_v, sc) in piece_defaults.items():
        m = re.search(rf'int Option::{name}\s*=\s*([0-9-]+);', content)
        cur = int(m.group(1)) if m else def_val
        params[name] = Parameter(name, cur, min_v, max_v, sc, enabled=True)

    # 2. Pawn Structure & End Pawn
    structure_defaults = {
        "DoubledPawnValue": (-20, -100, 0, 1.0),
        "EndPawnValue": (-20, -100, 100, 1.0),
        "IsolatedPawnMiddleGame": (-10, -60, 0, 1.0),
        "IsolatedPawnEndGame": (-8, -60, 0, 1.0),
    }
    for name, (def_val, min_v, max_v, sc) in structure_defaults.items():
        m = re.search(rf'int Option::{name}\s*=\s*([0-9-]+);', content)
        cur = int(m.group(1)) if m else def_val
        params[name] = Parameter(name, cur, min_v, max_v, sc, enabled=True)

    # 3. Rook & Knight Features
    feature_defaults = {
        "RookOpenFileMiddleGame": (18, 0, 60, 1.0),
        "RookOpenFileEndGame": (12, 0, 60, 1.0),
        "RookSemiOpenFileMiddleGame": (10, 0, 40, 1.0),
        "RookSemiOpenFileEndGame": (6, 0, 40, 1.0),
        "KnightOutpostMiddleGame": (12, 0, 60, 1.0),
        "KnightOutpostEndGame": (3, 0, 40, 1.0),
        "KnightSupportedOutpostMiddleGame": (8, 0, 40, 1.0),
        "KnightSupportedOutpostEndGame": (2, 0, 30, 1.0),
        "RookBehindPassedPawnMiddleGame": (4, 0, 40, 1.0),
        "RookBehindPassedPawnEndGame": (12, 0, 60, 1.0),
        "PassedPawnMiddleGameFileAmplitude": (7, 0, 30, 1.0),
    }
    for name, (def_val, min_v, max_v, sc) in feature_defaults.items():
        m = re.search(rf'int Option::{name}\s*=\s*([0-9-]+);', content)
        cur = int(m.group(1)) if m else def_val
        params[name] = Parameter(name, cur, min_v, max_v, sc, enabled=True)

    # Helper for array extraction
    def extract_array(arr_name: str, expected_size: int, default_min: int, default_max: int, scale: float = 1.0):
        pat = rf'int Option::{arr_name}(?:\[[0-9]*\])?\s*=\s*\{{([^}}]+)\}};'
        m = re.search(pat, content, re.MULTILINE | re.DOTALL)
        if not m:
            return
        tokens = [t.strip() for t in m.group(1).split(',')]
        nums = []
        for t in tokens:
            if t:
                try:
                    nums.append(int(t))
                except ValueError:
                    pass
        for i, val in enumerate(nums[:expected_size]):
            pname = f"{arr_name}_{i}"
            low = min(default_min, val - 40)
            high = max(default_max, val + 40)
            params[pname] = Parameter(pname, val, low, high, scale, enabled=True)

    # 4. Passed Pawn Parameters
    extract_array("PassedPawnMiddleGameParameters", 6, 0, 50, 1.0)
    extract_array("PassedPawnEndGameParameters", 6, 0, 100, 1.0)

    # 5. Mobility Parameters
    extract_array("KnightMobilityMiddleGameParameters", 4, -50, 50, 1.0)
    extract_array("KnightMobilityEndGameParameters", 4, -50, 50, 1.0)
    extract_array("BishopMobilityMiddleGameParameters", 5, -50, 50, 1.0)
    extract_array("BishopMobilityEndGameParameters", 5, -50, 50, 1.0)
    extract_array("RookMobilityMiddleGameParameters", 5, -50, 50, 1.0)
    extract_array("RookMobilityEndGameParameters", 5, -50, 50, 1.0)
    extract_array("QueenMobilityMiddleGameParameters", 5, -50, 50, 1.0)
    extract_array("QueenMobilityEndGameParameters", 5, -50, 50, 1.0)

    # 6. Piece Square Tables (White MG and EG)
    for piece in ["Pawn", "Knight", "Bishop", "Rook", "Queen", "King"]:
        extract_array(f"{piece}InValueWhiteMiddleGame", 64, -150, 150, 1.0)
        extract_array(f"{piece}InValueWhiteEndGame", 64, -150, 150, 1.0)

    # 7. Center Tables
    for piece in ["Pawn", "Knight", "Bishop", "Rook", "Queen", "King"]:
        extract_array(f"{piece}InCenterValueWhite", 64, -100, 100, 1.0)
        extract_array(f"{piece}MoveCenterValueWhite", 64, -100, 100, 1.0)

    # 8. King Safety
    extract_array("WhiteKingPlaceSafetyMiddleGame", 64, -100, 100, 1.0)
    extract_array("WhiteKingPlacePawnShieldMiddleGame", 64, -100, 100, 1.0)

    # 9. Attack Tables
    for piece in ["Pawn", "Knight", "Bishop", "Rook", "Queen"]:
        extract_array(f"{piece}AttackValueMiddleGame", 16, -50, 200, 1.0)
        extract_array(f"{piece}AttackValueEndGame", 16, -50, 200, 1.0)

    return params


def generate_option_cpp(overrides: Dict[str, float], base_source_path: str = "Option.cpp") -> str:
    """Produces customized Option.cpp source code reflecting parameter values."""
    with open(base_source_path, "r", encoding="utf-8") as f:
        res = f.read()

    # 1. Scalar replacements
    for name, val in overrides.items():
        if "_" not in name:
            pattern = rf'(int Option::{name}\s*=\s*)[0-9-]+;'
            if re.search(pattern, res):
                res = re.sub(pattern, rf'\g<1>{int(round(val))};', res)

    # 2. Array replacements
    arr_overrides: Dict[str, Dict[int, int]] = {}
    for name, val in overrides.items():
        if "_" in name:
            arr_name, idx_str = name.rsplit("_", 1)
            try:
                idx = int(idx_str)
                if arr_name not in arr_overrides:
                    arr_overrides[arr_name] = {}
                arr_overrides[arr_name][idx] = int(round(val))
            except ValueError:
                pass

    for arr_name, elem_dict in arr_overrides.items():
        pat = rf'(int Option::{arr_name}(?:\[[0-9]*\])?\s*=\s*\{{)([^}}]+)(\}};)'
        m = re.search(pat, res, re.MULTILINE | re.DOTALL)
        if m:
            prefix, body, suffix = m.group(1), m.group(2), m.group(3)
            tokens = [t.strip() for t in body.split(',')]
            numbers = []
            for t in tokens:
                if t:
                    try:
                        numbers.append(int(t))
                    except ValueError:
                        pass
            for idx, val in elem_dict.items():
                if 0 <= idx < len(numbers):
                    numbers[idx] = val

            if len(numbers) == 64:
                rows = []
                for r in range(8):
                    row_nums = numbers[r * 8:(r + 1) * 8]
                    rows.append(', '.join(f'{x:4d}' for x in row_nums))
                new_body = '\n  ' + ',\n  '.join(rows) + '\n'
            else:
                new_body = ', '.join(str(x) for x in numbers)

            res = res[:m.start()] + prefix + new_body + suffix + res[m.end():]

    return res


def build_engine_variant(
    overrides: Dict[str, float],
    out_binary_path: str,
    work_dir: str,
    variant_name: str,
    repo_root: str = ".",
    build_dir: str = "build",
) -> str:
    """Compiles Option.cpp with custom parameter overrides and links an isolated Howl executable."""
    os.makedirs(work_dir, exist_ok=True)
    out_binary_path = os.path.abspath(out_binary_path)
    base_option_path = os.path.join(repo_root, "Option.cpp")
    gen_cpp_path = os.path.join(work_dir, f"Option_{variant_name}.cpp")
    gen_obj_path = os.path.join(work_dir, f"Option_{variant_name}.o")

    # Generate source
    src = generate_option_cpp(overrides, base_source_path=base_option_path)
    with open(gen_cpp_path, "w", encoding="utf-8") as f:
        f.write(src)

    # Compile Option.cpp
    fathom_include = os.path.abspath(os.path.join(repo_root, "third_party/fathom/src"))
    compile_cmd = [
        "c++", "-O3", "-DNDEBUG", "-std=gnu++17",
        f"-I{fathom_include}",
        f"-I{os.path.abspath(repo_root)}",
        "-c", gen_cpp_path,
        "-o", gen_obj_path,
    ]
    subprocess.check_call(compile_cmd)

    # Collect existing precompiled object files
    objs_dir = os.path.join(repo_root, build_dir, "CMakeFiles/howl.dir")
    if not os.path.isdir(objs_dir):
        # Trigger cmake build to generate baseline objects
        subprocess.check_call(["cmake", "--build", os.path.join(repo_root, build_dir), "--target", "howl", "-j8"])

    existing_objs = [
        os.path.join(objs_dir, f)
        for f in os.listdir(objs_dir)
        if f.endswith(".o") and f != "Option.cpp.o"
    ]
    fathom_lib = os.path.join(repo_root, build_dir, "libfathom.a")

    link_cmd = [
        "c++", "-O3", "-DNDEBUG",
        *existing_objs,
        gen_obj_path,
        fathom_lib,
        "-o", out_binary_path,
    ]
    subprocess.check_call(link_cmd)

    return out_binary_path


def _atomic_write_json(filepath: str, data: Any, indent: int = 2) -> None:
    """Atomically writes JSON data to a file using a temporary file and os.replace."""
    dirname = os.path.dirname(os.path.abspath(filepath))
    os.makedirs(dirname, exist_ok=True)
    tmp_path = f"{filepath}.tmp.{os.getpid()}"
    try:
        with open(tmp_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=indent)
            f.flush()
            os.fsync(f.fileno())
        os.replace(tmp_path, filepath)
    except Exception:
        if os.path.exists(tmp_path):
            try:
                os.remove(tmp_path)
            except Exception:
                pass
        raise


def _atomic_write_text(filepath: str, text: str) -> None:
    """Atomically writes text data to a file using a temporary file and os.replace."""
    dirname = os.path.dirname(os.path.abspath(filepath))
    os.makedirs(dirname, exist_ok=True)
    tmp_path = f"{filepath}.tmp.{os.getpid()}"
    try:
        with open(tmp_path, "w", encoding="utf-8") as f:
            f.write(text)
            f.flush()
            os.fsync(f.fileno())
        os.replace(tmp_path, filepath)
    except Exception:
        if os.path.exists(tmp_path):
            try:
                os.remove(tmp_path)
            except Exception:
                pass
        raise


def _format_duration(seconds: float) -> str:
    """Formats seconds into concise string (e.g. 1h00m, 12m34s, 45.2s)."""
    total_sec = int(round(seconds))
    h = total_sec // 3600
    m = (total_sec % 3600) // 60
    s = total_sec % 60
    if h > 0:
        return f"{h}h{m:02d}m"
    elif m > 0:
        return f"{m}m{s:02d}s"
    else:
        return f"{seconds:.1f}s"


# =============================================================================
# SPSA Tuning Runner
# =============================================================================

class SPSATuner:
    def __init__(self, config: SPSAConfig, params_file: Optional[str] = None):
        self.config = config
        self.state_dir = os.path.abspath(config.state_dir)
        os.makedirs(self.state_dir, exist_ok=True)
        os.makedirs(os.path.join(self.state_dir, "iterations"), exist_ok=True)
        os.makedirs(os.path.join(self.state_dir, "pgn"), exist_ok=True)
        os.makedirs(os.path.join(self.state_dir, "builds"), exist_ok=True)

        self.checkpoint_file = os.path.join(self.state_dir, "spsa_checkpoint.json")
        self.params_file = params_file or os.path.join(self.state_dir, "spsa_parameters.json")

        # Baseline parameters from Option.cpp for from-base L1 calculation
        baseline_params = discover_parameters_from_option_cpp(os.path.join(config.repo_root, "Option.cpp"))
        self.baseline_theta: Dict[str, float] = {p.name: float(p.current_value) for p in baseline_params.values()}
        self.rolling_history: List[Dict[str, Any]] = []

        # Load or discover parameters
        if os.path.isfile(self.params_file):
            self.params = self._load_params(self.params_file)
        else:
            self.params = discover_parameters_from_option_cpp(os.path.join(config.repo_root, "Option.cpp"))
            self._save_params(self.params, self.params_file)

        self.theta: Dict[str, float] = {p.name: float(p.current_value) for p in self.params.values()}
        self.current_iteration = 1
        self.completed_iterations = 0

        # Check for existing checkpoint to resume
        if os.path.isfile(self.checkpoint_file):
            self._load_checkpoint()

    def _save_params(self, params: Dict[str, Parameter], path: str) -> None:
        data = {name: asdict(p) for name, p in params.items()}
        _atomic_write_json(path, data, indent=2)

    def _load_params(self, path: str) -> Dict[str, Parameter]:
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
        params = {}
        for name, d in data.items():
            params[name] = Parameter(**d)
        return params

    def _save_checkpoint(self, completed_iteration: int) -> None:
        self.completed_iterations = completed_iteration
        self.current_iteration = completed_iteration + 1
        ckpt = {
            "current_iteration": completed_iteration + 1,
            "completed_iterations": completed_iteration,
            "theta": self.theta,
            "config": asdict(self.config),
            "last_updated": datetime.datetime.now().isoformat(),
        }
        _atomic_write_json(self.checkpoint_file, ckpt, indent=2)

    def _load_checkpoint(self) -> None:
        with open(self.checkpoint_file, "r", encoding="utf-8") as f:
            ckpt = json.load(f)
        self.current_iteration = ckpt.get("current_iteration", 1)
        self.completed_iterations = ckpt.get("completed_iterations", self.current_iteration - 1 if self.current_iteration > 1 else 0)
        saved_theta = ckpt.get("theta", {})
        for k, v in saved_theta.items():
            if k in self.params:
                self.theta[k] = float(v)
                self.params[k].current_value = float(v)
        print(f"[SPSA] Resumed from checkpoint at iteration {self.current_iteration}")

    def build_checkpoint_candidate(self, out_binary_path: str) -> str:
        """Builds a standalone Howl binary from the current checkpoint theta without running SPSA or games."""
        theta_materialized = {name: param.format_value(self.theta[name]) for name, param in self.params.items()}
        work_dir = os.path.join(self.state_dir, "builds", "checkpoint_candidate")
        os.makedirs(work_dir, exist_ok=True)
        abs_out_path = os.path.abspath(out_binary_path)
        os.makedirs(os.path.dirname(abs_out_path), exist_ok=True)

        build_engine_variant(
            theta_materialized,
            abs_out_path,
            work_dir,
            "checkpoint_candidate",
            self.config.repo_root,
            self.config.build_dir,
        )
        try:
            shutil.rmtree(work_dir)
        except Exception:
            pass

        print(f"[SPSA] Checkpoint completed iterations: {self.completed_iterations}")
        print(f"[SPSA] Parameter count: {len(theta_materialized)}")
        print(f"[SPSA] Output binary path: {abs_out_path}")
        return abs_out_path

    def generate_perturbation(self, iteration_k: int, rng: random.Random) -> Tuple[Dict[str, int], float, float, Dict[str, float], Dict[str, float], Dict[str, float]]:
        """Generates random Rademacher perturbation and returns Delta, a_k, c_k, theta_plus, theta_minus, effective_h."""
        a_k = self.config.a / ((self.config.A + iteration_k) ** self.config.alpha)
        c_k = self.config.c / (iteration_k ** self.config.gamma)

        delta: Dict[str, int] = {}
        theta_plus: Dict[str, float] = {}
        theta_minus: Dict[str, float] = {}
        effective_h: Dict[str, float] = {}

        for name, param in self.params.items():
            if param.enabled:
                d = rng.choice([-1, 1])
                delta[name] = d
                p_plus, p_minus, eff_h = param.materialize_candidates(self.theta[name], c_k, d)
                theta_plus[name] = p_plus
                theta_minus[name] = p_minus
                effective_h[name] = eff_h
            else:
                delta[name] = 0
                val = param.format_value(self.theta[name])
                theta_plus[name] = val
                theta_minus[name] = val
                effective_h[name] = 0.0

        return delta, a_k, c_k, theta_plus, theta_minus, effective_h

    def run_iteration(self, iteration_k: int) -> bool:
        """Executes a single SPSA iteration (build, 8 concurrent games, SPSA update, checkpoint)."""
        t_iter_start = time.time()
        print(f"\n=======================================================")
        print(f" SPSA Iteration {iteration_k} / {self.config.max_iterations}")
        print(f"=======================================================")

        iter_seed = (self.config.base_seed + iteration_k * 10007) % (2**31 - 1)
        rng = random.Random(iter_seed)

        delta, a_k, c_k, theta_plus, theta_minus, effective_h = self.generate_perturbation(iteration_k, rng)
        enabled_count = sum(1 for p in self.params.values() if p.enabled)
        print(f"Enabled parameters: {enabled_count} | Learning rate a_k: {a_k:.6f} | Scale c_k: {c_k:.6f}")

        iter_work_dir = os.path.join(self.state_dir, "builds", f"iter_{iteration_k:05d}")
        os.makedirs(iter_work_dir, exist_ok=True)
        plus_binary = os.path.join(iter_work_dir, "howl_plus")
        minus_binary = os.path.join(iter_work_dir, "howl_minus")

        print("Building plus candidate engine...")
        build_engine_variant(theta_plus, plus_binary, iter_work_dir, "plus", self.config.repo_root, self.config.build_dir)
        print("Building minus candidate engine...")
        build_engine_variant(theta_minus, minus_binary, iter_work_dir, "minus", self.config.repo_root, self.config.build_dir)

        # Construct 8 game tasks (4 openings x 2 colors)
        tasks: List[SingleGameTask] = []
        uci_options = {
            "SyzygyPath": self.config.syzygy_path,
            "SyzygyProbeLimit": self.config.syzygy_probe_limit,
        }

        for op_idx, (pos_name, fen) in enumerate(STARTING_POSITIONS):
            # Game 1 for opening: Plus White vs Minus Black
            game_idx_1 = op_idx * 2
            core_1 = game_idx_1 % self.config.concurrency
            tasks.append(SingleGameTask(
                game_index=game_idx_1,
                pos_name=pos_name,
                starting_fen=fen,
                white_role="plus",
                black_role="minus",
                plus_engine_path=plus_binary,
                minus_engine_path=minus_binary,
                base_time_sec=self.config.base_time_sec,
                inc_sec=self.config.inc_sec,
                cpu_core=core_1,
                uci_options=uci_options,
            ))

            # Game 2 for opening: Minus White vs Plus Black
            game_idx_2 = op_idx * 2 + 1
            core_2 = game_idx_2 % self.config.concurrency
            tasks.append(SingleGameTask(
                game_index=game_idx_2,
                pos_name=pos_name,
                starting_fen=fen,
                white_role="minus",
                black_role="plus",
                plus_engine_path=plus_binary,
                minus_engine_path=minus_binary,
                base_time_sec=self.config.base_time_sec,
                inc_sec=self.config.inc_sec,
                cpu_core=core_2,
                uci_options=uci_options,
            ))

        print(f"Launching {len(tasks)} games concurrently on {self.config.concurrency} CPU cores (Time control: {self.config.base_time_sec}s + {self.config.inc_sec}s)...")
        results: List[SingleGameOutput] = [None] * len(tasks)  # type: ignore

        t0 = time.time()
        executor = ProcessPoolExecutor(max_workers=self.config.concurrency)
        try:
            future_to_idx = {executor.submit(_run_single_game_worker, task): task.game_index for task in tasks}
            for future in as_completed(future_to_idx):
                idx = future_to_idx[future]
                res = future.result()
                results[idx] = res
                print(f"  [Game {res.game_index + 1}/8] {res.pos_name} ({res.white_role} vs {res.black_role}): {res.result} ({res.termination})")
        except KeyboardInterrupt:
            print("\n[SPSA] Iteration interrupted by user (Ctrl+C). Terminating game workers...")
            executor.shutdown(wait=False, cancel_futures=True)
            try:
                shutil.rmtree(iter_work_dir, ignore_errors=True)
            except Exception:
                pass
            raise
        finally:
            executor.shutdown(wait=False, cancel_futures=True)

        total_elapsed = time.time() - t0
        print(f"All 8 games finished in {total_elapsed:.1f}s")

        # Check validity
        any_invalid = any(not r.is_valid for r in results)
        if any_invalid:
            print(f"[WARNING] Iteration {iteration_k} encountered infrastructure/engine failure! Rerunning iteration.")
            # Save failed log atomically
            failed_log = os.path.join(self.state_dir, "iterations", f"iteration_{iteration_k:05d}_failed.json")
            _atomic_write_json(failed_log, {
                "iteration": iteration_k,
                "status": "FAILED",
                "error": [r.error_message for r in results if not r.is_valid],
            }, indent=2)
            return False

        # Aggregate score
        total_plus_score = sum(r.plus_score for r in results)
        total_minus_score = sum(r.minus_score for r in results)
        score_diff = (total_plus_score - total_minus_score) / 8.0

        # Compute SPSA gradient and parameter update in normalized parameter coordinates
        prev_theta = dict(self.theta)
        theta_updated: Dict[str, float] = {}
        for name, param in self.params.items():
            eff_h = effective_h.get(name, 0.0)
            d = delta.get(name, 0)
            if param.enabled and d != 0 and eff_h > 0.0:
                # Normalized perturbation distance: norm_h = effective_h / scale
                norm_h = eff_h / param.scale
                # Gradient in normalized coordinates: g_norm = (score_diff * d) / (2 * norm_h)
                g_norm = (score_diff * d) / (2.0 * norm_h)
                # Raw theta step: theta_step = a_k * scale * g_norm
                step = a_k * param.scale * g_norm
                new_val = param.clamp(self.theta[name] + step)
                self.theta[name] = new_val
                param.current_value = new_val
                theta_updated[name] = new_val
            else:
                theta_updated[name] = self.theta[name]

        # Save Combined PGN atomically
        pgn_file = os.path.join(self.state_dir, "pgn", f"iteration_{iteration_k:05d}.pgn")
        combined_pgn = "\n\n".join(r.pgn_str for r in results) + "\n\n"
        _atomic_write_text(pgn_file, combined_pgn)

        # Save Iteration Details atomically
        iter_log = os.path.join(self.state_dir, "iterations", f"iteration_{iteration_k:05d}.json")
        iter_data = {
            "iteration": iteration_k,
            "status": "COMPLETED",
            "random_seed": iter_seed,
            "learning_rate_ak": a_k,
            "perturbation_scale_ck": c_k,
            "plus_score": total_plus_score,
            "minus_score": total_minus_score,
            "score_difference": score_diff,
            "delta": delta,
            "effective_h": effective_h,
            "theta_base": {k: float(v) for k, v in self.theta.items()},
            "theta_plus": theta_plus,
            "theta_minus": theta_minus,
            "theta_updated": theta_updated,
            "pgn_path": pgn_file,
            "games": [
                {
                    "game_index": r.game_index,
                    "pos_name": r.pos_name,
                    "white": r.white_role,
                    "black": r.black_role,
                    "result": r.result,
                    "termination": r.termination,
                }
                for r in results
            ],
        }
        _atomic_write_json(iter_log, iter_data, indent=2)

        # Save Checkpoint and Parameter files atomically
        self._save_checkpoint(iteration_k)
        self._save_params(self.params, self.params_file)

        # Clean up binary directory to conserve disk space
        try:
            shutil.rmtree(iter_work_dir)
        except Exception:
            pass

        # Progress calculation
        iter_duration = time.time() - t_iter_start
        moved_count = sum(1 for name, p in self.params.items() if p.enabled and abs(theta_updated[name] - prev_theta[name]) > 1e-9)
        bounds_blocked_count = sum(1 for name, p in self.params.items() if p.enabled and effective_h.get(name, 0.0) == 0.0)
        dL1 = sum(abs(theta_updated[name] - prev_theta[name]) for name, p in self.params.items() if p.enabled)

        # Single compact progress line
        print(
            f"[SPSA {iteration_k}/{self.config.max_iterations}] "
            f"Plus {total_plus_score:.1f} Minus {total_minus_score:.1f} | "
            f"diff {score_diff:+.3f} | "
            f"a={a_k:.3f} c={c_k:.3f} | "
            f"moved {moved_count}/{enabled_count} | "
            f"bounds {bounds_blocked_count} | "
            f"theta dL1={dL1:.1f} | "
            f"{iter_duration:.1f}s"
        )

        # Track 10-iteration rolling statistics
        self.rolling_history.append({
            "iteration": iteration_k,
            "plus_score": total_plus_score,
            "minus_score": total_minus_score,
            "score_diff": score_diff,
            "dL1": dL1,
            "bounds_blocked": bounds_blocked_count,
            "duration": iter_duration,
        })

        if iteration_k % 10 == 0 or len(self.rolling_history) >= 10:
            start_it = self.rolling_history[0]["iteration"]
            end_it = self.rolling_history[-1]["iteration"]
            total_pts = len(self.rolling_history) * 8
            cum_plus = sum(x["plus_score"] for x in self.rolling_history)
            mean_diff = sum(x["score_diff"] for x in self.rolling_history) / len(self.rolling_history)
            pos_cnt = sum(1 for x in self.rolling_history if x["score_diff"] > 1e-6)
            neg_cnt = sum(1 for x in self.rolling_history if x["score_diff"] < -1e-6)
            zero_cnt = sum(1 for x in self.rolling_history if abs(x["score_diff"]) <= 1e-6)
            mean_dL1 = sum(x["dL1"] for x in self.rolling_history) / len(self.rolling_history)
            tot_bounds = sum(x["bounds_blocked"] for x in self.rolling_history)
            from_base_L1 = sum(abs(self.theta[name] - self.baseline_theta.get(name, self.theta[name])) for name in self.params)
            tot_duration = sum(x["duration"] for x in self.rolling_history)

            dur_str = _format_duration(tot_duration)
            print(
                f"[SPSA summary {start_it}-{end_it}] "
                f"Plus {cum_plus:.1f}/{total_pts} | "
                f"mean diff {mean_diff:+.3f} | "
                f"dirs +{pos_cnt} -{neg_cnt} ={zero_cnt} | "
                f"mean dL1 {mean_dL1:.1f} | "
                f"bound blocks {tot_bounds} | "
                f"from-base L1 {from_base_L1:.1f} | "
                f"{dur_str}"
            )
            self.rolling_history.clear()

        return True

    def run(self) -> None:
        """Runs the continuous tuning loop."""
        print(f"[SPSA] Starting SPSA tuning loop from iteration {self.current_iteration} to {self.config.max_iterations}...")
        k = self.current_iteration
        try:
            while k <= self.config.max_iterations:
                success = self.run_iteration(k)
                if success:
                    k += 1
                else:
                    print(f"[SPSA] Retrying iteration {k} after transient failure...")
                    time.sleep(1.0)
        except KeyboardInterrupt:
            print(f"\n[SPSA] Tuning safely stopped by user (Ctrl+C).")
            print(f"[SPSA] Last valid checkpoint preserved at iteration {self.completed_iterations}.")
            print(f"[SPSA] You can resume at any time by rerunning tools/spsa_tune.py.")


# =============================================================================
# Validation and Unit Tests
# =============================================================================

def run_perturbation_unit_tests() -> None:
    """Deterministic validation test covering symmetric, bounded, and gradient updates."""
    # 1. Normal symmetric interior case
    p_int = Parameter(name="TestInterior", current_value=15.0, min_value=0.0, max_value=30.0, scale=1.0, is_integer=True)
    plus_p, minus_p, h_p = p_int.materialize_candidates(15.0, c_k=1.0, d=+1)
    plus_m, minus_m, h_m = p_int.materialize_candidates(15.0, c_k=1.0, d=-1)
    assert plus_p == 16 and minus_p == 14 and h_p == 1.0, f"Interior d=+1 failed: ({plus_p}, {minus_p}, {h_p})"
    assert plus_m == 14 and minus_m == 16 and h_m == 1.0, f"Interior d=-1 failed: ({plus_m}, {minus_m}, {h_m})"
    assert plus_p - minus_p == 2 * (+1) * h_p
    assert plus_m - minus_m == 2 * (-1) * h_m

    # 2. Lower-bound case around theta = 0.473
    p_low = Parameter(name="TestLower", current_value=0.473, min_value=0.0, max_value=30.0, scale=1.0, is_integer=True)
    p_low_plus, p_low_minus, h_low_p = p_low.materialize_candidates(0.473, c_k=1.0, d=+1)
    m_low_plus, m_low_minus, h_low_m = p_low.materialize_candidates(0.473, c_k=1.0, d=-1)
    assert p_low_plus == 1 and p_low_minus == 0 and h_low_p == 0.5, f"Lower bound d=+1 failed: ({p_low_plus}, {p_low_minus}, {h_low_p})"
    assert m_low_plus == 0 and m_low_minus == 1 and h_low_m == 0.5, f"Lower bound d=-1 failed: ({m_low_plus}, {m_low_minus}, {h_low_m})"
    assert p_low_plus - p_low_minus == 2 * (+1) * h_low_p
    assert m_low_plus - m_low_minus == 2 * (-1) * h_low_m
    assert 0 <= p_low_plus <= 30 and 0 <= p_low_minus <= 30

    # 3. Upper-bound equivalent around theta = 29.8
    p_high = Parameter(name="TestUpper", current_value=29.8, min_value=0.0, max_value=30.0, scale=1.0, is_integer=True)
    p_high_plus, p_high_minus, h_high_p = p_high.materialize_candidates(29.8, c_k=1.0, d=+1)
    m_high_plus, m_high_minus, h_high_m = p_high.materialize_candidates(29.8, c_k=1.0, d=-1)
    assert p_high_plus == 30 and p_high_minus == 29 and h_high_p == 0.5, f"Upper bound d=+1 failed: ({p_high_plus}, {p_high_minus}, {h_high_p})"
    assert m_high_plus == 29 and m_high_minus == 30 and h_high_m == 0.5, f"Upper bound d=-1 failed: ({m_high_plus}, {m_high_minus}, {h_high_m})"
    assert p_high_plus - p_high_minus == 2 * (+1) * h_high_p
    assert m_high_plus - m_high_minus == 2 * (-1) * h_high_m
    assert 0 <= p_high_plus <= 30 and 0 <= p_high_minus <= 30

    # 4. Confirmation that lower-bound parameter receives nonzero update pointing back into feasible region
    # Scenario A: d = +1, candidate + (val=1) scored better than candidate - (val=0) by score_diff = +0.25
    a_k = 0.1
    score_diff_a = 0.25
    d_a = +1
    norm_h_a = h_low_p / p_low.scale
    g_norm_a = (score_diff_a * d_a) / (2.0 * norm_h_a)
    step_a = a_k * p_low.scale * g_norm_a
    new_theta_a = p_low.clamp(0.473 + step_a)
    assert new_theta_a > 0.473, f"Update did not move inward: {new_theta_a} <= 0.473"

    # Scenario B: d = -1, candidate - (val=1) scored better than candidate + (val=0) by score_diff = -0.25
    score_diff_b = -0.25
    d_b = -1
    norm_h_b = h_low_m / p_low.scale
    g_norm_b = (score_diff_b * d_b) / (2.0 * norm_h_b)
    step_b = a_k * p_low.scale * g_norm_b
    new_theta_b = p_low.clamp(0.473 + step_b)
    assert new_theta_b > 0.473, f"Update did not move inward: {new_theta_b} <= 0.473"
    assert abs(new_theta_a - new_theta_b) < 1e-9, f"Gradient update directionally inconsistent: {new_theta_a} vs {new_theta_b}"


# =============================================================================
# CLI and Entry Point
# =============================================================================

def main() -> int:
    parser = argparse.ArgumentParser(description="Howl SPSA Evaluator Tuning Runner")
    parser.add_argument("--iterations", type=int, default=1000, help="Maximum number of iterations to run (default: 1000)")
    parser.add_argument("--base-time", type=float, default=120.0, help="Base time in seconds per game (default: 120)")
    parser.add_argument("--inc", type=float, default=1.0, help="Increment in seconds per move (default: 1.0)")
    parser.add_argument("--concurrency", type=int, default=8, help="Number of concurrent game processes (default: 8)")
    parser.add_argument("--syzygy-path", type=str, default="/home/masoud/syzygy/3-4-5-wdl", help="Syzygy tablebase path")
    parser.add_argument("--syzygy-limit", type=int, default=5, help="Syzygy probe limit (default: 5)")
    parser.add_argument("--state-dir", type=str, default="spsa_state", help="Directory for state checkpoints and logs")
    parser.add_argument("--repo-root", type=str, default=".", help="Repository root path")
    parser.add_argument("--build-dir", type=str, default="build", help="CMake build directory")
    parser.add_argument("--seed", type=int, default=42, help="Base random seed")
    parser.add_argument("--dry-run", action="store_true", help="Perform dry run with 1 perturbation without launching games")
    parser.add_argument("--smoke-test", action="store_true", help="Run a quick 1-iteration smoke test with fast time control (0.1s + 0.05s)")
    parser.add_argument("--list-params", action="store_true", help="List all discovered parameters and exit")
    parser.add_argument("--params-file", type=str, default=None, help="Custom parameters JSON file")
    parser.add_argument(
        "--build-checkpoint-candidate",
        type=str,
        default=None,
        metavar="PATH",
        help="Build a standalone Howl binary from the current checkpoint theta without running SPSA or games",
    )

    args = parser.parse_args()

    cfg = SPSAConfig(
        base_time_sec=args.base_time,
        inc_sec=args.inc,
        concurrency=args.concurrency,
        syzygy_path=args.syzygy_path,
        syzygy_probe_limit=args.syzygy_limit,
        state_dir=args.state_dir,
        repo_root=args.repo_root,
        build_dir=args.build_dir,
        max_iterations=args.iterations,
        base_seed=args.seed,
    )

    if args.smoke_test:
        cfg.base_time_sec = 0.1
        cfg.inc_sec = 0.05
        cfg.max_iterations = 1

    tuner = SPSATuner(cfg, params_file=args.params_file)

    if args.build_checkpoint_candidate:
        if not os.path.isfile(tuner.checkpoint_file):
            print(f"[SPSA] Error: Checkpoint file not found at {tuner.checkpoint_file}", file=sys.stderr)
            return 1
        tuner.build_checkpoint_candidate(args.build_checkpoint_candidate)
        return 0

    if args.list_params:
        print(f"\nDiscovered {len(tuner.params)} parameters ({sum(1 for p in tuner.params.values() if p.enabled)} enabled):")
        print(f"{'Name':<42} {'Current':>8} {'Min':>8} {'Max':>8} {'Scale':>6} {'Enabled':>8}")
        print("-" * 86)
        for p in tuner.params.values():
            print(f"{p.name:<42} {p.current_value:>8.1f} {p.min_value:>8.1f} {p.max_value:>8.1f} {p.scale:>6.1f} {str(p.enabled):>8}")
        return 0

    if args.dry_run:
        print("\n=== SPSA DRY RUN MODE ===")
        # Run perturbation and gradient unit tests
        run_perturbation_unit_tests()
        print("Perturbation & gradient unit tests passed (symmetric, lower-bound, upper-bound, inward-update).")

        rng = random.Random(cfg.base_seed)
        delta, a_k, c_k, theta_plus, theta_minus, effective_h = tuner.generate_perturbation(1, rng)
        print(f"Total parameters: {len(tuner.params)}")
        print(f"Enabled parameters: {sum(1 for p in tuner.params.values() if p.enabled)}")
        print(f"Schedule: a_1 = {a_k:.6f}, c_1 = {c_k:.6f}")
        print("\nSample parameter perturbations (first 10 enabled):")
        shown = 0
        for name, param in tuner.params.items():
            if param.enabled:
                print(f"  {name:<38}: base={tuner.theta[name]:>6.1f} | delta={delta[name]:+d} | h={effective_h[name]:>4.1f} | plus={theta_plus[name]:>6} | minus={theta_minus[name]:>6}")
                shown += 1
                if shown >= 10:
                    break
        print("\nTesting candidate build generation in dry-run mode...")
        test_dir = os.path.join(cfg.state_dir, "dry_run_test")
        plus_bin = os.path.join(test_dir, "howl_plus_test")
        build_engine_variant(theta_plus, plus_bin, test_dir, "dry_run_plus", cfg.repo_root, cfg.build_dir)
        print(f"Successfully built dry-run plus binary at: {plus_bin}")
        shutil.rmtree(test_dir, ignore_errors=True)
        print("Dry run completed successfully without launching match games.")
        return 0

    try:
        tuner.run()
    except KeyboardInterrupt:
        print("\n[SPSA] Stopped by user.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
