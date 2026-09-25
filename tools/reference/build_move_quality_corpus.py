#!/usr/bin/env python3
"""Build the permanent deterministic move-quality reference corpus from PGN."""

import argparse
import collections
import csv
import json
from pathlib import Path

import chess
import chess.pgn

from tools.build_knight_outpost_sample import stable_hash

PHASE_VALUES = {chess.KNIGHT: 1, chess.BISHOP: 1, chess.ROOK: 2, chess.QUEEN: 4}
MATERIAL_VALUES = {chess.PAWN: 100, chess.KNIGHT: 350, chess.BISHOP: 350,
                   chess.ROOK: 550, chess.QUEEN: 975}


def classify(board):
    phase_value = min(24, sum(PHASE_VALUES.get(piece.piece_type, 0)
                              for piece in board.piece_map().values()))
    phase = "low" if phase_value <= 8 else "middle" if phase_value <= 16 else "high"
    material_value = sum(MATERIAL_VALUES.get(piece.piece_type, 0)
                         for piece in board.piece_map().values())
    material = "low" if material_value <= 2600 else "middle" if material_value <= 5600 else "high"
    queens = len(board.pieces(chess.QUEEN, chess.WHITE)) + len(board.pieces(chess.QUEEN, chess.BLACK))
    queen_state = "both" if queens >= 2 else "one" if queens == 1 else "none"
    legal = list(board.legal_moves)
    captures = sum(board.is_capture(move) for move in legal)
    return phase_value, phase, material_value, material, queen_state, legal, captures


def proportional_sample(candidates, count):
    strata = collections.defaultdict(list)
    for row in candidates:
        strata[(row["phase"], row["material_bucket"])].append(row)
    for rows in strata.values():
        rows.sort(key=lambda row: (stable_hash(row["fen"] + "|permanent-reference"), row["fen"]))
    exact = {key: len(rows) * count / len(candidates) for key, rows in strata.items()}
    quotas = {key: min(len(strata[key]), int(value)) for key, value in exact.items()}
    remaining = count - sum(quotas.values())
    priority = sorted(strata, key=lambda key: (-(exact[key] - quotas[key]), key))
    while remaining:
        progressed = False
        for key in priority:
            if quotas[key] < len(strata[key]):
                quotas[key] += 1; remaining -= 1; progressed = True
                if not remaining: break
        if not progressed: break
    result = [row for key in sorted(strata) for row in strata[key][:quotas[key]]]
    if len(result) != count:
        raise RuntimeError(f"selected {len(result)} positions, expected {count}")
    return sorted(result, key=lambda row: (stable_hash(row["fen"] + "|reference-order"), row["fen"]))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--pgn", default="tuner-corpus/twic1660.pgn")
    parser.add_argument("--output", default="move-quality-reference/corpus-1000.tsv")
    parser.add_argument("--audit-json", default="move-quality-reference/corpus-audit.json")
    parser.add_argument("--audit-tsv", default="move-quality-reference/corpus-audit.tsv")
    parser.add_argument("--count", type=int, default=1000)
    args = parser.parse_args()

    candidates = []
    seen_fens = set()
    games = 0
    with open(args.pgn, encoding="utf-8", errors="replace") as stream:
        while True:
            game = chess.pgn.read_game(stream)
            if game is None: break
            games += 1
            board = game.board()
            eligible = []
            for ply, move in enumerate(game.mainline_moves(), 1):
                board.push(move)
                if ply <= 12 or board.is_game_over() or not board.is_valid():
                    continue
                phase_value, phase, material_value, material, queen_state, legal, captures = classify(board)
                if len(legal) <= 1 or board.fen() in seen_fens:
                    continue
                eligible.append({"fen": board.fen(), "ply": ply, "phase_value": phase_value,
                    "phase": phase, "material_value": material_value, "material_bucket": material,
                    "piece_count": len(board.piece_map()),
                    "pawn_count": len(board.pieces(chess.PAWN, chess.WHITE)) + len(board.pieces(chess.PAWN, chess.BLACK)),
                    "queen_state": queen_state, "legal_moves": len(legal), "legal_captures": captures})
            eligible.sort(key=lambda row: (stable_hash(row["fen"] + f"|game-{games}"), row["ply"]))
            chosen = []
            for row in eligible:
                if all(abs(row["ply"] - prior["ply"]) >= 12 for prior in chosen):
                    chosen.append(row)
                    if len(chosen) == 2: break
            for slot, row in enumerate(chosen, 1):
                if row["fen"] in seen_fens: continue
                seen_fens.add(row["fen"])
                row["source_game"] = f"twic1660-{games:05d}"
                row["source_slot"] = slot
                candidates.append(row)

    selected = proportional_sample(candidates, args.count)
    for row in selected:
        row["position_id"] = f"mqr-{stable_hash(row['fen']):016x}"
    fields = ["position_id", "fen", "source_game", "source_slot", "ply", "phase_value", "phase",
              "material_value", "material_bucket", "piece_count", "pawn_count", "queen_state",
              "legal_moves", "legal_captures"]
    output = Path(args.output); output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t")
        writer.writeheader(); writer.writerows(selected)

    audit = {"source_pgn": str(Path(args.pgn).resolve()), "games_read": games,
             "candidate_positions": len(candidates), "selected_positions": len(selected),
             "phase_distribution": dict(collections.Counter(row["phase"] for row in selected)),
             "material_distribution": dict(collections.Counter(row["material_bucket"] for row in selected)),
             "queen_state_distribution": dict(collections.Counter(row["queen_state"] for row in selected)),
             "piece_count": {"minimum": min(row["piece_count"] for row in selected),
                             "maximum": max(row["piece_count"] for row in selected),
                             "mean": sum(row["piece_count"] for row in selected)/len(selected)},
             "pawn_count": {"minimum": min(row["pawn_count"] for row in selected),
                            "maximum": max(row["pawn_count"] for row in selected),
                            "mean": sum(row["pawn_count"] for row in selected)/len(selected)},
             "positions_with_legal_capture": sum(row["legal_captures"] > 0 for row in selected),
             "legal_capture_frequency": sum(row["legal_captures"] > 0 for row in selected)/len(selected),
             "maximum_positions_per_game": max(collections.Counter(row["source_game"] for row in selected).values())}
    Path(args.audit_json).write_text(json.dumps(audit, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    with open(args.audit_tsv, "w", encoding="utf-8") as stream:
        stream.write("metric\tvalue\n")
        for key, value in audit.items():
            stream.write(f"{key}\t{json.dumps(value, sort_keys=True)}\n")
    print(json.dumps(audit, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
