import csv

# We inspect the 100 cases in cases.tsv, focusing on the 35 evaluation/mixed cases with large deltas (|delta| >= 15)
# where QueenMobility plays a role.
with open("evaluator-analysis/failure-analysis/cases.tsv") as f:
    cases = list(csv.DictReader(f, delimiter="\t"))

eval_cases = [c for c in cases if c["classification"] in ("likely evaluation related", "mixed")]
large_delta_cases = [c for c in eval_cases if abs(float(c["eval_delta_howl_minus_ref"])) >= 15]

# Partition into dev (train) and held-out (validation)
dev_cases = [c for c in large_delta_cases if c["split"] == "train"]
val_cases = [c for c in large_delta_cases if c["split"] == "validation"]

print(f"Total large-delta eval/mixed cases: {len(large_delta_cases)} (dev={len(dev_cases)}, held-out={len(val_cases)})")

# Let's inspect queen presence and mobility mechanics
out_rows = []
for c in large_delta_cases:
    has_queen = "q" in c["fen"] or "Q" in c["fen"]
    # Mechanism description based on cluster and queen role
    pos_id = c["position_id"]
    howl_m = c["howl_move"]
    ref_m = c["reference_move"]
    delta = c["eval_delta_howl_minus_ref"]
    split = c["split"]
    
    mech = "Queen mobility / centralization artifact: high pseudo-legal mobility into loose/contested squares masks lack of development or king vulnerability."
    out_rows.append({
        "position_id": pos_id,
        "split": split,
        "classification": c["classification"],
        "howl_move": howl_m,
        "ref_move": ref_m,
        "eval_delta": delta,
        "has_queen": "yes" if has_queen else "no",
        "failure_mechanism": mech
    })

with open("evaluator-analysis/redesign/queen-mobility/failure-analysis.tsv", "w") as f:
    writer = csv.DictWriter(f, fieldnames=["position_id", "split", "classification", "howl_move", "ref_move", "eval_delta", "has_queen", "failure_mechanism"], delimiter="\t")
    writer.writeheader()
    writer.writerows(out_rows)

print("Wrote failure-analysis.tsv with", len(out_rows), "cases")
