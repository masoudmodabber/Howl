#!/usr/bin/env python3
"""
tools/diagnostics/cluster_eval_responses.py

Hierarchical clustering of Howl evaluator parameters based on response correlation.

Inputs:
- evaluator-analysis/phase15/grouping/correlation-matrix.tsv
- evaluator-analysis/phase15/grouping/parameter-response-vectors.tsv

Method:
- distance(i,j) = 1 - abs(correlation(i,j))
- Hierarchical clustering (Ward / minimum variance linkage on the response geometry)
- Evaluates cluster counts k = 2 through 20.
- For each k, calculates:
    - Silhouette score (precomputed distance matrix)
    - Bootstrap cluster stability using 100 resamples of the 800 response rows (Adjusted Rand Index)
- Selects the partition with the best combination of silhouette and bootstrap stability.

Outputs:
- evaluator-analysis/phase15/grouping/groups.tsv
- evaluator-analysis/phase15/grouping/clustering-summary.tsv
"""

import argparse
import csv
import math
import sys
from pathlib import Path
import numpy as np
from scipy.spatial.distance import squareform
from scipy.cluster.hierarchy import linkage, cut_tree
from sklearn.metrics import silhouette_score, adjusted_rand_score

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
GROUPING_DIR = REPO_ROOT / "evaluator-analysis" / "phase15" / "grouping"
DEFAULT_CORR_PATH = GROUPING_DIR / "correlation-matrix.tsv"
DEFAULT_VEC_PATH = GROUPING_DIR / "parameter-response-vectors.tsv"
DEFAULT_GROUPS_PATH = GROUPING_DIR / "groups.tsv"
DEFAULT_SUMMARY_PATH = GROUPING_DIR / "clustering-summary.tsv"


def load_correlation_matrix(corr_path: Path):
    params = []
    matrix_rows = []
    with open(corr_path, "r", encoding="utf-8") as f:
        reader = csv.reader(f, delimiter="\t")
        header = next(reader)
        params = header[1:]
        for row in reader:
            matrix_rows.append([float(x) for x in row[1:]])
    corr = np.array(matrix_rows, dtype=np.float64)
    return params, corr


def load_response_vectors(vec_path: Path, params: list):
    vec_rows = []
    with open(vec_path, "r", encoding="utf-8") as f:
        reader = csv.reader(f, delimiter="\t")
        header = next(reader)
        p_indices = [header.index(p) for p in params]
        for row in reader:
            vec_rows.append([float(row[idx]) for idx in p_indices])
    X = np.array(vec_rows, dtype=np.float64)
    return X


def run_clustering_and_bootstrap(params, corr, X, k_min=2, k_max=20, n_bootstraps=100, seed=42):
    # Compute distance: distance(i,j) = 1 - abs(correlation(i,j))
    dist = 1.0 - np.abs(corr)
    dist = (dist + dist.T) / 2.0
    np.fill_diagonal(dist, 0.0)
    dist = np.clip(dist, 0.0, 1.0)
    condensed = squareform(dist)

    # Ward linkage on condensed distance
    Z_base = linkage(condensed, method="ward")

    # Base cluster assignments for each k
    base_labels = {}
    for k in range(k_min, k_max + 1):
        base_labels[k] = cut_tree(Z_base, n_clusters=k).flatten()

    # Calculate silhouette score for each k
    silhouettes = {}
    for k in range(k_min, k_max + 1):
        sil = float(silhouette_score(dist, base_labels[k], metric="precomputed"))
        silhouettes[k] = sil

    # Bootstrap stability across 100 resamples of the 800 positions
    N, P = X.shape
    rng = np.random.default_rng(seed)
    ari_scores = {k: [] for k in range(k_min, k_max + 1)}

    for _ in range(n_bootstraps):
        idx = rng.choice(N, size=N, replace=True)
        X_b = X[idx, :]
        means = np.mean(X_b, axis=0)
        stds = np.std(X_b, axis=0)
        stds_safe = np.where(stds < 1e-12, 1.0, stds)
        X_std = (X_b - means) / stds_safe
        X_std[:, stds < 1e-12] = 0.0

        corr_b = (X_std.T @ X_std) / N
        corr_b = np.clip(corr_b, -1.0, 1.0)
        dist_b = 1.0 - np.abs(corr_b)
        dist_b = (dist_b + dist_b.T) / 2.0
        np.fill_diagonal(dist_b, 0.0)
        dist_b = np.clip(dist_b, 0.0, 1.0)
        condensed_b = squareform(dist_b)

        Z_b = linkage(condensed_b, method="ward")
        for k in range(k_min, k_max + 1):
            labels_b = cut_tree(Z_b, n_clusters=k).flatten()
            ari = adjusted_rand_score(base_labels[k], labels_b)
            ari_scores[k].append(ari)

    stabilities = {k: float(np.mean(ari_scores[k])) for k in range(k_min, k_max + 1)}

    # Selection criterion: best combination of silhouette and bootstrap stability
    # Normalized harmonic mean (F1) / combination score
    sils = [silhouettes[k] for k in range(k_min, k_max + 1)]
    stabs = [stabilities[k] for k in range(k_min, k_max + 1)]
    min_sil, max_sil = min(sils), max(sils)
    min_stab, max_stab = min(stabs), max(stabs)

    scores = {}
    for k in range(k_min, k_max + 1):
        n_sil = (silhouettes[k] - min_sil) / (max_sil - min_sil + 1e-12)
        n_stab = (stabilities[k] - min_stab) / (max_stab - min_stab + 1e-12)
        comb = 2.0 * n_sil * n_stab / (n_sil + n_stab + 1e-12)
        scores[k] = comb

    best_k = max(scores.keys(), key=lambda k: scores[k])
    return base_labels, silhouettes, stabilities, best_k


def save_clustering_results(params, labels_dict, silhouettes, stabilities, best_k, groups_path: Path, summary_path: Path):
    groups_path.parent.mkdir(parents=True, exist_ok=True)
    summary_path.parent.mkdir(parents=True, exist_ok=True)

    # 1. Save groups.tsv for the selected partition
    best_labels = labels_dict[best_k]
    with open(groups_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, delimiter="\t")
        writer.writerow(["parameter", "cluster_id"])
        for p, cid in zip(params, best_labels):
            writer.writerow([p, cid])

    # 2. Save clustering-summary.tsv
    with open(summary_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, delimiter="\t")
        writer.writerow(["k", "silhouette", "bootstrap_stability"])
        for k in sorted(silhouettes.keys()):
            writer.writerow([k, f"{silhouettes[k]:.6f}", f"{stabilities[k]:.6f}"])


def main():
    parser = argparse.ArgumentParser(description="Cluster evaluator parameters by response correlation.")
    parser.add_argument("--corr", type=Path, default=DEFAULT_CORR_PATH, help="Path to correlation-matrix.tsv")
    parser.add_argument("--vectors", type=Path, default=DEFAULT_VEC_PATH, help="Path to parameter-response-vectors.tsv")
    parser.add_argument("--groups-out", type=Path, default=DEFAULT_GROUPS_PATH, help="Output path for groups.tsv")
    parser.add_argument("--summary-out", type=Path, default=DEFAULT_SUMMARY_PATH, help="Output path for clustering-summary.tsv")
    parser.add_argument("--bootstraps", type=int, default=100, help="Number of bootstrap resamples (default 100)")
    parser.add_argument("--seed", type=int, default=42, help="Random seed for bootstrapping")
    args = parser.parse_args()

    params, corr = load_correlation_matrix(args.corr)
    X = load_response_vectors(args.vectors, params)

    labels_dict, silhouettes, stabilities, best_k = run_clustering_and_bootstrap(
        params, corr, X, k_min=2, k_max=20, n_bootstraps=args.bootstraps, seed=args.seed
    )

    save_clustering_results(params, labels_dict, silhouettes, stabilities, best_k, args.groups_out, args.summary_out)

    # Calculate cluster sizes for selected partition
    counts = np.bincount(labels_dict[best_k])
    cluster_sizes = sorted([int(c) for c in counts], reverse=True)

    # Print only the required output
    print(f"selected k: {best_k}")
    print(f"cluster sizes: {cluster_sizes}")
    print(f"silhouette: {silhouettes[best_k]:.6f}")
    print(f"bootstrap stability: {stabilities[best_k]:.6f}")
    print(f"output paths: {args.groups_out}, {args.summary_out}")


if __name__ == "__main__":
    main()
