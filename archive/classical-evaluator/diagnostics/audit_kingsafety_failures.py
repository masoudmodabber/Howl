import csv

with open("evaluator-analysis/clusters/clusters.tsv") as f:
    clusters = {c["position_id"]: c for c in csv.DictReader(f, delimiter="\t")}

with open("evaluator-analysis/failure-analysis/cases.tsv") as f:
    cases = {c["position_id"]: c for c in csv.DictReader(f, delimiter="\t")}

with open("evaluator-analysis/failure-analysis/ablations.tsv") as f:
    ablations = [r for r in csv.DictReader(f, delimiter="\t") if r["family"] == "KingSafety"]

ks_reversals = {r["position_id"] for r in ablations if r["reversal_observed"] == "yes"}

# Identify all KingSafety failure cases (from cluster or ablation or high delta)
ks_cases = []
for pid, c in clusters.items():
    case = cases.get(pid, {})
    is_ks_cluster = c["cluster_name"] == "King Exposure & Attack Overvaluation"
    is_ks_rev = pid in ks_reversals
    is_ks_comp = c.get("dominant_component") == "king_danger"
    
    if is_ks_cluster or is_ks_rev or is_ks_comp:
        split = case.get("split", "unknown")
        howl_m = case.get("howl_move", "-")
        ref_m = case.get("reference_move", "-")
        delta = case.get("eval_delta_howl_minus_ref", "0")
        
        # Determine failure mechanism
        if is_ks_rev:
            mech = "Causal KingSafety Reversal: Quadratic danger escalation or phantom shelter penalty caused blunder; restored under 0x ablation."
        elif is_ks_cluster:
            mech = "King Exposure & Attack Overvaluation: Excessive danger score assigned to uncastled or checked king, causing premature pawn thrust or blunder."
        else:
            mech = "King danger component delta dominant."
            
        ks_cases.append({
            "position_id": pid,
            "split": split,
            "cluster_name": c["cluster_name"],
            "howl_move": howl_m,
            "ref_move": ref_m,
            "eval_delta": delta,
            "reversal_under_ablation": "yes" if is_ks_rev else "no",
            "failure_mechanism": mech
        })

print(f"Total KingSafety cases identified: {len(ks_cases)}")
dev_count = sum(1 for c in ks_cases if c["split"] == "train")
val_count = sum(1 for c in ks_cases if c["split"] == "validation")
print(f"Dev: {dev_count}, Held-out: {val_count}")

with open("evaluator-analysis/redesign/king-safety/failure-analysis.tsv", "w") as out:
    writer = csv.DictWriter(out, fieldnames=list(ks_cases[0].keys()), delimiter="\t")
    writer.writeheader()
    writer.writerows(ks_cases)

print("Generated failure-analysis.tsv")
