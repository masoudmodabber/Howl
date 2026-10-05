import csv
import math
import subprocess
from pathlib import Path

# Run howl_eval_identifiability if needed
cmd = ["./build/howl_eval_identifiability", "--corpus", "move-quality-reference/corpus-1000.tsv", "--output-dir", ".eval-identifiability-qm", "--limit", "1000"]
subprocess.run(cmd, check=True)

# Read parameter statistics
with open(".eval-identifiability-qm/parameter-statistics.tsv") as f:
    stats = list(csv.DictReader(f, delimiter="\t"))

qm_stats = [s for s in stats if s["family"] == "QueenMobility"]
print(f"Total QueenMobility parameters: {len(qm_stats)}")
for s in qm_stats:
    print(f"{s['name']:36} cur={s['current_value']:3} nonzero={s['nonzero_positions']:4} frac={s['nonzero_fraction']:6} mean={s['mean']:7} std={s['standard_deviation']:7} low_support={s['low_support']}")

with open(".eval-identifiability-qm/family-summary.tsv") as f:
    fam = list(csv.DictReader(f, delimiter="\t"))
qm_fam = [x for x in fam if x["family"] == "QueenMobility"][0]
print("\nFamily Summary:")
for k, v in qm_fam.items():
    print(f"  {k}: {v}")

