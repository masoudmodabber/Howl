#!/usr/bin/env python3
"""
Generate diagnostic candidate positions from the TWIC PGN corpus.

Samples 60 positions across 4 coarse game phases (early middlegame,
middlegame, late middlegame, endgame) with at most one position per game,
skipping the first 12 plies, with a fixed random seed of 42.
"""

import argparse
import csv
import os
from pathlib import Path
import random
import sys
from typing import Dict, List, Set, Tuple

import chess
import chess.pgn


PHASES = ["early_middlegame", "middlegame", "late_middlegame", "endgame"]


def classify_phase(board: chess.Board) -> str:
    """Coarsely classify game phase based on remaining non-pawn pieces."""
    non_pawn = sum(
        1
        for p in board.piece_map().values()
        if p.piece_type not in (chess.PAWN, chess.KING)
    )
    if non_pawn >= 13:
        return "early_middlegame"
    elif non_pawn >= 9:
        return "middlegame"
    elif non_pawn >= 5:
        return "late_middlegame"
    else:
        return "endgame"


def has_immediate_mate(board: chess.Board) -> bool:
    """Check if the position is already checkmate or if there is an immediate mate in 1."""
    if board.is_checkmate():
        return True
    for m in board.legal_moves:
        if board.gives_check(m):
            board.push(m)
            mate = board.is_checkmate()
            board.pop()
            if mate:
                return True
    return False


def generate_diagnostic_positions(
    corpus_file: str,
    output_file: str,
    seed: int = 42,
    total_count: int = 60,
    skip_plies: int = 12,
) -> Tuple[int, int, Dict[str, int], bool, List[Dict[str, str]]]:
    corpus_path = Path(corpus_file)
    if not corpus_path.exists():
        raise FileNotFoundError(f"Corpus file not found: {corpus_file}")

    rng = random.Random(seed)
    target_per_phase = total_count // len(PHASES)

    phase_candidates: Dict[str, List[Dict[str, any]]] = {p: [] for p in PHASES}
    games_considered = 0

    with open(corpus_path, "r", encoding="utf-8", errors="replace") as f:
        while True:
            game = chess.pgn.read_game(f)
            if game is None:
                break
            games_considered += 1

            board = game.board()
            ply = 0
            game_pos: Dict[str, List[Dict[str, any]]] = {p: [] for p in PHASES}

            white = game.headers.get("White", "?").replace("\t", " ").strip()
            black = game.headers.get("Black", "?").replace("\t", " ").strip()
            source_game = f"twic1660_g{games_considered:04d} ({white} vs {black})"

            for move in game.mainline_moves():
                board.push(move)
                ply += 1
                if ply <= skip_plies:
                    continue
                if not board.is_valid():
                    continue
                if board.is_game_over():
                    continue
                if not any(board.legal_moves):
                    continue
                if has_immediate_mate(board):
                    continue

                ph = classify_phase(board)
                game_pos[ph].append(
                    {
                        "fen": board.fen(),
                        "source_game": source_game,
                        "ply": ply,
                        "game_phase": ph,
                        "game_id": games_considered,
                    }
                )

            # Sample at most 1 candidate position per phase for this game
            for p in PHASES:
                if game_pos[p]:
                    phase_candidates[p].append(rng.choice(game_pos[p]))

    used_games: Set[int] = set()
    selected_by_phase: Dict[str, List[Dict[str, any]]] = {p: [] for p in PHASES}

    for p in PHASES:
        rng.shuffle(phase_candidates[p])
        for cand in phase_candidates[p]:
            if cand["game_id"] not in used_games:
                used_games.add(cand["game_id"])
                selected_by_phase[p].append(cand)
                if len(selected_by_phase[p]) == target_per_phase:
                    break
        if len(selected_by_phase[p]) < target_per_phase:
            raise RuntimeError(
                f"Could not find enough positions for phase '{p}': needed {target_per_phase}, got {len(selected_by_phase[p])}"
            )

    # Interleave positions across phases for sample diversity
    selected: List[Dict[str, any]] = []
    for i in range(target_per_phase):
        for p in PHASES:
            selected.append(selected_by_phase[p][i])

    out_records: List[Dict[str, str]] = []
    for idx, cand in enumerate(selected, 1):
        out_records.append(
            {
                "id": f"pos_{idx:03d}",
                "fen": cand["fen"],
                "source_game": cand["source_game"],
                "ply": str(cand["ply"]),
                "game_phase": cand["game_phase"],
            }
        )

    out_path = Path(output_file)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    fieldnames = ["id", "fen", "source_game", "ply", "game_phase"]
    with open(out_path, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(out_records)

    counts_by_phase = {p: len(selected_by_phase[p]) for p in PHASES}
    single_game_per_position = len(used_games) == len(out_records)

    return games_considered, len(out_records), counts_by_phase, single_game_per_position, out_records


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate diagnostic candidate positions from TWIC corpus."
    )
    parser.add_argument(
        "--corpus",
        default="tuner-corpus/twic1660.pgn",
        help="Path to PGN corpus (default: tuner-corpus/twic1660.pgn)",
    )
    parser.add_argument(
        "--output",
        default="diagnostics/positions.tsv",
        help="Path to output TSV (default: diagnostics/positions.tsv)",
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=42,
        help="Random seed (default: 42)",
    )
    parser.add_argument(
        "--count",
        type=int,
        default=60,
        help="Total number of positions (default: 60)",
    )
    parser.add_argument(
        "--skip-plies",
        type=int,
        default=12,
        help="Number of initial plies to skip per game (default: 12)",
    )

    args = parser.parse_args()

    games_considered, total_gen, phase_counts, unique_games, records = generate_diagnostic_positions(
        corpus_file=args.corpus,
        output_file=args.output,
        seed=args.seed,
        total_count=args.count,
        skip_plies=args.skip_plies,
    )

    print(f"Generated {total_gen} diagnostic positions from {games_considered} games.")
    for p, c in phase_counts.items():
        print(f"  {p}: {c}")
    print(f"All positions from distinct games: {unique_games}")
    print(f"Written to: {args.output}")


if __name__ == "__main__":
    main()
