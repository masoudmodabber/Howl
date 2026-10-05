#!/usr/bin/env python3
"""
Audit the mathematical structure and geometry of Howl's 261 canonical evaluator parameters.
Produces:
- evaluator-analysis/geometry/parameter-metrics.tsv
- evaluator-analysis/geometry/family-metrics.tsv
- evaluator-analysis/geometry/correlations.tsv
- evaluator-analysis/geometry/semantic-overlap.tsv
- evaluator-analysis/geometry/report.md
"""

import csv
import math
import os
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
OUTPUT_DIR = REPO_ROOT / "evaluator-analysis" / "geometry"
CORPUS_PATH = REPO_ROOT / "move-quality-reference" / "corpus-1000.tsv"

def run_identifiability():
    """Ensure howl_eval_identifiability has been run on corpus-1000.tsv."""
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    binary = REPO_ROOT / "build" / "howl_eval_identifiability"
    if not binary.exists():
        subprocess.run(["cmake", "--build", str(REPO_ROOT / "build"), "--target", "howl_eval_identifiability"], check=True)
    
    cmd = [
        str(binary),
        "--corpus", str(CORPUS_PATH),
        "--output-dir", str(OUTPUT_DIR),
        "--limit", "1000"
    ]
    print(f"Running: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)

def generate_parameter_metrics():
    inventory_path = OUTPUT_DIR / "parameter-inventory.tsv"
    stats_path = OUTPUT_DIR / "parameter-statistics.tsv"
    
    with open(inventory_path, "r", encoding="utf-8") as f:
        inv = {row["name"]: row for row in csv.DictReader(f, delimiter="\t")}
    
    with open(stats_path, "r", encoding="utf-8") as f:
        stats = list(csv.DictReader(f, delimiter="\t"))
        
    out_rows = []
    for s in stats:
        name = s["name"]
        item_inv = inv.get(name, {})
        family = s["family"]
        cur_val = int(s["current_value"])
        nonzero = int(s["nonzero_positions"])
        fraction = float(s["nonzero_fraction"])
        mean_val = float(s["mean"])
        std_val = float(s["standard_deviation"])
        min_val = int(s["minimum"])
        max_val = int(s["maximum"])
        low_support = s["low_support"]
        phase = item_inv.get("phase", "Both/Scalar")
        shape = item_inv.get("shape", "scalar")
        linear_exact = s["linear_exact"]
        
        # Mean absolute contribution per position (std_val * 1 unit approx or abs(mean))
        # Sensitivity: evaluator output change per unit parameter change is the feature value itself (mean |feature| or std)
        mean_abs_feature = float(s["positive"]) + float(s["negative"])
        # Granularity is integer (1 unit)
        granularity = 1
        
        out_rows.append({
            "name": name,
            "family": family,
            "phase": phase,
            "shape": shape,
            "current_value": cur_val,
            "linear_exact": linear_exact,
            "activation_count": nonzero,
            "activation_frequency": f"{fraction:.4f}",
            "mean_feature": f"{mean_val:.4f}",
            "standard_deviation": f"{std_val:.4f}",
            "variance": f"{std_val**2:.4f}",
            "min_feature": min_val,
            "max_feature": max_val,
            "granularity": granularity,
            "low_support": low_support
        })
        
    metrics_path = OUTPUT_DIR / "parameter-metrics.tsv"
    with open(metrics_path, "w", newline="", encoding="utf-8") as f:
        fieldnames = [
            "name", "family", "phase", "shape", "current_value", "linear_exact",
            "activation_count", "activation_frequency", "mean_feature",
            "standard_deviation", "variance", "min_feature", "max_feature",
            "granularity", "low_support"
        ]
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(out_rows)
    print(f"Wrote {len(out_rows)} parameters to {metrics_path}")

def generate_family_metrics():
    summary_path = OUTPUT_DIR / "family-summary.tsv"
    with open(summary_path, "r", encoding="utf-8") as f:
        fams = list(csv.DictReader(f, delimiter="\t"))
        
    out_fams = []
    for r in fams:
        out_fams.append(r)
        
    metrics_path = OUTPUT_DIR / "family-metrics.tsv"
    with open(metrics_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fams[0].keys(), delimiter="\t")
        writer.writeheader()
        writer.writerows(out_fams)
    print(f"Wrote {len(out_fams)} families to {metrics_path}")

def generate_correlations():
    corr_in = OUTPUT_DIR / "family-correlations.tsv"
    with open(corr_in, "r", encoding="utf-8") as f:
        corrs = list(csv.DictReader(f, delimiter="\t"))
    
    corr_out = OUTPUT_DIR / "correlations.tsv"
    with open(corr_out, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=corrs[0].keys(), delimiter="\t")
        writer.writeheader()
        writer.writerows(corrs)
    print(f"Wrote {len(corrs)} high correlation pairs to {corr_out}")

def generate_semantic_overlap():
    """
    Grounds semantic overlaps directly in Howl C++ source code formulas and architecture.
    """
    overlaps = [
        {
            "overlap_group": "King Attack vs King Safety vs Pawn Shield",
            "family_a": "Attack",
            "family_b": "KingSafety",
            "terms_a": "KnightAttackValue, BishopAttackValue, RookAttackValue, QueenAttackValue",
            "terms_b": "KingAttacker[Pawn/Minor/Rook/Queen]Weight, KingDefenderWeights, KingShelterDanger",
            "mechanism": "King zone pressure and attacker count are counted in both Attack (PieceMoveCountFast) and KingSafety (EvaluateKingDanger). Both evaluate attacking pieces converging on the opponent king.",
            "source_reference": "EvaluationLogic.cpp:2150-2220, 2600-2980; KingSetup.cpp"
        },
        {
            "overlap_group": "PST King Placement vs King Safety Shelter",
            "family_a": "PieceSquare",
            "family_b": "KingSafety",
            "terms_a": "KingInValueWhiteMiddleGame, WhiteKingPlaceSafetyMiddleGame",
            "terms_b": "KingShelterSecondRankDanger, KingShelterMissingPawnDanger, KingShelterOpenFileDanger",
            "mechanism": "King PST penalizes or rewards king squares based on castling destinations (g1/c1 vs e1/d1), while KingSafety independently scores pawn shelter defects, open files, and missing shield pawns on those exact same destination files.",
            "source_reference": "EvaluationLogic.cpp:2162-2206"
        },
        {
            "overlap_group": "Mobility vs PieceSquare vs Outpost",
            "family_a": "KnightMobility / BishopMobility / RookMobility",
            "family_b": "PieceSquare / KnightOutpost",
            "terms_a": "KnightMoveCountValue, BishopMoveCountValue, RookMoveCountValue",
            "terms_b": "KnightPieceSquare, BishopPieceSquare, KnightOutpost[MiddleGame/EndGame]",
            "mechanism": "Centralizing a minor piece simultaneously triggers higher PST values, higher available pseudo-legal move counts (Mobility), and KnightOutpost bonuses if placed on advanced protected squares.",
            "source_reference": "EvaluationLogic.cpp:2650-2710, 2828-2890; Option.cpp"
        },
        {
            "overlap_group": "Rook Open File vs Rook Mobility vs Rook PST",
            "family_a": "RookFile",
            "family_b": "RookMobility / PieceSquare",
            "terms_a": "RookOpenFileMiddleGame, RookSemiOpenFileMiddleGame",
            "terms_b": "RookMoveCountValue, RookPieceSquareMiddleGame",
            "mechanism": "An open or semi-open file provides open vertical rays, which directly elevates RookMoveCountValue popcounts, yields explicit RookFile bonuses, and overlaps with 7th/8th rank PST placements.",
            "source_reference": "EvaluationLogic.cpp:2154-2156, 2730-2750, 2900-2908"
        },
        {
            "overlap_group": "Passed Pawn Advancement vs Endgame Scaling",
            "family_a": "PassedPawnV2",
            "family_b": "EndgameWeights / Inline",
            "terms_a": "PassedPawnMiddleGameIncrement, PassedPawnEndGameIncrement",
            "terms_b": "EndgamePawnAdvancementRankMultiplier, OppositeColorBishopScale",
            "mechanism": "Passed pawn rank increments are scaled nonlinearly in PassedPawnV2, while endgame evaluation independently applies advancement multipliers and taper curves.",
            "source_reference": "EvaluationLogic.cpp:2230-2246, 580-606"
        },
        {
            "overlap_group": "Lone King Mate Confinement vs Edge/Corner",
            "family_a": "EndgameWeights",
            "family_b": "EndgameWeights",
            "terms_a": "LoneKingEdgeWeight, LoneKingCornerWeight",
            "terms_b": "LoneKingRestrictedNeighbourWeight, LoneKingBase",
            "mechanism": "Internal family collinearity (|r| > 0.98): LoneKingEdgeWeight and LoneKingCornerWeight have r = -0.985, and both collinear with RestrictedNeighbourWeight (r = 0.935). All represent geometric proximity of the lone king to board boundaries.",
            "source_reference": "EvaluationLogic.cpp:2386-2392; family-correlations.tsv"
        }
    ]
    
    path = OUTPUT_DIR / "semantic-overlap.tsv"
    with open(path, "w", newline="", encoding="utf-8") as f:
        fieldnames = ["overlap_group", "family_a", "family_b", "terms_a", "terms_b", "mechanism", "source_reference"]
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(overlaps)
    print(f"Wrote {len(overlaps)} semantic overlap groups to {path}")

def generate_report():
    with open(OUTPUT_DIR / "family-metrics.tsv", "r", encoding="utf-8") as f:
        fams = list(csv.DictReader(f, delimiter="\t"))
    with open(OUTPUT_DIR / "correlations.tsv", "r", encoding="utf-8") as f:
        corrs = list(csv.DictReader(f, delimiter="\t"))
    with open(OUTPUT_DIR / "semantic-overlap.tsv", "r", encoding="utf-8") as f:
        overlaps = list(csv.DictReader(f, delimiter="\t"))
    with open(OUTPUT_DIR / "parameter-metrics.tsv", "r", encoding="utf-8") as f:
        params = list(csv.DictReader(f, delimiter="\t"))

    low_support_total = sum(1 for p in params if p["low_support"] == "yes")
    rank_deficient_fams = [f["family"] for f in fams if f["condition_number"] == "rank_deficient"]
    
    report_path = OUTPUT_DIR / "report.md"
    with open(report_path, "w", encoding="utf-8") as f:
        f.write("# Evaluator Parameter Geometry & Identifiability Audit\n\n")
        f.write("## Executive Summary\n")
        f.write(f"- **Total canonical parameters audited:** 261\n")
        f.write(f"- **Total evaluator families audited:** {len(fams)}\n")
        f.write(f"- **Rank-deficient families:** {len(rank_deficient_fams)} ({', '.join(rank_deficient_fams)})\n")
        f.write(f"- **Total low-support parameters (<5% active or near-zero variance):** {low_support_total} / 261 ({low_support_total / 261 * 100:.1f}%)\n")
        f.write(f"- **High internal collinearity pairs (|r| >= 0.90):** {len(corrs)}\n")
        f.write(f"- **Semantic overlap groups identified:** {len(overlaps)}\n\n")
        
        f.write("## Family Identifiability Summary\n\n")
        f.write("| Family | Params | Rank | Condition No. | Low Support | Max |r| | Classification |\n")
        f.write("| :--- | :--- | :--- | :--- | :--- | :--- | :--- |\n")
        for fam in fams:
            cond = fam["condition_number"]
            if cond != "rank_deficient":
                try:
                    cond = f"{float(cond):.2f}"
                except ValueError:
                    pass
            f.write(f"| {fam['family']} | {fam['parameter_count']} | {fam['effective_rank']} | {cond} | {fam['low_support_count']} | {float(fam['strongest_absolute_correlation']):.3f} | {fam['classification']} |\n")
            
        f.write("\n## Severe Collinearity Highlights\n\n")
        for c in corrs:
            f.write(f"- **{c['family']}**: `{c['parameter_a']}` vs `{c['parameter_b']}` (r = {float(c['correlation']):.4f}, {c['threshold_band']})\n")
            
        f.write("\n## Semantic Overlap Groups\n\n")
        for o in overlaps:
            f.write(f"### {o['overlap_group']}\n")
            f.write(f"- **Families:** `{o['family_a']}` & `{o['family_b']}`\n")
            f.write(f"- **Terms:** `{o['terms_a']}` vs `{o['terms_b']}`\n")
            f.write(f"- **Source Reference:** `{o['source_reference']}`\n")
            f.write(f"- **Mechanism:** {o['mechanism']}\n\n")
            
    print(f"Generated comprehensive report at {report_path}")

def main():
    run_identifiability()
    generate_parameter_metrics()
    generate_family_metrics()
    generate_correlations()
    generate_semantic_overlap()
    generate_report()

if __name__ == "__main__":
    main()
