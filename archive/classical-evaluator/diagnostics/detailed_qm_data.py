import csv, math

# 1. Parameter metrics for QueenMobility
with open("evaluator-analysis/geometry/parameter-statistics.tsv") as f:
    stats = list(csv.DictReader(f, delimiter="\t"))

qm_stats = [s for s in stats if s["family"] == "QueenMobility"]

# Write to evaluator-analysis/redesign/queen-mobility/geometry-current.tsv
with open("evaluator-analysis/redesign/queen-mobility/geometry-current.tsv", "w") as out:
    out.write("parameter\tphase\tcurrent_value\tnonzero_positions\tnonzero_fraction\tmean\tstandard_deviation\tlow_support\tbucket_span\n")
    for s in qm_stats:
        phase = "MG" if "MiddleGame" in s["name"] else "EG"
        # bucket span
        if "Base" in s["name"]:
            span = "0"
        elif "Increment_1" in s["name"]:
            span = "1..4 (MG) / 1..3 (EG)"
        elif "Increment_2" in s["name"]:
            span = "5..8 (MG) / 4..6 (EG)"
        elif "Increment_3" in s["name"]:
            span = "9..12 (MG) / 7..9 (EG)"
        elif "Increment_4" in s["name"]:
            span = "13..15 (MG) / 10..12 (EG)"
        out.write(f"{s['name']}\t{phase}\t{s['current_value']}\t{s['nonzero_positions']}\t{s['nonzero_fraction']}\t{s['mean']}\t{s['standard_deviation']}\t{s['low_support']}\t{span}\n")

print("Wrote geometry-current.tsv")
