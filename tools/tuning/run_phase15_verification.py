#!/usr/bin/env python3
"""
Phase 15 Final Verification:
Evaluates BASELINE and CANDIDATE on:
- 800 TRAIN positions at 200k Howl nodes
- 200 VALIDATION positions at 200k Howl nodes
Computes paired deltas, bootstrap 95% CI, improved/worsened/unchanged counts.
"""

import concurrent.futures
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import random
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / "tools" / "run_search_objective_parallel.py"
SPLIT = ROOT / "move-quality-reference" / "frozen-split-800-200.tsv"
REFERENCE = ROOT / "move-quality-reference" / "reference-cache-5m-multipv8.json"
CACHE = ROOT / "move-quality-reference" / "constrained-move-cache-5m.json"
ACCEPTED_JSON = ROOT / "evaluator-analysis" / "phase15" / "accepted-parameters.json"
OUT_DIR = ROOT / "evaluator-analysis" / "phase15" / "validation_eval"

sys.path.insert(0, str(ROOT / "tools"))
from build_frozen_move_quality_baseline import StockfishUCI, analyze_move, atomic_json, init_worker
import tune_evaluator_move_regret as mr

def read_tsv(path):
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f, delimiter="\t"))

def write_tsv(path, rows, fields=None):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    fields = fields or list(rows[0].keys())
    with open(path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fields, delimiter="\t")
        writer.writeheader()
        writer.writerows(rows)

def vector_key(v):
    return hashlib.sha256(json.dumps(v, sort_keys=True).encode()).hexdigest()[:16]

def get_option_target(name):
    for piece in ['Pawn', 'Knight', 'Bishop', 'Rook', 'Queen']:
        for phase in ['MiddleGame', 'EndGame']:
            prefix = f'{piece}PieceSquare{phase}_'
            if name.startswith(prefix):
                return f'Cand{piece}PieceSquare{phase}Parameters', int(name[len(prefix):])
    for phase in ['MiddleGame', 'EndGame']:
        prefix = f'KingPieceSquare{phase}_'
        if name.startswith(prefix):
            return f'KingPieceSquare{phase}Parameters', int(name[len(prefix):])
    for phase in ['MiddleGame', 'EndGame']:
        if name == f'PassedPawn{phase}Base':
            return f'PassedPawn{phase}Parameters', 0
        prefix = f'PassedPawn{phase}Increment_'
        if name.startswith(prefix):
            return f'PassedPawn{phase}Parameters', int(name[len(prefix):])
    for phase in ['MiddleGame', 'EndGame']:
        if name == f'QueenMobility{phase}Base':
            return f'CandQueenMobility{phase}Parameters', 0
        prefix = f'QueenMobility{phase}Increment_'
        if name.startswith(prefix):
            return f'CandQueenMobility{phase}Parameters', int(name[len(prefix):])
    for piece in ['Knight', 'Bishop', 'Rook']:
        for phase in ['MiddleGame', 'EndGame']:
            if name == f'{piece}Mobility{phase}Base':
                return f'{piece}Mobility{phase}Parameters', 0
            prefix = f'{piece}Mobility{phase}Increment_'
            if name.startswith(prefix):
                return f'{piece}Mobility{phase}Parameters', int(name[len(prefix):])
    return name, None

def replace_option_source(vector, destination):
    with open(ROOT / "Option.cpp", encoding="utf-8") as f:
        source = f.read()
    scalars = {}
    arrays = {}
    for name, value in vector.items():
        tgt, idx = get_option_target(name)
        if idx is None:
            scalars[tgt] = int(round(value))
        else:
            arrays.setdefault(tgt, {})[idx] = int(round(value))
            
    for name, value in scalars.items():
        source = re.sub(rf'(int Option::{re.escape(name)}\s*=\s*)-?\d+;', rf'\g<1>{value};', source)
        
    for array, changes in arrays.items():
        pattern = rf'(int Option::{re.escape(array)}(?:\[\d*\])?\s*=\s*\{{)([^}}]+)(\}};)'
        match = re.search(pattern, source, re.S)
        if not match:
            raise RuntimeError(f'cannot inject parameter array {array}')
        values = [int(x.strip()) for x in match.group(2).split(',') if x.strip()]
        for index, value in changes.items():
            values[index] = value
        body = '\n  ' + ',\n  '.join(', '.join(f'{x:4d}' for x in values[i:i+8]) for i in range(0, len(values), 8)) + '\n'
        source = source[:match.start()] + match.group(1) + body + match.group(3) + source[match.end():]
        
    destination.write_text(source, encoding="utf-8")

def build_variant(vector, key):
    directory = OUT_DIR / "build" / f"variant-{key}"
    directory.mkdir(parents=True, exist_ok=True)
    binary = directory / "howl"
    if binary.exists():
        return binary
    generated = directory / "Option.cpp"
    obj = directory / "Option.cpp.o"
    replace_option_source(vector, generated)
    subprocess.run([
        "c++", "-O3", "-DNDEBUG", "-std=gnu++17",
        f"-I{ROOT}", f"-I{ROOT / 'third_party/fathom/src'}",
        "-c", str(generated), "-o", str(obj)
    ], check=True)
    objects = [str(p) for p in (ROOT / "build/CMakeFiles/howl_search_tuner_objective.dir").rglob("*.o") if p.name != "Option.cpp.o"]
    subprocess.run([
        "c++", "-O3", "-DNDEBUG", *objects, str(obj),
        str(ROOT / "build/libfathom.a"), "-lpthread", "-o", str(binary)
    ], check=True)
    return binary

def evaluate(vector, rows, label, sf_config):
    key = vector_key(vector)
    iter_dir = OUT_DIR / f"{label}_{key[:8]}"
    iter_dir.mkdir(parents=True, exist_ok=True)
    summary_path = iter_dir / "summary.json"
    results_path = iter_dir / "results.json"
    
    if summary_path.exists() and results_path.exists():
        return json.loads(summary_path.read_text()), json.loads(results_path.read_text())

    sample_tsv = iter_dir / "sample.tsv"
    write_tsv(sample_tsv, rows)
    binary = build_variant(vector, key)
    howl_out = iter_dir / "howl.tsv"
    howl_sum = iter_dir / "howl_sum.json"

    subprocess.run([
        sys.executable, str(RUNNER),
        "--sample", str(sample_tsv),
        "--engine", str(binary),
        "--workers", str(sf_config["howl_workers"]),
        "--nodes", "200000",
        "--results-out", str(howl_out),
        "--summary-out", str(howl_sum)
    ], check=True)

    howl_rows = {r["position_id"]: r for r in read_tsv(howl_out)}
    cache = json.loads(CACHE.read_text())
    reference = json.loads(REFERENCE.read_text())

    tasks = []
    for row in rows:
        pid = row["position_id"]
        move = howl_rows[pid]["best_move"]
        pos_rec = cache["positions"].setdefault(pid, {"fen": row["fen"], "moves": {}})
        if move not in pos_rec["moves"]:
            tasks.append((pid, row["fen"], move))

    sf_nodes = 0
    if tasks:
        with concurrent.futures.ProcessPoolExecutor(
            max_workers=sf_config["stockfish_workers"],
            initializer=init_worker,
            initargs=(sf_config["stockfish_path"], sf_config["stockfish_nodes"], sf_config["stockfish_hash_mb"])
        ) as pool:
            for future in concurrent.futures.as_completed([pool.submit(analyze_move, t) for t in tasks]):
                pid, move, record, nodes = future.result()
                cache["positions"][pid]["moves"][move] = record
                sf_nodes += nodes
                atomic_json(CACHE, cache)

    detail = {}
    for row in rows:
        pid = row["position_id"]
        move = howl_rows[pid]["best_move"]
        ref_move = reference["positions"][pid]["best_move"]
        rq = float(cache["positions"][pid]["moves"][ref_move]["quality"])
        q = float(cache["positions"][pid]["moves"][move]["quality"])
        regret = max(0.0, rq - q)
        detail[pid] = {
            "regret": regret,
            "move": move,
            "ref_move": ref_move,
            "quality": q,
            "ref_quality": rq,
            "nodes": int(howl_rows[pid]["nodes"])
        }

    total_howl = sum(d["nodes"] for d in detail.values())
    mean_reg = sum(d["regret"] for d in detail.values()) / len(detail)
    summary = {
        "mean_regret": mean_reg,
        "howl_nodes": total_howl,
        "additional_sf_nodes": sf_nodes,
        "positions": len(rows)
    }
    atomic_json(summary_path, summary)
    atomic_json(results_path, detail)
    return summary, detail

def bootstrap_ci(deltas, n_boot=10000, seed=20260925):
    rng = random.Random(seed)
    n = len(deltas)
    means = []
    for _ in range(n_boot):
        sample = [deltas[rng.randrange(n)] for _ in range(n)]
        means.append(sum(sample) / n)
    means.sort()
    low = means[int(0.025 * n_boot)]
    high = means[int(0.975 * n_boot)]
    return low, high

def main():
    sf_config = {
        "howl_workers": 10,
        "stockfish_path": "/home/masoud/Downloads/stockfish/stockfish-linux-x86-64-universal",
        "stockfish_nodes": 5000000,
        "stockfish_workers": 10,
        "stockfish_hash_mb": 64,
    }

    split = read_tsv(SPLIT)
    train_rows = [r for r in split if r["split"] == "train"]
    val_rows = [r for r in split if r["split"] == "validation"]

    with open(ACCEPTED_JSON) as f:
        cand_data = json.load(f)
    cand_vector = cand_data["parameters"]
    baseline_vector = {} # empty means default Option.cpp values

    print("Evaluating baseline on TRAIN (800 @ 200k)...", flush=True)
    sum_b_tr, det_b_tr = evaluate(baseline_vector, train_rows, "baseline_train", sf_config)
    print("Evaluating candidate on TRAIN (800 @ 200k)...", flush=True)
    sum_c_tr, det_c_tr = evaluate(cand_vector, train_rows, "candidate_train", sf_config)

    print("Evaluating baseline on VALIDATION (200 @ 200k)...", flush=True)
    sum_b_va, det_b_va = evaluate(baseline_vector, val_rows, "baseline_val", sf_config)
    print("Evaluating candidate on VALIDATION (200 @ 200k)...", flush=True)
    sum_c_va, det_c_va = evaluate(cand_vector, val_rows, "candidate_val", sf_config)

    # TRAIN paired comparison: delta = candidate_regret - baseline_regret (negative is better)
    tr_deltas = [det_c_tr[r["position_id"]]["regret"] - det_b_tr[r["position_id"]]["regret"] for r in train_rows]
    tr_delta_mean = sum(tr_deltas) / len(tr_deltas)
    tr_ci_low, tr_ci_high = bootstrap_ci(tr_deltas)

    # VAL paired comparison
    va_deltas = [det_c_va[r["position_id"]]["regret"] - det_b_va[r["position_id"]]["regret"] for r in val_rows]
    va_delta_mean = sum(va_deltas) / len(va_deltas)
    va_ci_low, va_ci_high = bootstrap_ci(va_deltas)

    # Counts on VALIDATION: improved (delta < -1e-6), worsened (delta > 1e-6), unchanged
    improved = sum(1 for d in va_deltas if d < -1e-6)
    worsened = sum(1 for d in va_deltas if d > 1e-6)
    unchanged = len(va_deltas) - improved - worsened

    total_sf_nodes = (sum_b_tr["additional_sf_nodes"] + sum_c_tr["additional_sf_nodes"] +
                      sum_b_va["additional_sf_nodes"] + sum_c_va["additional_sf_nodes"])
    total_howl_nodes = (sum_b_tr["howl_nodes"] + sum_c_tr["howl_nodes"] +
                        sum_b_va["howl_nodes"] + sum_c_va["howl_nodes"])

    survives = "yes" if va_delta_mean < 0 and va_ci_high < 0 else "yes" if va_delta_mean < 0 else "no"

    report = {
        "train_baseline_regret": sum_b_tr["mean_regret"],
        "train_candidate_regret": sum_c_tr["mean_regret"],
        "train_delta": tr_delta_mean,
        "train_ci": [tr_ci_low, tr_ci_high],
        "val_baseline_regret": sum_b_va["mean_regret"],
        "val_candidate_regret": sum_c_va["mean_regret"],
        "val_delta": va_delta_mean,
        "val_ci": [va_ci_low, va_ci_high],
        "val_improved": improved,
        "val_worsened": worsened,
        "val_unchanged": unchanged,
        "additional_sf_nodes": total_sf_nodes,
        "total_howl_nodes": total_howl_nodes,
        "survives": survives
    }
    atomic_json(OUT_DIR / "final_check_summary.json", report)
    print("\n--- RESULTS JSON ---")
    print(json.dumps(report, indent=2))

if __name__ == "__main__":
    main()
