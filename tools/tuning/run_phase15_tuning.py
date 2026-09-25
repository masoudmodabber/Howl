#!/usr/bin/env python3
"""
Phase 15 Tuning Runner
----------------------
Executes Phase 15 tuning using the EXISTING derived tuning blocks:
evaluator-analysis/phase15/grouping/tuning-blocks.tsv

Rules:
- Active parameters: 179 (excluding 13 zero-response parameters).
- Budget: Minimum 100k nodes per position. Max total budget: 20 billion Howl nodes.
- For each block:
    size 1 to 3: direct bounded coordinate search
    size 4 to 12: bounded SPSA
- Staged TRAIN evaluation:
    100 positions first -> then 300 -> then 800 only for surviving candidates.
- Tune blocks sequentially against the currently accepted global candidate.
- Single-line progress output:
    block number / total
    candidate evaluations completed
    Howl nodes consumed
    current accepted changes
"""

import argparse
import concurrent.futures
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import random
import re
import shutil
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / "tools" / "run_search_objective_parallel" if (ROOT / "tools" / "run_search_objective_parallel").exists() else ROOT / "tools" / "run_search_objective_parallel.py"
SPLIT = ROOT / "move-quality-reference" / "frozen-split-800-200.tsv"
REFERENCE = ROOT / "move-quality-reference" / "reference-cache-5m-multipv8.json"
CACHE = ROOT / "move-quality-reference" / "constrained-move-cache-5m.json"
BLOCKS_TSV = ROOT / "evaluator-analysis" / "phase15" / "grouping" / "tuning-blocks.tsv"
ACCEPTED_OUT = ROOT / "evaluator-analysis" / "phase15" / "accepted-parameters.json"
TUNING_DIR = ROOT / "evaluator-analysis" / "phase15" / "tuning_run"

# Import helper functions from tools
sys.path.insert(0, str(ROOT / "tools"))
from build_frozen_move_quality_baseline import StockfishUCI, analyze_move, atomic_json, init_worker

ZERO_RESPONSE_PARAMS = {
    'BishopMobilityMiddleGameIncrement_3', 'RookMobilityMiddleGameIncrement_4',
    'QueenMobilityEndGameIncrement_3', 'Threat_PawnOnMinor_MG',
    'Threat_PawnOnMajor_MG', 'Threat_MinorOnPawn_MG',
    'Threat_MinorOnMinor_MG', 'Threat_MinorOnMajor_MG',
    'Threat_RookOnPawn_MG', 'Threat_RookOnMinor_MG',
    'Threat_RookOnQueen_MG', 'Threat_QueenOnPawn_MG',
    'Threat_QueenOnPiece_MG'
}

def read_tsv(path):
    with open(path, newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))

def write_tsv(path, rows, fields=None):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    fields = fields or list(rows[0].keys())
    with open(path, "w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t")
        writer.writeheader()
        writer.writerows(rows)

def vector_key(vector):
    return hashlib.sha256(json.dumps(vector, sort_keys=True).encode()).hexdigest()[:16]

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

def extract_baseline_vector(block_params):
    with open(ROOT / "Option.cpp", encoding="utf-8") as f:
        src = f.read()
    values = {}
    for p in block_params:
        tgt, idx = get_option_target(p)
        if idx is None:
            m = re.search(rf'int Option::{re.escape(tgt)}\s*=\s*(-?\d+);', src)
            if not m:
                raise RuntimeError(f"Could not find scalar Option::{tgt}")
            values[p] = int(m.group(1))
        else:
            m = re.search(rf'int Option::{re.escape(tgt)}(?:\[\d*\])?\s*=\s*\{{([^}}]+)\}};', src, re.S)
            if not m:
                raise RuntimeError(f"Could not find array Option::{tgt}")
            arr = [int(x.strip()) for x in m.group(1).split(',') if x.strip()]
            values[p] = arr[idx]
    return values

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
    directory = TUNING_DIR / "build" / f"variant-{key}"
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

def bounds_and_scale(name, value):
    scale = max(2.0, abs(value) * 0.1)
    if 'Permille' in name:
        scale = max(scale, 10.0)
    if 'Percent' in name:
        scale = max(scale, 2.0)
    span = int(math.ceil(scale * 20))
    low, high = value - span, value + span
    if 'Attack' in name or 'King' in name or 'Increment_' in name or name in {'BishopPairValue', 'PassedPawnMiddleGameFileAmplitude'}:
        low = 0
    if 'ScalePermille' in name:
        low, high = 0, 1000
    return scale, low, high

class Tuner:
    def __init__(self, config_path=None):
        self.config = {
            "howl_nodes": 100000,
            "howl_workers": 10,
            "stockfish_path": "/home/masoud/Downloads/stockfish/stockfish-linux-x86-64-universal",
            "stockfish_nodes": 5000000,
            "stockfish_workers": 10,
            "stockfish_hash_mb": 64,
            "max_budget": 20000000000, # 20 Billion nodes
        }
        if config_path and Path(config_path).exists():
            user_cfg = json.loads(Path(config_path).read_text())
            self.config.update(user_cfg)
        self.config["howl_nodes"] = max(100000, self.config["howl_nodes"])

        # Load blocks
        self.blocks = {}
        with open(BLOCKS_TSV, newline="", encoding="utf-8") as f:
            for row in csv.DictReader(f, delimiter="\t"):
                p = row["parameter"]
                if p not in ZERO_RESPONSE_PARAMS:
                    self.blocks.setdefault(int(row["block_id"]), []).append(p)

        self.block_ids = sorted(self.blocks.keys())
        all_block_params = set()
        for p_list in self.blocks.values():
            all_block_params.update(p_list)
        self.all_params = sorted(all_block_params)

        # Baseline vector
        self.baseline_vector = extract_baseline_vector(self.all_params)
        self.current_global_vector = dict(self.baseline_vector)
        self.param_bounds = {}
        self.param_scales = {}
        for p in self.all_params:
            scale, low, high = bounds_and_scale(p, self.baseline_vector[p])
            self.param_scales[p] = scale
            self.param_bounds[p] = (low, high)

        # Split positions
        split_rows = read_tsv(SPLIT)
        self.train_positions = [r for r in split_rows if r["split"] == "train"]
        if len(self.train_positions) != 800:
            raise RuntimeError(f"Expected 800 train positions, found {len(self.train_positions)}")
        
        self.train_100 = self.train_positions[:100]
        self.train_300 = self.train_positions[:300]
        self.train_800 = self.train_positions[:800]

        # Accounting
        self.total_howl_nodes = 0
        self.total_candidates_evaluated = 0
        self.blocks_attempted = 0
        self.eval_cache = {}

        # Reference & SF Cache
        self.reference = json.loads(REFERENCE.read_text())
        self.cache = json.loads(CACHE.read_text())

        # Baseline train regret
        TUNING_DIR.mkdir(parents=True, exist_ok=True)
        print("Computing baseline regret on 800 TRAIN positions...", flush=True)
        base_res = self.evaluate(self.baseline_vector, self.train_800, "baseline-train-800")
        self.baseline_train_regret = base_res["mean_regret"]
        self.current_global_loss_800 = self.baseline_train_regret
        self.current_global_loss_300 = self.evaluate(self.baseline_vector, self.train_300, "baseline-train-300")["mean_regret"]
        self.current_global_loss_100 = self.evaluate(self.baseline_vector, self.train_100, "baseline-train-100")["mean_regret"]
        print(f"Baseline TRAIN regrets: 100 pos = {self.current_global_loss_100:.6f}, 300 pos = {self.current_global_loss_300:.6f}, 800 pos = {self.baseline_train_regret:.6f}", flush=True)

    def evaluate(self, vector, rows, label):
        n_pos = len(rows)
        key = vector_key(vector)
        cache_key = (key, n_pos)
        if cache_key in self.eval_cache:
            return self.eval_cache[cache_key]

        iter_dir = TUNING_DIR / "evaluations" / f"{label}_{key[:8]}"
        iter_dir.mkdir(parents=True, exist_ok=True)
        summary_path = iter_dir / "summary.json"
        
        if summary_path.exists():
            res = json.loads(summary_path.read_text())
            self.eval_cache[cache_key] = res
            return res

        sample_tsv = iter_dir / "sample.tsv"
        write_tsv(sample_tsv, rows)
        binary = build_variant(vector, key)
        howl_out = iter_dir / "howl.tsv"
        howl_sum = iter_dir / "howl_sum.json"

        subprocess.run([
            sys.executable, str(RUNNER),
            "--sample", str(sample_tsv),
            "--engine", str(binary),
            "--workers", str(self.config["howl_workers"]),
            "--nodes", str(self.config["howl_nodes"]),
            "--results-out", str(howl_out),
            "--summary-out", str(howl_sum)
        ], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        howl_rows = {r["position_id"]: r for r in read_tsv(howl_out)}
        
        # Check Stockfish cache for missing moves
        tasks = []
        for row in rows:
            pid = row["position_id"]
            move = howl_rows[pid]["best_move"]
            pos_rec = self.cache["positions"].setdefault(pid, {"fen": row["fen"], "moves": {}})
            if move not in pos_rec["moves"]:
                tasks.append((pid, row["fen"], move))

        if tasks:
            with concurrent.futures.ProcessPoolExecutor(
                max_workers=self.config["stockfish_workers"],
                initializer=init_worker,
                initargs=(self.config["stockfish_path"], self.config["stockfish_nodes"], self.config["stockfish_hash_mb"])
            ) as pool:
                for future in concurrent.futures.as_completed([pool.submit(analyze_move, t) for t in tasks]):
                    pid, move, record, nodes = future.result()
                    self.cache["positions"][pid]["moves"][move] = record
                    atomic_json(CACHE, self.cache)

        detail = []
        for row in rows:
            pid = row["position_id"]
            move = howl_rows[pid]["best_move"]
            ref_move = self.reference["positions"][pid]["best_move"]
            rq = float(self.cache["positions"][pid]["moves"][ref_move]["quality"])
            q = float(self.cache["positions"][pid]["moves"][move]["quality"])
            detail.append({"regret": max(0.0, rq - q), "nodes": int(howl_rows[pid]["nodes"])})

        nodes_consumed = sum(d["nodes"] for d in detail)
        mean_regret = sum(d["regret"] for d in detail) / len(detail)

        res = {
            "mean_regret": mean_regret,
            "howl_nodes": nodes_consumed,
            "positions": n_pos,
            "key": key
        }
        atomic_json(summary_path, res)
        self.eval_cache[cache_key] = res
        self.total_howl_nodes += nodes_consumed
        return res

    def count_changed(self):
        return sum(1 for p in self.all_params if self.current_global_vector[p] != self.baseline_vector[p])

    def print_progress(self, current_block_idx):
        # Progress output only:
        # block number / total
        # candidate evaluations completed
        # Howl nodes consumed
        # current accepted changes
        print(f"block {current_block_idx + 1}/{len(self.block_ids)} | "
              f"candidates evaluated: {self.total_candidates_evaluated} | "
              f"nodes: {self.total_howl_nodes:,} | "
              f"accepted changes: {self.count_changed()} | "
              f"loss: {self.current_global_loss_800:.6f}", flush=True)

    def test_candidate_staged(self, candidate, label):
        """
        Staged TRAIN evaluation:
        100 positions first -> then 300 -> then 800 only for surviving candidates.
        Returns: (survived: bool, regret_800: float)
        """
        self.total_candidates_evaluated += 1
        
        # Stage 1: 100 positions
        res100 = self.evaluate(candidate, self.train_100, f"{label}-100")
        if res100["mean_regret"] >= self.current_global_loss_100:
            return False, res100["mean_regret"]

        # Stage 2: 300 positions
        res300 = self.evaluate(candidate, self.train_300, f"{label}-300")
        if res300["mean_regret"] >= self.current_global_loss_300:
            return False, res300["mean_regret"]

        # Stage 3: 800 positions
        res800 = self.evaluate(candidate, self.train_800, f"{label}-800")
        if res800["mean_regret"] < self.current_global_loss_800:
            return True, res800["mean_regret"]
        return False, res800["mean_regret"]

    def tune_coordinate_block(self, block_id, params):
        """Direct bounded coordinate search for blocks size 1 to 3."""
        for p in params:
            if self.total_howl_nodes >= self.config["max_budget"]:
                break
            low, high = self.param_bounds[p]
            scale = self.param_scales[p]
            step = max(1, int(round(scale * 0.5)))
            current_val = self.current_global_vector[p]

            candidates = []
            for delta in [-step, step]:
                val = max(low, min(high, current_val + delta))
                if val != current_val:
                    cand = dict(self.current_global_vector)
                    cand[p] = val
                    candidates.append((cand, f"b{block_id}-{p}-{val}"))

            for cand, lbl in candidates:
                if self.total_howl_nodes >= self.config["max_budget"]:
                    break
                survived, new_loss = self.test_candidate_staged(cand, lbl)
                if survived:
                    self.current_global_vector = cand
                    self.current_global_loss_800 = new_loss
                    self.current_global_loss_300 = self.evaluate(cand, self.train_300, f"{lbl}-300")["mean_regret"]
                    self.current_global_loss_100 = self.evaluate(cand, self.train_100, f"{lbl}-100")["mean_regret"]
                    self.print_progress(self.block_ids.index(block_id))

    def tune_spsa_block(self, block_id, params):
        """Bounded SPSA for blocks size 4 to 12."""
        rng = random.Random(20260925 + block_id * 1000)
        c0 = 0.5
        gamma = 0.101
        alpha = 0.602
        a0 = 0.2
        A = 5.0
        
        # Run 4 iterations of SPSA
        for k in range(4):
            if self.total_howl_nodes >= self.config["max_budget"]:
                break
            ck = c0 / ((k + 1) ** gamma)
            ak = a0 / ((A + k + 1) ** alpha)

            delta = {p: rng.choice((-1, 1)) for p in params}
            plus = dict(self.current_global_vector)
            minus = dict(self.current_global_vector)

            for p in params:
                scale = self.param_scales[p]
                low, high = self.param_bounds[p]
                step = ck * scale * delta[p]
                plus[p] = max(low, min(high, int(round(self.current_global_vector[p] + step))))
                minus[p] = max(low, min(high, int(round(self.current_global_vector[p] - step))))

            lbl_p = f"b{block_id}-it{k}-plus"
            lbl_m = f"b{block_id}-it{k}-minus"

            # Check both candidates against staged TRAIN
            survived_p, loss_p = self.test_candidate_staged(plus, lbl_p)
            if survived_p:
                self.current_global_vector = plus
                self.current_global_loss_800 = loss_p
                self.current_global_loss_300 = self.evaluate(plus, self.train_300, f"{lbl_p}-300")["mean_regret"]
                self.current_global_loss_100 = self.evaluate(plus, self.train_100, f"{lbl_p}-100")["mean_regret"]
                self.print_progress(self.block_ids.index(block_id))
                continue

            survived_m, loss_m = self.test_candidate_staged(minus, lbl_m)
            if survived_m:
                self.current_global_vector = minus
                self.current_global_loss_800 = loss_m
                self.current_global_loss_300 = self.evaluate(minus, self.train_300, f"{lbl_m}-300")["mean_regret"]
                self.current_global_loss_100 = self.evaluate(minus, self.train_100, f"{lbl_m}-100")["mean_regret"]
                self.print_progress(self.block_ids.index(block_id))
                continue

            # If neither survived the full 800, compute gradient step from 100 positions and test gradient candidate
            res_p100 = self.evaluate(plus, self.train_100, f"{lbl_p}-100")
            res_m100 = self.evaluate(minus, self.train_100, f"{lbl_m}-100")
            grad_cand = dict(self.current_global_vector)
            for p in params:
                scale = self.param_scales[p]
                low, high = self.param_bounds[p]
                ghat = (res_p100["mean_regret"] - res_m100["mean_regret"]) / (2 * ck * delta[p])
                step = ak * scale * ghat
                grad_cand[p] = max(low, min(high, int(round(self.current_global_vector[p] - step))))

            if grad_cand != self.current_global_vector:
                survived_g, loss_g = self.test_candidate_staged(grad_cand, f"b{block_id}-it{k}-grad")
                if survived_g:
                    self.current_global_vector = grad_cand
                    self.current_global_loss_800 = loss_g
                    self.current_global_loss_300 = self.evaluate(grad_cand, self.train_300, f"b{block_id}-it{k}-grad-300")["mean_regret"]
                    self.current_global_loss_100 = self.evaluate(grad_cand, self.train_100, f"b{block_id}-it{k}-grad-100")["mean_regret"]
                    self.print_progress(self.block_ids.index(block_id))

    def run(self):
        for idx, block_id in enumerate(self.block_ids):
            if self.total_howl_nodes >= self.config["max_budget"]:
                print(f"Reached max budget limit ({self.config['max_budget']:,} nodes). Stopping tuning.", flush=True)
                break
            params = self.blocks[block_id]
            self.blocks_attempted += 1
            if len(params) <= 3:
                self.tune_coordinate_block(block_id, params)
            else:
                self.tune_spsa_block(block_id, params)
            self.print_progress(idx)

        # Save accepted parameter file
        ACCEPTED_OUT.parent.mkdir(parents=True, exist_ok=True)
        accepted_payload = {
            "parameters": self.current_global_vector,
            "baseline_regret": self.baseline_train_regret,
            "final_regret": self.current_global_loss_800,
            "howl_nodes": self.total_howl_nodes,
            "candidates_evaluated": self.total_candidates_evaluated,
            "blocks_attempted": self.blocks_attempted,
            "parameters_changed": self.count_changed()
        }
        atomic_json(ACCEPTED_OUT, accepted_payload)

if __name__ == "__main__":
    tuner = Tuner()
    tuner.run()
