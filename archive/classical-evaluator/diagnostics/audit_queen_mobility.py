import csv
import math

# 1. Parameter statistics
with open("evaluator-analysis/geometry/parameter-statistics.tsv") as f:
    stats = list(csv.DictReader(f, delimiter="\t"))

qm_stats = [s for s in stats if s["family"] == "QueenMobility"]
print(f"QueenMobility parameter count: {len(qm_stats)}")
for s in qm_stats:
    print(f"{s['name']:36} cur={s['current_value']:3} nonzero={s['nonzero_positions']:4} frac={s['nonzero_fraction']:6} mean={s['mean']:7} std={s['standard_deviation']:7} low={s['low_support']}")

with open("evaluator-analysis/geometry/family-summary.tsv") as f:
    fams = list(csv.DictReader(f, delimiter="\t"))
qm_fam = [x for x in fams if x["family"] == "QueenMobility"][0]
print("\nFamily Summary:")
for k, v in qm_fam.items():
    print(f"  {k}: {v}")

