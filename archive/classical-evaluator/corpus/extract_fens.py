#!/usr/bin/env python3
"""
Extract unique, quiet chess positions (FENs) from existing PGN game files
in the Howl repository for offline phase-curve calibration.

Quiet-position criteria:
1. Side to move is NOT in check (not board.is_check()).
2. Side to move does NOT have any immediate tactical capture or promotion
   worth more than approximately 100 cp, determined by Static Exchange
   Evaluation (SEE > 100 cp) using Howl's piece value scale.
"""

import sys
import glob
import os
import chess
import chess.pgn

# Piece values matching Howl's pieceValue100 in QSearcher.cpp
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
    """
    to_sq = move.to_square
    victim = board.piece_type_at(to_sq)
    if not victim:
        victim = chess.PAWN if board.is_en_passant(move) else 0

    promo_gain = (PIECE_VALUES[move.promotion] - PIECE_VALUES[chess.PAWN]) if move.promotion else 0
    gain = [PIECE_VALUES.get(victim, 0) + promo_gain]

    # Fast check: if no initial material gain, return 0
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
            # If victim is an ordinary pawn and no promotion, max possible gain is 100 cp.
            # Only higher-value victims (Knight, Bishop, Rook, Queen) can exceed 100 cp.
            if victim and PIECE_VALUES.get(victim, 0) > 100:
                if static_exchange_evaluation(board, move) > 100:
                    return True
    return False

def main():
    pgn_files = sorted(glob.glob("*.pgn"))
    if not pgn_files:
        print("No *.pgn files found in current directory.", file=sys.stderr)
        return 1

    output_file = sys.argv[1] if len(sys.argv) > 1 else "repository_positions.fen"

    unique_all = set()
    unique_non_check = set()
    unique_quiet = set()
    total_games = 0

    print(f"Scanning {len(pgn_files)} PGN files...")
    for pgn_path in pgn_files:
        with open(pgn_path, "r", encoding="utf-8", errors="replace") as f:
            while True:
                try:
                    game = chess.pgn.read_game(f)
                except Exception:
                    break
                if game is None:
                    break
                total_games += 1
                board = game.board()
                for move in game.mainline_moves():
                    board.push(move)
                    fen = board.fen()
                    unique_all.add(fen)
                    if not board.is_check():
                        unique_non_check.add(fen)
                        if not has_tactical_move(board):
                            unique_quiet.add(fen)

    print(f"Extraction summary from {total_games} games:")
    print(f"  Total unique positions (all):        {len(unique_all)}")
    print(f"  Unique non-check positions:          {len(unique_non_check)}")
    print(f"  Unique quiet positions (SEE <= 100): {len(unique_quiet)}")

    with open(output_file, "w", encoding="utf-8") as out:
        for fen in sorted(unique_quiet):
            out.write(fen + "\n")

    print(f"Saved {len(unique_quiet)} quiet positions to: {output_file}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
