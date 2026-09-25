#!/usr/bin/env python3
"""Build the frozen, prevalence-weighted KnightOutpost search sample."""

import argparse
import collections
import json
from pathlib import Path

import chess

PREVALENCE = 0.087532
PIECE_VALUES = {
    chess.PAWN: 100,
    chess.KNIGHT: 350,
    chess.BISHOP: 350,
    chess.ROOK: 550,
    chess.QUEEN: 975,
}
PHASE_VALUES = {
    chess.KNIGHT: 1,
    chess.BISHOP: 1,
    chess.ROOK: 2,
    chess.QUEEN: 4,
}


def stable_hash(text):
    value = 1469598103934665603
    for byte in text.encode():
        value = ((value ^ byte) * 1099511628211) & ((1 << 64) - 1)
    return value


def category(entry):
    board = entry["board"]
    stm_result = entry["result"] if board.turn else 1.0 - entry["result"]
    result = "win" if stm_result == 1.0 else "loss" if stm_result == 0.0 else "draw"
    phase = min(24, sum(PHASE_VALUES.get(piece.piece_type, 0)
                        for piece in board.piece_map().values()))
    phase_group = "low" if phase <= 8 else "middle" if phase <= 16 else "high"
    queens = len(board.pieces(chess.QUEEN, chess.WHITE)) + len(board.pieces(chess.QUEEN, chess.BLACK))
    queen_group = "both" if queens >= 2 else "one" if queens == 1 else "none"
    material = sum(PIECE_VALUES.get(piece.piece_type, 0)
                   for piece in board.piece_map().values())
    material_group = "low" if material <= 2600 else "middle" if material <= 5600 else "high"
    return result, phase_group, queen_group, material_group


def distribution(entries):
    dimensions = ("result", "phase", "queen", "material")
    counters = [collections.Counter() for _ in dimensions]
    for entry in entries:
        for counter, value in zip(counters, entry["category"]):
            counter[value] += 1
    return {
        name: {key: count / len(entries) for key, count in sorted(counter.items())}
        for name, counter in zip(dimensions, counters)
    }


def proportional_strata(entries, count):
    strata = collections.defaultdict(list)
    for entry in entries:
        strata[entry["category"]].append(entry)
    for values in strata.values():
        values.sort(key=lambda item: (stable_hash(item["fen"] + "|anchor"), item["fen"]))
    exact = {key: len(values) * count / len(entries) for key, values in strata.items()}
    quotas = {key: min(len(strata[key]), int(value)) for key, value in exact.items()}
    remaining = count - sum(quotas.values())
    priority = sorted(strata, key=lambda key: (-(exact[key] - quotas[key]), key))
    while remaining:
        progressed = False
        for key in priority:
            if quotas[key] < len(strata[key]):
                quotas[key] += 1
                remaining -= 1
                progressed = True
                if not remaining:
                    break
        if not progressed:
            break
    selected = [entry for key in sorted(strata) for entry in strata[key][:quotas[key]]]
    if len(selected) != count:
        raise RuntimeError(f"only {len(selected)} eligible anchor positions")
    return selected


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--corpus", default="tuner-train.tsv")
    parser.add_argument("--active-fens", required=True)
    parser.add_argument("--output", default="tuner-samples/knight-outpost-weighted-500.tsv")
    parser.add_argument("--audit-output", default="tuner-samples/knight-outpost-weighted-500-audit.json")
    args = parser.parse_args()

    active_fens = {line.rstrip("\n") for line in open(args.active_fens, encoding="utf-8") if line.strip()}
    entries = []
    seen = set()
    with open(args.corpus, encoding="utf-8") as corpus:
        for line_number, line in enumerate(corpus, 1):
            if not line.strip():
                continue
            fen, result_text = line.rstrip("\n").rsplit("\t", 1)
            if stable_hash(fen) % 100 >= 70 or fen in seen:
                continue
            try:
                result = float(result_text)
                board = chess.Board(fen)
            except (ValueError, chess.InvalidMoveError):
                continue
            if result not in (0.0, 0.5, 1.0) or not board.is_valid() or board.is_game_over():
                continue
            seen.add(fen)
            entry = {"fen": fen, "result": result, "board": board,
                     "id": f"ko-{stable_hash(fen):016x}", "active": fen in active_fens}
            entry["category"] = category(entry)
            entries.append(entry)

    anchor = proportional_strata(entries, 250)
    anchor_fens = {entry["fen"] for entry in anchor}
    enriched_pool = [entry for entry in entries if entry["active"] and entry["fen"] not in anchor_fens]
    enriched_pool.sort(key=lambda item: (stable_hash(item["fen"] + "|enriched"), item["fen"]))
    enriched = enriched_pool[:250]
    if len(enriched) != 250:
        raise RuntimeError(f"only {len(enriched)} non-overlapping active positions")

    sample = [(entry, "anchor") for entry in anchor] + [(entry, "enriched") for entry in enriched]
    active_count = sum(entry["active"] for entry, _ in sample)
    inactive_count = len(sample) - active_count
    active_weight = PREVALENCE / active_count
    inactive_weight = (1.0 - PREVALENCE) / inactive_count

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="utf-8") as stream:
        stream.write("position_id\tfen\tresult\tgroup\tweight\n")
        for entry, group in sample:
            weight = active_weight if entry["active"] else inactive_weight
            stream.write(f'{entry["id"]}\t{entry["fen"]}\t{entry["result"]:.1f}\t{group}\t{weight:.17g}\n')

    audit = {
        "eligible_training_positions": len(entries),
        "anchor_positions": len(anchor),
        "enriched_positions": len(enriched),
        "anchor_active": sum(entry["active"] for entry in anchor),
        "enriched_active": sum(entry["active"] for entry in enriched),
        "full_training_active_rate": sum(entry["active"] for entry in entries) / len(entries),
        "target_prevalence": PREVALENCE,
        "effective_weighted_prevalence": active_count * active_weight,
        "active_position_weight": active_weight,
        "inactive_position_weight": inactive_weight,
        "full_distribution": distribution(entries),
        "anchor_distribution": distribution(anchor),
    }
    audit_output = Path(args.audit_output)
    audit_output.parent.mkdir(parents=True, exist_ok=True)
    audit_output.write_text(json.dumps(audit, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps(audit, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
