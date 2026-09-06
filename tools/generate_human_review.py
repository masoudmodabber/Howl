#!/usr/bin/env python3
"""
Generate human review tables for Howl diagnostic results.

Produces:
1. diagnostics/results/human_review.tsv: Machine-readable raw scores (types and values separated).
2. diagnostics/results/human_review_readable.tsv: Human-readable pawn-unit scores (+0.62, -5.13, +M1, -M2).
"""

import argparse
import csv
from pathlib import Path
from typing import Dict, List, Optional, Tuple


def format_score_readable(score_type: str, score_value: str) -> str:
    """Format score as readable pawn units (+0.62, -5.13) or mate (+M1, -M2)."""
    if not score_type or not score_value:
        return ""
    if score_type == "mate":
        try:
            val = int(score_value)
            if val > 0:
                return f"+M{val}"
            elif val < 0:
                return f"-M{abs(val)}"
            else:
                return "M0"
        except ValueError:
            return f"M{score_value}"
    elif score_type == "cp":
        try:
            val = int(score_value)
            pawns = val / 100.0
            if pawns > 0:
                return f"+{pawns:.2f}"
            elif pawns < 0:
                return f"-{abs(pawns):.2f}"
            else:
                return "+0.00"
        except ValueError:
            return score_value
    else:
        return f"{score_type} {score_value}"


def generate_human_review(
    howl_file: str = "diagnostics/results/howl_diagnostics_20s.tsv",
    reference_file: str = "diagnostics/reference.tsv",
    disagreements_file: str = "diagnostics/results/stockfish_howl_disagreements.tsv",
    output_raw: str = "diagnostics/results/human_review.tsv",
    output_readable: str = "diagnostics/results/human_review_readable.tsv",
) -> Tuple[int, str, str, List[str]]:
    howl_path = Path(howl_file)
    ref_path = Path(reference_file)
    disag_path = Path(disagreements_file)

    if not howl_path.exists():
        raise FileNotFoundError(f"Howl diagnostics file not found: {howl_file}")
    if not ref_path.exists():
        raise FileNotFoundError(f"Reference file not found: {reference_file}")
    if not disag_path.exists():
        raise FileNotFoundError(f"Disagreements file not found: {disagreements_file}")

    howl_data: Dict[str, Dict[str, str]] = {}
    with open(howl_path, "r", encoding="utf-8") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            howl_data[row["id"]] = row

    ref_data: Dict[str, Dict[str, str]] = {}
    with open(ref_path, "r", encoding="utf-8") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            ref_data[row["id"]] = row

    disag_data: Dict[str, Dict[str, str]] = {}
    with open(disag_path, "r", encoding="utf-8") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            disag_data[row["id"]] = row

    # Sorted list of position IDs
    pos_ids = sorted(howl_data.keys(), key=lambda x: int(x.split("_")[-1]) if "_" in x and x.split("_")[-1].isdigit() else x)

    raw_fieldnames = [
        "id",
        "game_phase",
        "fen",
        "howl_move",
        "howl_score_type",
        "howl_score_value",
        "howl_depth",
        "howl_nodes",
        "stockfish_move_1",
        "stockfish_score_1_type",
        "stockfish_score_1_value",
        "stockfish_move_2",
        "stockfish_score_2_type",
        "stockfish_score_2_value",
        "stockfish_move_3",
        "stockfish_score_3_type",
        "stockfish_score_3_value",
        "stockfish_rank_of_howl_move",
        "stockfish_score_for_howl_move_type",
        "stockfish_score_for_howl_move_value",
        "human_assessment",
        "human_notes",
    ]

    readable_fieldnames = [
        "id",
        "game_phase",
        "fen",
        "howl_move",
        "howl_score",
        "howl_depth",
        "howl_nodes",
        "stockfish_move_1",
        "stockfish_score_1",
        "stockfish_move_2",
        "stockfish_score_2",
        "stockfish_move_3",
        "stockfish_score_3",
        "stockfish_rank_of_howl_move",
        "stockfish_score_for_howl_move",
        "human_assessment",
        "human_notes",
    ]

    raw_rows: List[Dict[str, str]] = []
    readable_rows: List[Dict[str, str]] = []
    missing_scores: List[str] = []

    for pid in pos_ids:
        h = howl_data[pid]
        r = ref_data.get(pid, {})
        d = disag_data.get(pid, {})

        game_phase = h.get("game_phase", "")
        fen = h.get("fen", "")
        howl_move = h.get("best_move", "")
        howl_score_type = h.get("score_type", "")
        howl_score_value = h.get("score_value", "")
        howl_depth = h.get("completed_depth", "")
        howl_nodes = h.get("nodes", "")

        sf_m1 = r.get("stockfish_move_1", "")
        sf_s1_t = r.get("stockfish_score_1_type", "")
        sf_s1_v = r.get("stockfish_score_1_value", "")

        sf_m2 = r.get("stockfish_move_2", "")
        sf_s2_t = r.get("stockfish_score_2_type", "")
        sf_s2_v = r.get("stockfish_score_2_value", "")

        sf_m3 = r.get("stockfish_move_3", "")
        sf_s3_t = r.get("stockfish_score_3_type", "")
        sf_s3_v = r.get("stockfish_score_3_value", "")

        sf_rank = h.get("stockfish_rank_of_howl_move", "")

        # Determine Stockfish score for Howl's move
        if sf_rank == "1":
            sf_howl_t = sf_s1_t
            sf_howl_v = sf_s1_v
        elif sf_rank == "2":
            sf_howl_t = sf_s2_t
            sf_howl_v = sf_s2_v
        elif sf_rank == "3":
            sf_howl_t = sf_s3_t
            sf_howl_v = sf_s3_v
        elif sf_rank == "outside_top_3":
            sf_howl_t = d.get("targeted_howl_score_type", "")
            sf_howl_v = d.get("targeted_howl_score_value", "")
        else:
            sf_howl_t = ""
            sf_howl_v = ""

        if not sf_howl_t or not sf_howl_v:
            missing_scores.append(f"{pid} (rank={sf_rank})")

        raw_row = {
            "id": pid,
            "game_phase": game_phase,
            "fen": fen,
            "howl_move": howl_move,
            "howl_score_type": howl_score_type,
            "howl_score_value": howl_score_value,
            "howl_depth": howl_depth,
            "howl_nodes": howl_nodes,
            "stockfish_move_1": sf_m1,
            "stockfish_score_1_type": sf_s1_t,
            "stockfish_score_1_value": sf_s1_v,
            "stockfish_move_2": sf_m2,
            "stockfish_score_2_type": sf_s2_t,
            "stockfish_score_2_value": sf_s2_v,
            "stockfish_move_3": sf_m3,
            "stockfish_score_3_type": sf_s3_t,
            "stockfish_score_3_value": sf_s3_v,
            "stockfish_rank_of_howl_move": sf_rank,
            "stockfish_score_for_howl_move_type": sf_howl_t,
            "stockfish_score_for_howl_move_value": sf_howl_v,
            "human_assessment": "",
            "human_notes": "",
        }
        raw_rows.append(raw_row)

        readable_row = {
            "id": pid,
            "game_phase": game_phase,
            "fen": fen,
            "howl_move": howl_move,
            "howl_score": format_score_readable(howl_score_type, howl_score_value),
            "howl_depth": howl_depth,
            "howl_nodes": howl_nodes,
            "stockfish_move_1": sf_m1,
            "stockfish_score_1": format_score_readable(sf_s1_t, sf_s1_v),
            "stockfish_move_2": sf_m2,
            "stockfish_score_2": format_score_readable(sf_s2_t, sf_s2_v),
            "stockfish_move_3": sf_m3,
            "stockfish_score_3": format_score_readable(sf_s3_t, sf_s3_v),
            "stockfish_rank_of_howl_move": sf_rank,
            "stockfish_score_for_howl_move": format_score_readable(sf_howl_t, sf_howl_v),
            "human_assessment": "",
            "human_notes": "",
        }
        readable_rows.append(readable_row)

    out_raw_path = Path(output_raw)
    out_raw_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_raw_path, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=raw_fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(raw_rows)

    out_readable_path = Path(output_readable)
    out_readable_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_readable_path, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=readable_fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(readable_rows)

    return len(raw_rows), str(out_raw_path), str(out_readable_path), missing_scores


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate human review tables.")
    parser.add_argument(
        "--howl-results",
        default="diagnostics/results/howl_diagnostics_20s.tsv",
        help="Howl diagnostics TSV path",
    )
    parser.add_argument(
        "--reference",
        default="diagnostics/reference.tsv",
        help="Stockfish reference TSV path",
    )
    parser.add_argument(
        "--disagreements",
        default="diagnostics/results/stockfish_howl_disagreements.tsv",
        help="Stockfish disagreements TSV path",
    )
    parser.add_argument(
        "--output-raw",
        default="diagnostics/results/human_review.tsv",
        help="Output path for human_review.tsv",
    )
    parser.add_argument(
        "--output-readable",
        default="diagnostics/results/human_review_readable.tsv",
        help="Output path for human_review_readable.tsv",
    )

    args = parser.parse_args()

    count, raw_path, readable_path, missing = generate_human_review(
        howl_file=args.howl_results,
        reference_file=args.reference,
        disagreements_file=args.disagreements,
        output_raw=args.output_raw,
        output_readable=args.output_readable,
    )

    print(f"Human review tables generated successfully ({count} rows).")
    print(f"Raw TSV: {raw_path}")
    print(f"Readable TSV: {readable_path}")
    if missing:
        print(f"Missing score cases: {missing}")
    else:
        print("Missing score cases: None")


if __name__ == "__main__":
    main()
