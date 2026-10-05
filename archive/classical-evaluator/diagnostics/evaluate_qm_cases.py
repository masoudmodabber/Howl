import csv
import subprocess

# Let's inspect how the cases were evaluated for piecesquare
with open("evaluator-analysis/failure-analysis/cases.tsv") as f:
    cases = list(csv.DictReader(f, delimiter="\t"))

print(f"Total cases: {len(cases)}")
dev_cases = [c for c in cases if c["split"] == "train"]
val_cases = [c for c in cases if c["split"] == "validation"]
print(f"Dev cases: {len(dev_cases)}, Validation cases: {len(val_cases)}")

