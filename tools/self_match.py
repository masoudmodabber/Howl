#!/usr/bin/env python3
"""
Standalone self-match harness for Howl UCI engine.
Compares two engine binaries playing fixed positions swapping colours.
"""

from __future__ import annotations

import argparse
import collections
from concurrent.futures import ProcessPoolExecutor, as_completed
from dataclasses import dataclass
import os
import subprocess
import sys
import threading
import time
from typing import List, Optional, Tuple

import chess
import chess.pgn


# Four simple, shallow, varied named opening positions
STARTING_POSITIONS: List[Tuple[str, str]] = [
    ("Italian Game", "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3"),
    ("Sicilian Defense Open", "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq - 1 5"),
    ("Queen's Gambit Declined", "rnbqkb1r/ppp2ppp/4pn2/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 2 4"),
    ("French Defense Advance", "rnbqkbnr/ppp2ppp/4p3/3pP3/3P4/8/PPP2PPP/RNBQKBNR b KQkq - 0 3"),
]


@dataclass
class EngineConfig:
    name: str
    path: str


@dataclass
class MatchConfig:
    old_engine: EngineConfig
    new_engine: EngineConfig
    movetime_sec: float
    inc_sec: float
    concurrency: int
    pgn_output: Optional[str] = None


@dataclass
class GameResult:
    game_index: int
    pos_name: str
    starting_fen: str
    white_engine: str
    black_engine: str
    result: str  # "1-0", "0-1", "1/2-1/2"
    winner: Optional[str]  # "old", "new", or None for draw
    termination: str
    pgn_str: str


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
    def __init__(self, path: str):
        self.path = os.path.abspath(path)
        if not os.path.isfile(self.path):
            raise FileNotFoundError(f"Engine executable not found at: {self.path}")

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
        # Give process a moment to exit and capture remaining stderr
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


def format_crash_diagnostics(
    crash: EngineCrashError,
    role: str,
    side_name: str,
    move_number: int,
    fen: str,
) -> str:
    lines = [
        f"Engine crash ({role}) on move {move_number} [{side_name}]",
        f"Reason: {crash.message}",
        f"Return code: {crash.returncode}",
        f"Current FEN: {fen}",
        f"Last UCI sent: {crash.last_uci_command}",
        f"Last bestmove received: {crash.last_bestmove}",
    ]
    if crash.stderr_lines:
        lines.append("Stderr (last lines):")
        for stderr_l in crash.stderr_lines:
            lines.append(f"  {stderr_l}")
    else:
        lines.append("Stderr: <empty>")
    return "\n".join(lines)


def play_single_game(
    game_idx: int,
    pos_name: str,
    starting_fen: str,
    white_role: str,  # "old" or "new"
    black_role: str,  # "old" or "new"
    old_engine_path: str,
    new_engine_path: str,
    base_time_sec: float,
    inc_sec: float,
) -> GameResult:
    white_path = old_engine_path if white_role == "old" else new_engine_path
    black_path = old_engine_path if black_role == "old" else new_engine_path
    white_name = f"Howl-old" if white_role == "old" else f"Howl-new"
    black_name = f"Howl-old" if black_role == "old" else f"Howl-new"

    engine_procs = {}
    try:
        engine_procs[chess.WHITE] = UCIEngineProcess(white_path)
    except Exception as e:
        return _make_error_result(
            game_idx, pos_name, starting_fen, white_name, black_name,
            winner=black_role,
            result="0-1",
            termination=f"White failed to launch: {e}"
        )

    try:
        engine_procs[chess.BLACK] = UCIEngineProcess(black_path)
    except Exception as e:
        engine_procs[chess.WHITE].close()
        return _make_error_result(
            game_idx, pos_name, starting_fen, white_name, black_name,
            winner=white_role,
            result="1-0",
            termination=f"Black failed to launch: {e}"
        )

    board = chess.Board(starting_fen)
    played_moves: List[str] = []

    wtime_ms = int(base_time_sec * 1000)
    btime_ms = int(base_time_sec * 1000)
    winc_ms = int(inc_sec * 1000)
    binc_ms = int(inc_sec * 1000)

    game_pgn = chess.pgn.Game()
    game_pgn.headers["Event"] = "Howl Self Match"
    game_pgn.headers["Site"] = "Local"
    game_pgn.headers["Date"] = time.strftime("%Y.%m.%d")
    game_pgn.headers["Round"] = str(game_idx)
    game_pgn.headers["White"] = white_name
    game_pgn.headers["Black"] = black_name
    game_pgn.headers["SetUp"] = "1" if starting_fen != chess.STARTING_FEN else "0"
    if starting_fen != chess.STARTING_FEN:
        game_pgn.headers["FEN"] = starting_fen
    game_pgn.headers["Opening"] = pos_name

    current_node = game_pgn
    winner: Optional[str] = None
    result_str = "*"
    termination = ""
    crash_details: Optional[str] = None

    try:
        while not board.is_game_over(claim_draw=True):
            side = board.turn
            side_name = "White" if side == chess.WHITE else "Black"
            engine = engine_procs[side]
            role = white_role if side == chess.WHITE else black_role
            opp_role = black_role if side == chess.WHITE else white_role
            move_num = board.fullmove_number

            curr_clock_ms = wtime_ms if side == chess.WHITE else btime_ms
            # Timeout margin: clock time plus 10 seconds buffer
            timeout_sec = (curr_clock_ms / 1000.0) + 10.0

            t0 = time.monotonic()
            move_str = None
            try:
                move_str = engine.get_move(
                    starting_fen=starting_fen,
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
                crash_details = format_crash_diagnostics(
                    crash=e,
                    role=role,
                    side_name=side_name,
                    move_number=move_num,
                    fen=board.fen(),
                )
                termination = f"Engine crash ({role}): {e.message} (rc={e.returncode})"
                break
            except Exception as e:
                result_str = "0-1" if side == chess.WHITE else "1-0"
                winner = opp_role
                termination = f"Engine crash ({role}): {e}"
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
                break

            try:
                uci_move = chess.Move.from_uci(move_str)
            except ValueError:
                result_str = "0-1" if side == chess.WHITE else "1-0"
                winner = opp_role
                termination = f"Illegal move syntax '{move_str}' by {role}"
                break

            if uci_move not in board.legal_moves:
                result_str = "0-1" if side == chess.WHITE else "1-0"
                winner = opp_role
                termination = f"Illegal move '{move_str}' played by {role}"
                break

            current_node = current_node.add_variation(uci_move)
            board.push(uci_move)
            played_moves.append(move_str)

        if not termination:
            if board.is_checkmate():
                # The side to move is checkmated, so opposite side wins
                if board.turn == chess.WHITE:
                    result_str = "0-1"
                    winner = black_role
                    termination = f"Checkmate ({black_name} wins)"
                else:
                    result_str = "1-0"
                    winner = white_role
                    termination = f"Checkmate ({white_name} wins)"
            elif board.is_stalemate():
                result_str = "1/2-1/2"
                winner = None
                termination = "Stalemate"
            elif board.is_repetition(3):
                result_str = "1/2-1/2"
                winner = None
                termination = "Threefold repetition"
            elif board.is_fifty_moves():
                result_str = "1/2-1/2"
                winner = None
                termination = "Fifty-move rule"
            elif board.is_insufficient_material():
                result_str = "1/2-1/2"
                winner = None
                termination = "Insufficient material"
            else:
                result_str = "1/2-1/2"
                winner = None
                termination = "Draw by rules"

    finally:
        for p in engine_procs.values():
            p.close()

    game_pgn.headers["Result"] = result_str
    game_pgn.headers["Termination"] = termination
    if crash_details:
        game_pgn.headers["CrashDetails"] = crash_details.replace("\n", " | ")
        current_node.comment = f"Game terminated by crash:\n{crash_details}"

    return GameResult(
        game_index=game_idx,
        pos_name=pos_name,
        starting_fen=starting_fen,
        white_engine=white_name,
        black_engine=black_name,
        result=result_str,
        winner=winner,
        termination=crash_details if crash_details else termination,
        pgn_str=str(game_pgn),
    )


def _make_error_result(
    game_idx: int,
    pos_name: str,
    starting_fen: str,
    white_name: str,
    black_name: str,
    winner: Optional[str],
    result: str,
    termination: str,
) -> GameResult:
    game_pgn = chess.pgn.Game()
    game_pgn.headers["Event"] = "Howl Self Match"
    game_pgn.headers["Round"] = str(game_idx)
    game_pgn.headers["White"] = white_name
    game_pgn.headers["Black"] = black_name
    game_pgn.headers["Opening"] = pos_name
    game_pgn.headers["Result"] = result
    game_pgn.headers["Termination"] = termination
    return GameResult(
        game_index=game_idx,
        pos_name=pos_name,
        starting_fen=starting_fen,
        white_engine=white_name,
        black_engine=black_name,
        result=result,
        winner=winner,
        termination=termination,
        pgn_str=str(game_pgn),
    )


def run_match(config: MatchConfig) -> List[GameResult]:
    tasks = []
    game_idx = 1
    for pos_name, fen in STARTING_POSITIONS:
        # Pairing 1: old White vs new Black
        tasks.append((game_idx, pos_name, fen, "old", "new"))
        game_idx += 1
        # Pairing 2: new White vs old Black
        tasks.append((game_idx, pos_name, fen, "new", "old"))
        game_idx += 1

    results: List[GameResult] = []
    max_workers = max(1, config.concurrency)

    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        future_map = {
            executor.submit(
                play_single_game,
                idx,
                pos,
                fen,
                w_role,
                b_role,
                config.old_engine.path,
                config.new_engine.path,
                config.movetime_sec,
                config.inc_sec,
            ): idx
            for (idx, pos, fen, w_role, b_role) in tasks
        }

        for future in as_completed(future_map):
            res = future.result()
            results.append(res)
            # Display single-line summary on console
            summary_term = res.termination.splitlines()[0] if "\n" in res.termination else res.termination
            print(
                f"[Game {res.game_index:02d}/08] {res.pos_name:<24} "
                f"{res.white_engine} vs {res.black_engine} -> {res.result:<7} ({summary_term})"
            )

    results.sort(key=lambda r: r.game_index)
    return results


def print_summary(results: List[GameResult]) -> None:
    old_wins = sum(1 for r in results if r.winner == "old")
    new_wins = sum(1 for r in results if r.winner == "new")
    draws = sum(1 for r in results if r.winner is None)

    old_score = old_wins + 0.5 * draws
    new_score = new_wins + 0.5 * draws

    print("\n" + "=" * 60)
    print("MATCH SUMMARY")
    print("=" * 60)
    print(f"Overall Old Wins: {old_wins}")
    print(f"Overall New Wins: {new_wins}")
    print(f"Draws:            {draws}")
    print(f"Old Score:        {old_score:.1f} / {len(results)}")
    print(f"New Score:        {new_score:.1f} / {len(results)}")
    print("-" * 60)
    print("RESULTS BY STARTING POSITION:")
    print("-" * 60)

    # Group by position name
    pos_groups: dict[str, List[GameResult]] = {}
    for r in results:
        pos_groups.setdefault(r.pos_name, []).append(r)

    for pos_name, games in pos_groups.items():
        pos_old = sum(1 for g in games if g.winner == "old") + 0.5 * sum(1 for g in games if g.winner is None)
        pos_new = sum(1 for g in games if g.winner == "new") + 0.5 * sum(1 for g in games if g.winner is None)
        print(f"{pos_name:<26} Old: {pos_old:.1f}  New: {pos_new:.1f}")
        for g in games:
            summary_term = g.termination.splitlines()[0] if "\n" in g.termination else g.termination
            print(f"  Game {g.game_index:02d}: {g.white_engine} vs {g.black_engine} | {g.result} | {summary_term}")
    print("=" * 60)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Self-match harness comparing two Howl UCI engine executables."
    )
    parser.add_argument("old", help="Path to the 'old' Howl executable")
    parser.add_argument("new", help="Path to the 'new' Howl executable")
    parser.add_argument(
        "--time",
        type=float,
        default=120.0,
        help="Base time per side in seconds (default: 120.0)",
    )
    parser.add_argument(
        "--inc",
        type=float,
        default=1.0,
        help="Time increment per move in seconds (default: 1.0)",
    )
    parser.add_argument(
        "--concurrency",
        type=int,
        default=4,
        help="Number of games to run concurrently (default: 4)",
    )
    parser.add_argument(
        "--pgn",
        type=str,
        default="match_output.pgn",
        help="Output file for PGN games (default: match_output.pgn)",
    )

    args = parser.parse_args()

    old_cfg = EngineConfig("Howl-old", args.old)
    new_cfg = EngineConfig("Howl-new", args.new)
    match_cfg = MatchConfig(
        old_engine=old_cfg,
        new_engine=new_cfg,
        movetime_sec=args.time,
        inc_sec=args.inc,
        concurrency=args.concurrency,
        pgn_output=args.pgn,
    )

    print(f"Starting self match: {args.old} vs {args.new}")
    print(f"Time Control: {args.time}s + {args.inc}s | Concurrency: {args.concurrency}\n")

    results = run_match(match_cfg)

    if args.pgn:
        with open(args.pgn, "w", encoding="utf-8") as f:
            for r in results:
                f.write(r.pgn_str + "\n\n")
        print(f"\nPGN written to {args.pgn}")

    print_summary(results)
    return 0


if __name__ == "__main__":
    sys.exit(main())
