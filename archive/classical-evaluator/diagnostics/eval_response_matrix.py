#!/usr/bin/env python3
"""
tools/diagnostics/eval_response_matrix.py

Produce parameter response vectors and a full 192 x 192 Pearson correlation matrix
for the CURRENT canonical evaluator parameter registry.

Requirements:
1. Dynamically read current canonical parameter registry (count: 192, no hardcoded names).
2. Use existing frozen TRAIN corpus.
3. Symmetrically perturb each canonical parameter around current baseline using metadata.
4. Measure resulting evaluator response across corpus.
5. Standardize each parameter response vector.
6. Compute full Pearson correlation matrix between all parameter response vectors.
7. Output:
   - evaluator-analysis/phase15/grouping/parameter-response-vectors.tsv
   - evaluator-analysis/phase15/grouping/correlation-matrix.tsv
8. Smoke test mode for cheap validation.
"""

import argparse
import csv
import math
import os
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DEFAULT_CORPUS = REPO_ROOT / "evaluator-analysis" / "tuning" / "bishop-pair" / "train-sample.tsv"
DEFAULT_OUT_DIR = REPO_ROOT / "evaluator-analysis" / "phase15" / "grouping"
DEFAULT_VECTORS_OUT = DEFAULT_OUT_DIR / "parameter-response-vectors.tsv"
DEFAULT_MATRIX_OUT = DEFAULT_OUT_DIR / "correlation-matrix.tsv"
EXTRACTOR_TARGET = "howl_extract_canonical_responses"
EXTRACTOR_BIN = REPO_ROOT / "build" / EXTRACTOR_TARGET


def ensure_extractor_built():
    """Build the C++ extractor binary if not present or rebuild needed."""
    if not EXTRACTOR_BIN.exists():
        cmd = ["cmake", "--build", str(REPO_ROOT / "build"), "--target", EXTRACTOR_TARGET, "-j4"]
        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode != 0:
            sys.exit(f"Failed to build {EXTRACTOR_TARGET}:\n{res.stderr}\n{res.stdout}")


def run_extractor(corpus_path: Path, limit: int, output_responses: Path, output_registry: Path):
    """Invoke howl_extract_canonical_responses to compute symmetric responses."""
    output_responses.parent.mkdir(parents=True, exist_ok=True)
    output_registry.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(EXTRACTOR_BIN),
        "--corpus", str(corpus_path),
        "--output", str(output_responses),
        "--param-list", str(output_registry),
    ]
    if limit > 0:
        cmd.extend(["--limit", str(limit)])
    
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        sys.exit(f"Extractor failed:\n{res.stderr}\n{res.stdout}")


def compute_standardized_and_correlation(responses_path: Path, registry_path: Path):
    """
    Reads responses and registry dynamically, standardizes vectors,
    and calculates full Pearson correlation matrix.
    """
    # 1. Read registry dynamically
    param_names = []
    with open(registry_path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            param_names.append(row["name"])

    P = len(param_names)

    # 2. Read response vectors
    pos_ids = []
    fens = []
    vectors = {p: [] for p in param_names}

    with open(responses_path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            pos_ids.append(row["position_id"])
            fens.append(row["fen"])
            for p in param_names:
                vectors[p].append(float(row[p]))

    N = len(pos_ids)
    if N == 0:
        raise ValueError("No positions loaded from responses file.")

    # 3. Standardize response vectors: z = (x - mean) / std
    standardized = {}
    std_devs = {}
    means = {}

    for p in param_names:
        vec = vectors[p]
        mean = sum(vec) / N
        var = sum((x - mean) ** 2 for x in vec) / N
        std = math.sqrt(var)
        means[p] = mean
        std_devs[p] = std
        if std <= 1e-12:
            standardized[p] = [0.0] * N
        else:
            standardized[p] = [(x - mean) / std for x in vec]

    # 4. Compute Pearson correlation matrix
    # R_jk = (1/N) * sum_i (z_ij * z_ik)
    matrix = [[0.0] * P for _ in range(P)]
    for j in range(P):
        pj = param_names[j]
        zj = standardized[pj]
        std_j = std_devs[pj]
        # Diagonal is always 1.0 (self-correlation)
        matrix[j][j] = 1.0

        for k in range(j + 1, P):
            pk = param_names[k]
            std_k = std_devs[pk]
            if std_j <= 1e-12 or std_k <= 1e-12:
                corr = 0.0
            else:
                zk = standardized[pk]
                corr = sum(zj[i] * zk[i] for i in range(N)) / N
                corr = max(-1.0, min(1.0, corr))
            matrix[j][k] = corr
            matrix[k][j] = corr

    return param_names, pos_ids, fens, standardized, matrix


def save_outputs(param_names, pos_ids, fens, standardized, matrix, vectors_out: Path, matrix_out: Path):
    """Write standardized vectors and correlation matrix to TSV files."""
    vectors_out.parent.mkdir(parents=True, exist_ok=True)
    matrix_out.parent.mkdir(parents=True, exist_ok=True)

    # Output standardized vectors
    with open(vectors_out, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, delimiter="\t")
        header = ["position_id", "fen"] + param_names
        writer.writerow(header)
        for i in range(len(pos_ids)):
            row = [pos_ids[i], fens[i]] + [f"{standardized[p][i]:.8g}" for p in param_names]
            writer.writerow(row)

    # Output correlation matrix
    with open(matrix_out, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, delimiter="\t")
        header = ["parameter"] + param_names
        writer.writerow(header)
        for j, pj in enumerate(param_names):
            row = [pj] + [f"{matrix[j][k]:.8f}" for k in range(len(param_names))]
            writer.writerow(row)


def run_smoke_test(param_names, matrix):
    """
    Smoke test verification:
    - parameter count == 192
    - matrix shape == (192, 192)
    - diagonal == 1.0
    - matrix is symmetric
    - no NaN or Inf
    """
    P = len(param_names)
    assert P == 192, f"Expected 192 parameters, got {P}"
    assert len(matrix) == 192, f"Matrix rows = {len(matrix)}, expected 192"
    assert all(len(row) == 192 for row in matrix), "Matrix columns != 192"

    for i in range(192):
        # Diagonal check
        assert math.isclose(matrix[i][i], 1.0, abs_tol=1e-6), f"Diagonal not 1.0 at {i}: {matrix[i][i]}"
        for j in range(192):
            val = matrix[i][j]
            assert not math.isnan(val), f"NaN found at ({i}, {j})"
            assert not math.isinf(val), f"Inf found at ({i}, {j})"
            assert -1.000001 <= val <= 1.000001, f"Correlation out of bounds [-1, 1]: {val} at ({i}, {j})"
            assert math.isclose(matrix[i][j], matrix[j][i], abs_tol=1e-6), f"Asymmetry at ({i}, {j}) vs ({j}, {i})"

    return True


def main():
    parser = argparse.ArgumentParser(description="Generate evaluator response vectors and correlation matrix.")
    parser.add_argument("--corpus", type=Path, default=DEFAULT_CORPUS, help="Path to corpus TSV")
    parser.add_argument("--limit", type=int, default=0, help="Limit number of positions (0 for all)")
    parser.add_argument("--output-vectors", type=Path, default=DEFAULT_VECTORS_OUT, help="Path for output response vectors")
    parser.add_argument("--output-matrix", type=Path, default=DEFAULT_MATRIX_OUT, help="Path for output correlation matrix")
    parser.add_argument("--smoke-test", action="store_true", help="Run smoke test assertions")
    args = parser.parse_args()

    ensure_extractor_built()

    # Paths for temporary responses/registry
    tmp_dir = DEFAULT_OUT_DIR / "tmp"
    tmp_responses = tmp_dir / "raw_responses.tsv"
    tmp_registry = tmp_dir / "registry.tsv"

    run_extractor(args.corpus, args.limit, tmp_responses, tmp_registry)

    param_names, pos_ids, fens, standardized, matrix = compute_standardized_and_correlation(
        tmp_responses, tmp_registry
    )

    save_outputs(param_names, pos_ids, fens, standardized, matrix, args.output_vectors, args.output_matrix)

    if args.smoke_test:
        run_smoke_test(param_names, matrix)
        print(f"Smoke test PASSED: registry_count={len(param_names)}, matrix_shape=({len(matrix)}, {len(matrix[0])}), diagonal=1.0, symmetric=True, no NaN/inf.")
    else:
        print(f"Successfully processed {len(pos_ids)} positions. Registry count: {len(param_names)}. Outputs saved.")


if __name__ == "__main__":
    main()
