#!/usr/bin/env python3
"""
Convert PGN games into a quiet tuner dataset with 90/10 game split.

Output format:
<FEN>\t<result>
where result is 1.0 (White win), 0.5 (draw), 0.0 (Black win).
"""

import sys
import os
import glob
import random
import argparse
import chess
import chess.pgn

# Piece values matching Howl's pieceValue100 in QSearcher.cpp (reused from tools/extract_fens.py)
PIECE_VALUES = {
    chess.PAWN: 100,
    chess.KNIGHT: 350,
    chess.BISHOP: 350,
    chess.ROOK: 550,
    chess.QUEEN: 975,
    chess.KING: 25000,
}

def static_exchange_evaluation(board: chess.Board, move: chess.Move) -> int:
    """
    Computes the Static Exchange Evaluation (SEE) gain for the side to move.
    Reused directly from tools/extract_fens.py.
    """
    to_sq = move.to_square
    victim = board.piece_type_at(to_sq)
    if not victim:
        victim = chess.PAWN if board.is_en_passant(move) else 0

    promo_gain = (PIECE_VALUES[move.promotion] - PIECE_VALUES[chess.PAWN]) if move.promotion else 0
    gain = [PIECE_VALUES.get(victim, 0) + promo_gain]

    if gain[0] <= 0:
        return 0

    b = board.copy(stack=False)
    b.push(move)
    color = b.turn

    while True:
        attackers = b.attackers(color, to_sq)
        if not attackers:
            break
        least_sq = min(attackers, key=lambda sq: PIECE_VALUES[b.piece_type_at(sq)])
        least_val = PIECE_VALUES[b.piece_type_at(least_sq)]
        b.push(chess.Move(least_sq, to_sq))
        gain.append(least_val)
        color = not color

    while len(gain) > 1:
        g = gain.pop()
        gain[-1] = max(0, gain[-1] - g)

    return gain[0]

def has_tactical_move(board: chess.Board) -> bool:
    """
    Returns True if the side to move has an immediately available capture
    or promotion with net material gain > 100 cp.
    Reused directly from tools/extract_fens.py.
    """
    for move in board.legal_moves:
        if move.promotion:
            if static_exchange_evaluation(board, move) > 100:
                return True
        elif board.is_capture(move):
            to_sq = move.to_square
            victim = board.piece_type_at(to_sq)
            if not victim and board.is_en_passant(move):
                victim = chess.PAWN
            if victim and PIECE_VALUES.get(victim, 0) > 100:
                if static_exchange_evaluation(board, move) > 100:
                    return True
    return False

def is_quiet(board: chess.Board) -> bool:
    """
    A position is quiet only if:
    1. Side to move is not in check.
    2. There is no promotion move exceeding 100 cp SEE.
    3. There is no legal capture whose SEE exceeds 100 centipawns.
    """
    if board.is_check():
        return False
    return not has_tactical_move(board)

def extract_positions_from_game(game, last_stride=4):
    positions = []
    board = game.board()
    last_ply = None

    for ply, move in enumerate(game.mainline_moves(), 1):
        board.push(move)
        # Skip the first 12 plies
        if ply <= 12:
            continue
        # Sample at most one position every 4 plies
        if last_ply is not None and (ply - last_ply < last_stride):
            continue
        if is_quiet(board):
            positions.append(board.fen())
            last_ply = ply

    return positions

def collect_pgn_files(input_paths):
    """
    Collect all PGN files from explicit paths (files or directories).
    Does not search repository automatically.
    """
    pgn_files = []
    for path in input_paths:
        if os.path.isdir(path):
            found = sorted(glob.glob(os.path.join(path, "*.pgn")))
            pgn_files.extend(found)
        elif os.path.isfile(path):
            pgn_files.append(path)
        else:
            # Check if glob pattern provided explicitly
            matched = sorted(glob.glob(path))
            if matched:
                for m in matched:
                    if os.path.isdir(m):
                        pgn_files.extend(sorted(glob.glob(os.path.join(m, "*.pgn"))))
                    elif os.path.isfile(m):
                        pgn_files.append(m)
            else:
                print(f"Warning: input path not found: {path}", file=sys.stderr)

    seen = set()
    unique_files = []
    for f in pgn_files:
        norm = os.path.normpath(f)
        if norm not in seen:
            seen.add(norm)
            unique_files.append(norm)
    return unique_files

def main():
    parser = argparse.ArgumentParser(
        description="Convert PGN games into a quiet tuner dataset."
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        help="One or more explicit PGN file paths or directories containing PGN files."
    )
    parser.add_argument(
        "--train-out",
        default="tuner-train.tsv",
        help="Output path for training TSV (default: tuner-train.tsv)"
    )
    parser.add_argument(
        "--val-out",
        default="tuner-validation.tsv",
        help="Output path for validation TSV (default: tuner-validation.tsv)"
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=42,
        help="Deterministic split seed (default: 42)"
    )

    args = parser.parse_args()

    pgn_files = collect_pgn_files(args.inputs)
    if not pgn_files:
        print("No valid PGN files found from supplied input paths.", file=sys.stderr)
        return 1

    valid_games = []
    result_map = {
        "1-0": 1.0,
        "0-1": 0.0,
        "1/2-1/2": 0.5,
    }

    for pgn_path in pgn_files:
        with open(pgn_path, "r", encoding="utf-8", errors="replace") as f:
            while True:
                try:
                    game = chess.pgn.read_game(f)
                except Exception:
                    break
                if game is None:
                    break
                result_str = game.headers.get("Result")
                if result_str in result_map:
                    valid_games.append((game, result_map[result_str]))

    total_games = len(valid_games)
    if total_games == 0:
        print("No games with decisive or draw result found.", file=sys.stderr)
        return 1

    # Deterministic split by game: 90% train, 10% validation
    rng = random.Random(args.seed)
    rng.shuffle(valid_games)

    num_train = int(total_games * 0.9)
    train_games = valid_games[:num_train]
    val_games = valid_games[num_train:]

    seen_fens = set()
    train_entries = []
    for game, result in train_games:
        fens = extract_positions_from_game(game)
        for fen in fens:
            if fen not in seen_fens:
                seen_fens.add(fen)
                train_entries.append((fen, result))

    val_entries = []
    for game, result in val_games:
        fens = extract_positions_from_game(game)
        for fen in fens:
            if fen not in seen_fens:
                seen_fens.add(fen)
                val_entries.append((fen, result))

    with open(args.train_out, "w", encoding="utf-8") as f:
        for fen, res in train_entries:
            f.write(f"{fen}\t{res:.1f}\n")

    with open(args.val_out, "w", encoding="utf-8") as f:
        for fen, res in val_entries:
            f.write(f"{fen}\t{res:.1f}\n")

    print(f"Total source games: {total_games}")
    print(f"Training game count: {len(train_games)}")
    print(f"Validation game count: {len(val_games)}")
    print(f"Training position count: {len(train_entries)}")
    print(f"Validation position count: {len(val_entries)}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
