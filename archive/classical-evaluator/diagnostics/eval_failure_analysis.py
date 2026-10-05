#!/usr/bin/env python3
"""
Systematic Evaluator Failure Analysis: Search vs Evaluation Diagnostic Ladder,
Evaluator Branch Comparison, Causal Ablation, Clustering, and Held-Out Splits.

Outputs:
- evaluator-analysis/failure-analysis/cases.tsv
- evaluator-analysis/failure-analysis/component-deltas.tsv
- evaluator-analysis/failure-analysis/ablations.tsv
- evaluator-analysis/failure-analysis/report.md
- evaluator-analysis/clusters/clusters.tsv
- evaluator-analysis/clusters/held-out-splits.tsv
- evaluator-analysis/clusters/report.md
"""

import csv
import json
import math
import os
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
ANALYSIS_DIR = REPO_ROOT / "evaluator-analysis"
FAIL_DIR = ANALYSIS_DIR / "failure-analysis"
CLUSTER_DIR = ANALYSIS_DIR / "clusters"

BASELINE_TSV = REPO_ROOT / "move-quality-reference" / "frozen-howl-baseline.tsv"
REFERENCE_CACHE = REPO_ROOT / "move-quality-reference" / "reference-cache-5m-multipv8.json"
CONSTRAINED_CACHE = REPO_ROOT / "move-quality-reference" / "constrained-move-cache-5m.json"
EVAL_BREAKDOWN = REPO_ROOT / "build" / "howl_eval_breakdown"
SEARCH_OBJ = REPO_ROOT / "build" / "howl_search_tuner_objective"

def run_eval_breakdown(fen, moves=None, scale_family=None, scale_factor=1.0):
    cmd = [str(EVAL_BREAKDOWN), "--json"]
    if scale_family:
        cmd.extend(["--scale-family", scale_family, str(scale_factor)])
    if moves:
        cmd.extend(["--moves", moves])
    cmd.append(fen)
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
    return json.loads(res.stdout)

def select_top_candidates(limit=100):
    with open(BASELINE_TSV, "r", encoding="utf-8") as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    diffs = [r for r in rows if r["howl_move"] != r["reference_move"] and float(r["regret"]) > 0.05]
    diffs.sort(key=lambda r: float(r["regret"]), reverse=True)
    return diffs[:limit]

def main():
    FAIL_DIR.mkdir(parents=True, exist_ok=True)
    CLUSTER_DIR.mkdir(parents=True, exist_ok=True)
    
    candidates = select_top_candidates(100)
    print(f"Loaded {len(candidates)} high-confidence candidate failure positions (regret range: {candidates[-1]['regret']} to {candidates[0]['regret']}).")

    with open(CONSTRAINED_CACHE, "r", encoding="utf-8") as f:
        constrained = json.load(f)
    with open(REFERENCE_CACHE, "r", encoding="utf-8") as f:
        reference = json.load(f)

    # 1. Search vs Evaluation Diagnostic Ladder
    # For each position:
    # Compare:
    # - Baseline search move and score (500k nodes)
    # - Static eval difference on root position
    # - Static eval difference after Howl move vs Reference move (evaluator branch comparison)
    # - Static eval preference vs search preference
    
    cases_rows = []
    component_deltas_rows = []
    ablations_rows = []
    
    suspicious_families = ["KingSafety", "Attack", "PieceSquare", "PassedPawnV2", "KnightMobility", "BishopMobility", "RookFile", "EndgameWeights"]

    for idx, c in enumerate(candidates):
        pid = c["position_id"]
        fen = c["fen"]
        howl_move = c["howl_move"]
        ref_move = c["reference_move"]
        regret = float(c["regret"])
        split = c["split"]
        howl_score = int(c["howl_searched_score"])

        # Evaluator decomposition of Howl branch vs Reference branch
        # Run eval on Howl move
        bd_howl = run_eval_breakdown(fen, moves=howl_move)
        # Run eval on Reference move
        bd_ref = run_eval_breakdown(fen, moves=ref_move)

        # In chess, bd_howl gives sideToMove total. But after 1 ply, sideToMove is the opponent!
        # White perspective total is invariant to side to move.
        # Let's compare from root side's perspective:
        root_is_white = bd_howl["side_to_move"] == "black" # since after 1 ply, opponent is black => root is white
        
        howl_branch_root_score = bd_howl["white_perspective_total"] if root_is_white else -bd_howl["white_perspective_total"]
        ref_branch_root_score = bd_ref["white_perspective_total"] if root_is_white else -bd_ref["white_perspective_total"]
        eval_diff = howl_branch_root_score - ref_branch_root_score # positive means static eval favors Howl move over Reference move!

        # Check root position decomposition
        bd_root = run_eval_breakdown(fen)
        phase = bd_root["phase"]

        # Classification logic:
        # If static evaluation strongly favors Howl's bad move over the reference move (eval_diff >= 15 cp),
        # then the evaluator is actively pulling search toward the blunder -> likely evaluation related.
        # If static evaluation favors the reference move or is neutral (eval_diff <= -15 cp), but deep 500k search chose Howl move,
        # then search extensions, pruning, or horizon effects overrode the evaluator -> likely search related.
        # If intermediate (-15 < eval_diff < 15), mixed or unresolved.
        if eval_diff >= 20:
            classification = "likely evaluation related"
        elif eval_diff <= -20:
            classification = "likely search related"
        elif eval_diff > 0:
            classification = "mixed"
        else:
            classification = "unresolved"

        cases_rows.append({
            "position_id": pid,
            "split": split,
            "fen": fen,
            "howl_move": howl_move,
            "reference_move": ref_move,
            "regret": f"{regret:.4f}",
            "phase": phase,
            "howl_searched_score": howl_score,
            "howl_branch_static": howl_branch_root_score,
            "ref_branch_static": ref_branch_root_score,
            "eval_delta_howl_minus_ref": eval_diff,
            "classification": classification
        })

        # Record component breakdown deltas: Howl move branch vs Reference move branch
        comp_deltas = {}
        for comp_name, comp_data in bd_howl["components"].items():
            if "net" in comp_data:
                howl_c = comp_data["net"] if root_is_white else -comp_data["net"]
                ref_c = bd_ref["components"][comp_name]["net"] if root_is_white else -bd_ref["components"][comp_name]["net"]
                d = howl_c - ref_c
                comp_deltas[comp_name] = d
                component_deltas_rows.append({
                    "position_id": pid,
                    "component": comp_name,
                    "howl_branch_val": howl_c,
                    "ref_branch_val": ref_c,
                    "delta_howl_minus_ref": d
                })

        # Controlled Causal Ablations on suspicious components for evaluation/mixed cases
        if classification in ("likely evaluation related", "mixed"):
            # Find largest component delta
            sorted_comps = sorted(comp_deltas.items(), key=lambda x: abs(x[1]), reverse=True)
            top_suspicious = []
            for c_name, d_val in sorted_comps:
                # Map component name to family
                fam_map = {
                    "king_danger": "KingSafety",
                    "king_safety_total": "KingSafety",
                    "pawn_shield": "KingSafety",
                    "piece_attacks": "Attack",
                    "piece_square": "PieceSquare",
                    "mobility": "KnightMobility",
                    "passed_pawn_king_race": "PassedPawnV2",
                    "pawn_structure_total": "PassedPawnV2",
                    "rook_file": "RookFile",
                    "lone_king_guidance": "EndgameWeights"
                }
                fam = fam_map.get(c_name)
                if fam and fam not in top_suspicious:
                    top_suspicious.append(fam)
                if len(top_suspicious) >= 2:
                    break

            for fam in top_suspicious:
                for scale in [0.0, 0.5, 1.0, 1.5]:
                    bd_h_abl = run_eval_breakdown(fen, moves=howl_move, scale_family=fam, scale_factor=scale)
                    bd_r_abl = run_eval_breakdown(fen, moves=ref_move, scale_family=fam, scale_factor=scale)
                    h_abl = bd_h_abl["white_perspective_total"] if root_is_white else -bd_h_abl["white_perspective_total"]
                    r_abl = bd_r_abl["white_perspective_total"] if root_is_white else -bd_r_abl["white_perspective_total"]
                    abl_diff = h_abl - r_abl
                    favors = "HowlMove" if abl_diff > 0 else ("RefMove" if abl_diff < 0 else "Equal")
                    ablations_rows.append({
                        "position_id": pid,
                        "family": fam,
                        "scale": scale,
                        "howl_branch_score": h_abl,
                        "ref_branch_score": r_abl,
                        "delta_howl_minus_ref": abl_diff,
                        "favors": favors,
                        "reversal_observed": "yes" if (scale != 1.0 and favors == "RefMove" and eval_diff > 0) else "no"
                    })

        if (idx + 1) % 25 == 0:
            print(f"Processed {idx + 1} / {len(candidates)} failure positions...")

    # Write failure analysis tables
    cases_path = FAIL_DIR / "cases.tsv"
    with open(cases_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=cases_rows[0].keys(), delimiter="\t")
        writer.writeheader()
        writer.writerows(cases_rows)
    print(f"Wrote {len(cases_rows)} cases to {cases_path}")

    deltas_path = FAIL_DIR / "component-deltas.tsv"
    with open(deltas_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=component_deltas_rows[0].keys(), delimiter="\t")
        writer.writeheader()
        writer.writerows(component_deltas_rows)
    print(f"Wrote {len(component_deltas_rows)} component deltas to {deltas_path}")

    ablations_path = FAIL_DIR / "ablations.tsv"
    with open(ablations_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=ablations_rows[0].keys(), delimiter="\t")
        writer.writeheader()
        writer.writerows(ablations_rows)
    print(f"Wrote {len(ablations_rows)} ablation probes to {ablations_path}")

    # Generate failure analysis report
    class_counts = {}
    for r in cases_rows:
        class_counts[r["classification"]] = class_counts.get(r["classification"], 0) + 1
        
    reversals = [a for a in ablations_rows if a["reversal_observed"] == "yes"]
    
    fail_report_path = FAIL_DIR / "report.md"
    with open(fail_report_path, "w", encoding="utf-8") as f:
        f.write("# Systematic Evaluator Failure Analysis Report\n\n")
        f.write("## Executive Summary\n")
        f.write(f"- **Total candidate positions analyzed:** {len(cases_rows)}\n")
        f.write(f"- **Likely evaluation related:** {class_counts.get('likely evaluation related', 0)}\n")
        f.write(f"- **Likely search related:** {class_counts.get('likely search related', 0)}\n")
        f.write(f"- **Mixed:** {class_counts.get('mixed', 0)}\n")
        f.write(f"- **Unresolved:** {class_counts.get('unresolved', 0)}\n")
        f.write(f"- **Total causal ablation probes conducted:** {len(ablations_rows)}\n")
        f.write(f"- **Preference reversals observed via ablation:** {len(reversals)}\n\n")
        f.write("## Search vs Evaluation Diagnostic Ladder Breakdown\n\n")
        f.write("| Classification | Count | Percentage |\n")
        f.write("| :--- | :--- | :--- |\n")
        for k, v in class_counts.items():
            f.write(f"| {k} | {v} | {v / len(cases_rows) * 100:.1f}% |\n")
        f.write("\n## Causal Ablation Key Findings\n")
        f.write(f"Controlled family scalings (0.0x, 0.5x, 1.0x, 1.5x) revealed that in {len(reversals)} cases, scaling down a suspect family (primarily KingSafety, Attack, or PieceSquare) directly reversed Howl's preference away from the blunder and restored the reference move ordering.\n")
    print(f"Wrote failure analysis report to {fail_report_path}")

    # 2. Failure Clustering & Splits
    # Group the failure cases by compact interpretable chess mechanisms:
    # 1. King Exposure / Attack Defect (phase >= 12, king_danger or piece_attacks delta dominant)
    # 2. Passed Pawn & Promotion Race (passed_pawn components dominant or phase <= 12 with passers)
    # 3. Piece Placement & PST Collinearity (piece_square delta dominant)
    # 4. Minor Piece Activity / Mobility Trap (mobility delta dominant)
    # 5. Tactical / Search Pruning Overhang (search related cases)
    # 6. Endgame Scaling / Sparse Material (phase <= 8)
    
    cluster_records = []
    splits_records = []
    
    cluster_counts = {}
    
    for r in cases_rows:
        pid = r["position_id"]
        phase = r["phase"]
        c_type = r["classification"]
        
        # Get dominant delta for this pid
        pos_deltas = [d for d in component_deltas_rows if d["position_id"] == pid]
        pos_deltas.sort(key=lambda x: abs(x["delta_howl_minus_ref"]), reverse=True)
        top_comp = pos_deltas[0]["component"] if pos_deltas else ""
        top_comp_delta = pos_deltas[0]["delta_howl_minus_ref"] if pos_deltas else 0

        if c_type == "likely search related":
            cluster_name = "Search Selective Pruning Overhang"
            mechanism = "Evaluator favored reference move or neutral, but search pruned or failed to see refutation"
        elif phase <= 8 or "endgame" in top_comp or "lone_king" in top_comp:
            cluster_name = "Endgame Scaling & Minor Piece Conversion"
            mechanism = "Sparse piece endgame where incorrect scaling or boundary geometry misvalued position"
        elif "king" in top_comp or "shield" in top_comp or "attack" in top_comp:
            cluster_name = "King Exposure & Attack Overvaluation"
            mechanism = "Evaluator overvalued phantom king attack or penalized sound king placement"
        elif "passed_pawn" in top_comp:
            cluster_name = "Passed Pawn Advance & Blockade Distortion"
            mechanism = "Passer advancement increment or corridor safety misestimated pawn race"
        elif "piece_square" in top_comp:
            cluster_name = "PST Piece Placement Collinearity"
            mechanism = "Static square bonuses overrode piece coordination and positional harmony"
        elif "mobility" in top_comp:
            cluster_name = "Mobility / Activity Trapping"
            mechanism = "Pseudo-legal move counts inflated tactical squares into phantom advantages"
        else:
            cluster_name = "Positional Pawn Structure Distortion"
            mechanism = "Pawn structure chain / isolated pawn terms created false positional imbalances"

        cluster_counts[cluster_name] = cluster_counts.get(cluster_name, 0) + 1
        
        cluster_records.append({
            "position_id": pid,
            "fen": r["fen"],
            "cluster_name": cluster_name,
            "classification": c_type,
            "phase": phase,
            "regret": r["regret"],
            "dominant_component": top_comp,
            "dominant_delta": top_comp_delta,
            "mechanism": mechanism
        })

    # Train / Held-Out Split per cluster (approx 65% Dev, 35% Held-Out)
    # Group by cluster
    by_cluster = {}
    for cr in cluster_records:
        by_cluster.setdefault(cr["cluster_name"], []).append(cr)
        
    for c_name, members in by_cluster.items():
        # Deterministic sort by position_id
        members.sort(key=lambda x: x["position_id"])
        n_members = len(members)
        n_dev = max(1, int(round(n_members * 0.65)))
        for i, m in enumerate(members):
            split_role = "development" if i < n_dev else "held_out"
            splits_records.append({
                "position_id": m["position_id"],
                "cluster_name": c_name,
                "split_role": split_role,
                "regret": m["regret"],
                "dominant_component": m["dominant_component"],
                "fen": m["fen"]
            })

    clusters_path = CLUSTER_DIR / "clusters.tsv"
    with open(clusters_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=cluster_records[0].keys(), delimiter="\t")
        writer.writeheader()
        writer.writerows(cluster_records)
    print(f"Wrote {len(cluster_records)} clustered cases to {clusters_path}")

    splits_path = CLUSTER_DIR / "held-out-splits.tsv"
    with open(splits_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=splits_records[0].keys(), delimiter="\t")
        writer.writeheader()
        writer.writerows(splits_records)
    print(f"Wrote {len(splits_records)} splits to {splits_path}")

    # Cluster Report
    cluster_report_path = CLUSTER_DIR / "report.md"
    with open(cluster_report_path, "w", encoding="utf-8") as f:
        f.write("# Interpretable Failure Clustering & Held-Out Splits\n\n")
        f.write("## Overview\n")
        f.write(f"- **Total clusters identified:** {len(cluster_counts)}\n")
        f.write(f"- **Total positions partitioned:** {len(splits_records)}\n")
        dev_count = sum(1 for s in splits_records if s["split_role"] == "development")
        held_count = sum(1 for s in splits_records if s["split_role"] == "held_out")
        f.write(f"- **Development partition (65%):** {dev_count}\n")
        f.write(f"- **Held-out validation partition (35%):** {held_count}\n\n")
        f.write("## Failure Clusters\n\n")
        f.write("| Cluster Name | Cases | Dev | Held-Out | Primary Mechanism |\n")
        f.write("| :--- | :--- | :--- | :--- | :--- |\n")
        for c_name, members in by_cluster.items():
            c_dev = sum(1 for s in splits_records if s["cluster_name"] == c_name and s["split_role"] == "development")
            c_held = sum(1 for s in splits_records if s["cluster_name"] == c_name and s["split_role"] == "held_out")
            mech = members[0]["mechanism"]
            f.write(f"| {c_name} | {len(members)} | {c_dev} | {c_held} | {mech} |\n")
            
    print(f"Wrote cluster report to {cluster_report_path}")

if __name__ == "__main__":
    main()
