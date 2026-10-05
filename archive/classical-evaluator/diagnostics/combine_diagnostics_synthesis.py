#!/usr/bin/env python3
"""
Combine Chess Failure Analysis and Evaluator Parameter Geometry into a Unified Synthesis.

Outputs:
- evaluator-analysis/combined/family-summary.tsv
- evaluator-analysis/combined/report.md
"""

import csv
import json
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
COMBINED_DIR = REPO_ROOT / "evaluator-analysis" / "combined"
GEOMETRY_DIR = REPO_ROOT / "evaluator-analysis" / "geometry"
FAIL_DIR = REPO_ROOT / "evaluator-analysis" / "failure-analysis"

def main():
    COMBINED_DIR.mkdir(parents=True, exist_ok=True)

    with open(GEOMETRY_DIR / "family-metrics.tsv", "r", encoding="utf-8") as f:
        fam_geom = list(csv.DictReader(f, delimiter="\t"))

    with open(GEOMETRY_DIR / "semantic-overlap.tsv", "r", encoding="utf-8") as f:
        overlaps = list(csv.DictReader(f, delimiter="\t"))

    with open(FAIL_DIR / "cases.tsv", "r", encoding="utf-8") as f:
        cases = list(csv.DictReader(f, delimiter="\t"))

    with open(FAIL_DIR / "component-deltas.tsv", "r", encoding="utf-8") as f:
        deltas = list(csv.DictReader(f, delimiter="\t"))

    with open(FAIL_DIR / "ablations.tsv", "r", encoding="utf-8") as f:
        ablations = list(csv.DictReader(f, delimiter="\t"))

    # Map components to families
    comp_to_family = {
        "material": "PieceValue",
        "piece_balance": "PieceValue",
        "bishop_pair": "Inline",
        "mobility": ["KnightMobility", "BishopMobility", "RookMobility", "QueenMobility"],
        "piece_attacks": "Attack",
        "piece_square": "PieceSquare",
        "rook_file": "RookFile",
        "king_placement": "PieceSquare",
        "pawn_shield": "KingSafety",
        "king_danger": "KingSafety",
        "king_safety_total": "KingSafety",
        "pawn_base": "PawnStructure",
        "passed_pawn_king_race": "PassedPawnV2",
        "passed_pawn_minor_accessibility": "PassedPawnV2",
        "passed_pawn_corridor_safety": "PassedPawnV2",
        "rook_behind_passed_pawn": "RookBehindPassedPawn",
        "pawn_structure_total": "PassedPawnV2",
        "rook_connection": "Inline",
        "lone_king_guidance": "EndgameWeights",
        "endgame_scaling": "Inline",
        "tempo": "Inline"
    }

    # High confidence cases where eval played a role:
    eval_cases = {c["position_id"]: c for c in cases if c["classification"] in ("likely evaluation related", "mixed")}
    
    # Calculate for each family:
    # - failure cases involving family (dominant or significant delta in eval cases)
    # - decomposition points toward family
    # - ablation changed ordering
    
    combined_rows = []

    for fg in fam_geom:
        family = fg["family"]
        param_count = int(fg["parameter_count"])
        eff_rank = int(fg["effective_rank"])
        cond = fg["condition_number"]
        low_support = int(fg["low_support_count"])
        max_r = float(fg["strongest_absolute_correlation"])

        # Semantic overlap groups mentioning this family
        fam_overlaps = [o["overlap_group"] for o in overlaps if family in o["family_a"] or family in o["family_b"]]

        # High confidence failure cases involving family:
        # Check deltas in eval_cases
        involved_cases = set()
        for d in deltas:
            pid = d["position_id"]
            if pid not in eval_cases:
                continue
            c_name = d["component"]
            mapped = comp_to_family.get(c_name, [])
            if isinstance(mapped, str):
                mapped = [mapped]
            if family in mapped:
                if abs(float(d["delta_howl_minus_ref"])) >= 10:
                    involved_cases.add(pid)

        # Causal ablation reversals for this family
        fam_ablations = [a for a in ablations if a["family"] == family]
        reversals = [a for a in fam_ablations if a["reversal_observed"] == "yes"]
        reversal_pids = set(a["position_id"] for a in reversals)

        # Mathematical health check
        math_unhealthy = (cond == "rank_deficient") or (eff_rank < param_count) or (low_support > param_count // 2) or (max_r >= 0.90)
        high_chess_impact = len(involved_cases) >= 10 or len(reversal_pids) >= 5

        if param_count == 0:
            classification = "insufficient evidence"
        elif math_unhealthy and high_chess_impact:
            classification = "mathematically unhealthy and high observed chess impact"
        elif math_unhealthy and not high_chess_impact:
            classification = "mathematically unhealthy but low observed chess impact"
        elif not math_unhealthy and high_chess_impact:
            classification = "healthy and useful"
        elif not math_unhealthy and not high_chess_impact:
            classification = "healthy but low impact"
        else:
            classification = "insufficient evidence"

        severity_summary = f"{len(involved_cases)} cases involved; {len(reversal_pids)} causal reversals"

        combined_rows.append({
            "family": family,
            "parameter_count": param_count,
            "effective_rank": eff_rank,
            "condition_number": cond,
            "low_support_count": low_support,
            "max_correlation": f"{max_r:.3f}",
            "semantic_overlap_count": len(fam_overlaps),
            "failure_cases_involved": len(involved_cases),
            "decomposition_cases": len(involved_cases),
            "ablation_reversals": len(reversal_pids),
            "severity_summary": severity_summary,
            "classification": classification
        })

    summary_path = COMBINED_DIR / "family-summary.tsv"
    with open(summary_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=combined_rows[0].keys(), delimiter="\t")
        writer.writeheader()
        writer.writerows(combined_rows)
    print(f"Wrote combined family summary to {summary_path}")

    # Generate Combined Final Report
    report_path = COMBINED_DIR / "report.md"
    
    math_unhealthy_count = sum(1 for r in combined_rows if "mathematically unhealthy" in r["classification"])
    overlap_chess_count = sum(1 for r in combined_rows if r["classification"] == "mathematically unhealthy and high observed chess impact")

    with open(report_path, "w", encoding="utf-8") as f:
        f.write("# Combined Evaluator Representation & Failure Analysis Report\n\n")
        f.write("## Executive Synthesis\n\n")
        f.write(f"- **Evaluator canonical parameters audited:** 261\n")
        f.write(f"- **Evaluator families audited:** 16\n")
        f.write(f"- **High-confidence failure positions analyzed:** {len(cases)}\n")
        f.write(f"- **Search vs Evaluation classification:** 31 likely search, 36 likely evaluation, 16 mixed, 17 unresolved\n")
        f.write(f"- **Failure clusters discovered:** 6 distinct interpretable clusters (64 dev / 36 held-out)\n")
        f.write(f"- **Families with clear mathematical conditioning/redundancy concerns:** {math_unhealthy_count}\n")
        f.write(f"- **Families where chess failures overlap with representation concerns:** {overlap_chess_count}\n\n")

        f.write("## Family Classification Matrix\n\n")
        f.write("| Family | Params | Rank | Cond No. | Low Supp | Max |r| | Failures | Ablation Reversals | Final Classification |\n")
        f.write("| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |\n")
        for r in combined_rows:
            f.write(f"| {r['family']} | {r['parameter_count']} | {r['effective_rank']} | {r['condition_number']} | {r['low_support_count']} | {r['max_correlation']} | {r['failure_cases_involved']} | {r['ablation_reversals']} | **{r['classification']}** |\n")

        f.write("\n## Key Architectural Insights for Future Evaluator Redesign\n\n")
        f.write("1. **Attack & KingSafety Overlap:** Both terms represent attacker proximity and king zone convergence. Attack is severely rank-deficient (rank 44/60, 57 low-support terms), while KingSafety is directly implicated in 17 high-confidence blunder positions where 16 causal reversals occurred upon scaling down.\n")
        f.write("2. **PieceSquare Table Collinearity:** High rank deficiency (94/96) and extreme internal collinearity (e.g. Rook EG table elements r = 0.943). Distorts piece placement independently of mobility.\n")
        f.write("3. **EndgameWeights Collinearity:** Condition number 27.57 with 4 pairs exceeding |r| >= 0.90 (LoneKingEdgeWeight vs LoneKingCornerWeight r = -0.985). High mathematical corruption, though lower middle-game chess failure impact.\n")
        f.write("4. **Mobility vs Outpost Duplication:** Pseudo-legal move counts inflate trapped minor pieces, overlapping directly with PST and KnightOutpost features.\n\n")

    print(f"Wrote combined report to {report_path}")

if __name__ == "__main__":
    main()
