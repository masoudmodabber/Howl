#!/usr/bin/env python3
"""
Paired-opening fixed-node self-match runner for Howl evaluator redesign validation.
Supports:
- Deterministic paired openings (Candidate as White, Candidate as Black)
- Fixed node search (nodes per move)
- Multiprocess concurrency (16 concurrent games on 24-core host)
- Pentanomial statistics, Elo difference, and 95% confidence intervals
- Complete error, timeout, illegal move, and crash tracking
"""

from __future__ import annotations

import argparse
from concurrent.futures import ProcessPoolExecutor, as_completed
import csv
import json
import math
import os
from pathlib import Path
import sys
import time
from typing import Any, Dict, List, Optional, Tuple

import chess
import chess.pgn

# Add tuning directory to path to reuse robust UCIEngineProcess
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tuning"))
from spsa_tune import UCIEngineProcess, EngineCrashError


def load_openings(path: str, count: int) -> List[Tuple[str, str]]:
    openings: List[Tuple[str, str]] = []
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            openings.append((row["id"], row["fen"]))
            if len(openings) >= count:
                break
    return openings


def play_game(
    game_idx: int,
    opening_id: str,
    fen: str,
    cand_color: chess.Color,
    cand_bin: str,
    prod_bin: str,
    nodes_per_move: int,
    cpu_core: int,
) -> dict[str, Any]:
    if hasattr(os, "sched_setaffinity"):
        try:
            os.sched_setaffinity(0, {cpu_core})
        except Exception:
            pass

    white_bin = cand_bin if cand_color == chess.WHITE else prod_bin
    black_bin = prod_bin if cand_color == chess.WHITE else cand_bin
    white_name = "Howl-Candidate" if cand_color == chess.WHITE else "Howl-Production"
    black_name = "Howl-Production" if cand_color == chess.WHITE else "Howl-Candidate"

    engine_procs = {}
    uci_opts = {"Hash": 32}

    try:
        engine_procs[chess.WHITE] = UCIEngineProcess(white_bin, uci_opts)
    except Exception as e:
        return {
            "game_idx": game_idx,
            "opening_id": opening_id,
            "cand_color": "white" if cand_color == chess.WHITE else "black",
            "result": "0-1",
            "winner": "production",
            "cand_score": 0.0,
            "prod_score": 1.0,
            "termination": f"White launch error: {e}",
            "plies": 0,
            "nodes": 0,
            "crashes": 1,
            "illegal_moves": 0,
            "timeouts": 0,
        }

    try:
        engine_procs[chess.BLACK] = UCIEngineProcess(black_bin, uci_opts)
    except Exception as e:
        engine_procs[chess.WHITE].close()
        return {
            "game_idx": game_idx,
            "opening_id": opening_id,
            "cand_color": "white" if cand_color == chess.WHITE else "black",
            "result": "1-0",
            "winner": "production",
            "cand_score": 0.0,
            "prod_score": 1.0,
            "termination": f"Black launch error: {e}",
            "plies": 0,
            "nodes": 0,
            "crashes": 1,
            "illegal_moves": 0,
            "timeouts": 0,
        }

    board = chess.Board(fen)
    played_moves: List[str] = []
    total_nodes = 0
    crashes = 0
    illegal_moves = 0
    timeouts = 0
    winner = None
    termination = ""
    result_str = "*"

    try:
        # Limit to 200 moves (400 plies) to avoid infinite loops in drawn positions
        max_plies = 300
        while not board.is_game_over(claim_draw=True) and len(played_moves) < max_plies:
            side = board.turn
            engine = engine_procs[side]
            is_cand = (side == cand_color)

            move_str = None
            try:
                move_str = engine.get_move(
                    starting_fen=fen,
                    moves=played_moves,
                    wtime_ms=1000,
                    btime_ms=1000,
                    winc_ms=0,
                    binc_ms=0,
                    timeout_sec=max(30.0, (nodes_per_move / 5000.0) + 15.0),
                    nodes_per_move=nodes_per_move,
                )
            except EngineCrashError as e:
                crashes += 1
                termination = f"Crash ({'Candidate' if is_cand else 'Production'}): {e.message}"
                winner = "production" if is_cand else "candidate"
                result_str = "0-1" if side == chess.WHITE else "1-0"
                break
            except Exception as e:
                crashes += 1
                termination = f"Process exception ({'Candidate' if is_cand else 'Production'}): {e}"
                winner = "production" if is_cand else "candidate"
                result_str = "0-1" if side == chess.WHITE else "1-0"
                break

            total_nodes += nodes_per_move

            if move_str is None or move_str == "(none)":
                timeouts += 1
                termination = f"Timeout / no move ({'Candidate' if is_cand else 'Production'})"
                winner = "production" if is_cand else "candidate"
                result_str = "0-1" if side == chess.WHITE else "1-0"
                break

            try:
                uci_move = chess.Move.from_uci(move_str)
            except ValueError:
                illegal_moves += 1
                termination = f"Malformed move syntax '{move_str}'"
                winner = "production" if is_cand else "candidate"
                result_str = "0-1" if side == chess.WHITE else "1-0"
                break

            if uci_move not in board.legal_moves:
                illegal_moves += 1
                termination = f"Illegal move '{move_str}'"
                winner = "production" if is_cand else "candidate"
                result_str = "0-1" if side == chess.WHITE else "1-0"
                break

            board.push(uci_move)
            played_moves.append(move_str)

        if not termination:
            if board.is_checkmate():
                if board.turn == cand_color:
                    winner = "production"
                    result_str = "0-1" if cand_color == chess.WHITE else "1-0"
                    termination = "Checkmate (Production wins)"
                else:
                    winner = "candidate"
                    result_str = "1-0" if cand_color == chess.WHITE else "0-1"
                    termination = "Checkmate (Candidate wins)"
            elif board.is_stalemate():
                winner = None
                result_str = "1/2-1/2"
                termination = "Stalemate"
            elif board.is_insufficient_material():
                winner = None
                result_str = "1/2-1/2"
                termination = "Insufficient material"
            elif board.can_claim_threefold_repetition():
                winner = None
                result_str = "1/2-1/2"
                termination = "Threefold repetition"
            elif board.can_claim_fifty_moves():
                winner = None
                result_str = "1/2-1/2"
                termination = "50-move rule"
            elif len(played_moves) >= max_plies:
                winner = None
                result_str = "1/2-1/2"
                termination = f"Adjudicated draw ({max_plies} plies limit)"
            else:
                winner = None
                result_str = "1/2-1/2"
                termination = "Draw by rule"

    finally:
        engine_procs[chess.WHITE].close()
        engine_procs[chess.BLACK].close()

    cand_score = 1.0 if winner == "candidate" else (0.5 if winner is None else 0.0)
    prod_score = 1.0 if winner == "production" else (0.5 if winner is None else 0.0)

    return {
        "game_idx": game_idx,
        "opening_id": opening_id,
        "cand_color": "white" if cand_color == chess.WHITE else "black",
        "result": result_str,
        "winner": winner,
        "cand_score": cand_score,
        "prod_score": prod_score,
        "termination": termination,
        "plies": len(played_moves),
        "nodes": total_nodes,
        "crashes": crashes,
        "illegal_moves": illegal_moves,
        "timeouts": timeouts,
    }


def compute_elo(score_pct: float) -> float:
    score = min(max(score_pct, 1e-6), 1.0 - 1e-6)
    return -400.0 * math.log10(1.0 / score - 1.0)


def compute_pentanomial_ci(penta: list[int]) -> tuple[float, float, float, float]:
    """
    Computes Elo, win percentage, standard error, and 95% confidence interval
    from Fishtest pentanomial pair counts: [LL, LD, DD/WL, WD, WW]
    Corresponding scores: [0.0, 0.5, 1.0, 1.5, 2.0]
    """
    total_pairs = sum(penta)
    if total_pairs == 0:
        return 0.0, 0.0, 0.0, 0.0
    
    # Normalized weights
    weights = [0.0, 0.25, 0.5, 0.75, 1.0]
    mean = sum(penta[i] * weights[i] for i in range(5)) / total_pairs
    variance = sum(penta[i] * (weights[i] - mean) ** 2 for i in range(5)) / total_pairs
    stderror = math.sqrt(variance / total_pairs)
    
    elo = compute_elo(mean)
    elo_lower = compute_elo(mean - 1.96 * stderror)
    elo_upper = compute_elo(mean + 1.96 * stderror)
    
    return elo, mean * 100.0, elo_lower, elo_upper


def main():
    parser = argparse.ArgumentParser(description="Fixed-node paired self match runner")
    parser.add_argument("--candidate", default="builds/candidate/howl_candidate", help="Candidate executable")
    parser.add_argument("--production", default="builds/production/howl_production", help="Production executable")
    parser.add_argument("--suite", default="tools/matches/suites/openings-200.tsv", help="Openings suite TSV")
    parser.add_argument("--pairs", type=int, default=200, help="Number of opening pairs (each played twice)")
    parser.add_argument("--nodes", type=int, default=20000, help="Fixed nodes per move")
    parser.add_argument("--concurrency", type=int, default=16, help="Concurrent games")
    parser.add_argument("--out-dir", default="evaluator-analysis/redesign/attack/match", help="Output directory")
    args = parser.parse_args()

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    openings = load_openings(args.suite, args.pairs)
    actual_pairs = len(openings)
    total_games = actual_pairs * 2

    print(f"=== Starting Howl Evaluator Self Match ===")
    print(f"Candidate:  {args.candidate}")
    print(f"Production: {args.production}")
    print(f"Suite:      {args.suite} ({actual_pairs} openings -> {total_games} paired games)")
    print(f"Search:     Fixed {args.nodes} nodes/move | Concurrency: {args.concurrency}")
    print(f"Output:     {out_dir}")
    print("=" * 50)

    # Build tasks: each opening played as:
    # 1. Candidate White, Production Black
    # 2. Candidate Black, Production White
    tasks = []
    for pair_idx, (oid, fen) in enumerate(openings):
        tasks.append((pair_idx * 2, oid, fen, chess.WHITE, pair_idx % args.concurrency))
        tasks.append((pair_idx * 2 + 1, oid, fen, chess.BLACK, (pair_idx + 8) % args.concurrency))

    results: list[dict[str, Any] | None] = [None] * len(tasks)
    start_time = time.monotonic()
    completed_count = 0

    with ProcessPoolExecutor(max_workers=args.concurrency) as pool:
        futures = {
            pool.submit(play_game, t[0], t[1], t[2], t[3], args.candidate, args.production, args.nodes, t[4]): t[0]
            for t in tasks
        }
        for future in as_completed(futures):
            g_idx = futures[future]
            res = future.result()
            results[g_idx] = res
            completed_count += 1
            if completed_count % 20 == 0 or completed_count == total_games:
                elapsed = time.monotonic() - start_time
                print(f"Progress: {completed_count}/{total_games} games ({completed_count / elapsed:.1f} games/s) [Elapsed: {elapsed:.1f}s]")

    wall_time = time.monotonic() - start_time

    # Tally results from candidate perspective
    cand_wins = sum(1 for r in results if r and r["winner"] == "candidate")
    prod_wins = sum(1 for r in results if r and r["winner"] == "production")
    draws = sum(1 for r in results if r and r["winner"] is None)
    total_score = cand_wins + 0.5 * draws
    score_pct = (total_score / total_games) * 100.0

    total_crashes = sum(r["crashes"] for r in results if r)
    total_illegal = sum(r["illegal_moves"] for r in results if r)
    total_timeouts = sum(r["timeouts"] for r in results if r)
    total_nodes_all = sum(r["nodes"] for r in results if r)
    total_plies_all = sum(r["plies"] for r in results if r)
    avg_nodes_per_move = total_nodes_all / max(1, total_plies_all)

    # Pentanomial calculation: [LL, LD, DD/WL, WD, WW]
    penta = [0, 0, 0, 0, 0]
    for p in range(actual_pairs):
        g1 = results[2 * p]
        g2 = results[2 * p + 1]
        pair_score = g1["cand_score"] + g2["cand_score"]
        penta_idx = int(round(pair_score * 2.0))
        penta[penta_idx] += 1

    elo, mean_pct, elo_low, elo_high = compute_pentanomial_ci(penta)

    # Write games TSV
    with open(out_dir / "games.tsv", "w", newline="", encoding="utf-8") as f:
        fields = ["game_idx", "opening_id", "cand_color", "result", "winner", "cand_score", "prod_score", "termination", "plies", "nodes", "crashes", "illegal_moves", "timeouts"]
        writer = csv.DictWriter(f, fieldnames=fields, delimiter="\t")
        writer.writeheader()
        for r in results:
            if r: writer.writerow(r)

    summary = {
        "games": total_games,
        "paired_openings": actual_pairs,
        "nodes_per_move": args.nodes,
        "concurrency": args.concurrency,
        "candidate_wins": cand_wins,
        "draws": draws,
        "candidate_losses": prod_wins,
        "pentanomial": penta,
        "candidate_score_percent": score_pct,
        "elo": elo,
        "elo_95ci_lower": elo_low,
        "elo_95ci_upper": elo_high,
        "crashes": total_crashes,
        "illegal_moves": total_illegal,
        "timeouts": total_timeouts,
        "avg_nodes_per_move": avg_nodes_per_move,
        "wall_time_sec": wall_time,
    }

    with open(out_dir / "summary.json", "w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)

    print("\n" + "=" * 50)
    print("MATCH SUMMARY (Candidate vs Production)")
    print(f"Games:            {total_games} ({actual_pairs} paired openings)")
    print(f"Fixed Nodes:      {args.nodes} nodes/move")
    print(f"Score:            +{cand_wins} ={draws} -{prod_wins} ({total_score:.1f}/{total_games})")
    print(f"Candidate Score:  {score_pct:.2f}%")
    print(f"Pentanomial:      {penta} [0.0, 0.5, 1.0, 1.5, 2.0]")
    print(f"Elo Difference:   {elo:+.2f} [{elo_low:+.2f}, {elo_high:+.2f}] (95% CI)")
    print(f"Anomalies:        Crashes: {total_crashes} | Illegal: {total_illegal} | Timeouts: {total_timeouts}")
    print(f"Avg Nodes/Move:   {avg_nodes_per_move:.0f}")
    print(f"Wall Time:        {wall_time:.1f}s ({total_games/wall_time:.2f} games/s)")
    print("=" * 50)


if __name__ == "__main__":
    main()
