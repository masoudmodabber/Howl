#!/usr/bin/env python3
"""Resumable blockwise SPSA over the frozen external move-regret objective."""

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

from build_frozen_move_quality_baseline import StockfishUCI, analyze_move, atomic_json, init_worker

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "move-quality-tuning"
RUN = OUT / "run"
INVENTORY = ROOT / ".eval-identifiability/parameter-inventory.tsv"
STATS = ROOT / ".eval-identifiability/parameter-statistics.tsv"
FAMILY_STATS = ROOT / ".eval-identifiability/family-summary.tsv"
CORRELATIONS = ROOT / ".eval-identifiability/family-correlations.tsv"
SPLIT = ROOT / "move-quality-reference/frozen-split-800-200.tsv"
BASELINE = ROOT / "move-quality-reference/frozen-howl-baseline.tsv"
REFERENCE = ROOT / "move-quality-reference/reference-cache-5m-multipv8.json"
CACHE = ROOT / "move-quality-reference/constrained-move-cache-5m.json"
RUNNER = ROOT / "tools/run_search_objective_parallel.py"
CONFIG = OUT / "tuning-config.json"


def read_tsv(path):
    with Path(path).open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def write_tsv(path, rows, fields=None):
    path = Path(path); path.parent.mkdir(parents=True, exist_ok=True)
    fields = fields or list(rows[0])
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t")
        writer.writeheader(); writer.writerows(rows)


def stable_hash(text):
    return int.from_bytes(hashlib.sha256(text.encode()).digest()[:8], "big")


def family_block(name):
    if name.startswith("PieceValue") or name == "Inline": return "MaterialGlobal"
    if name in ("PawnStructure", "IsolatedPawn"): return "PawnStructure"
    if name in ("PassedPawnV2", "RookBehindPassedPawn"): return "PassedPawns"
    if name in ("RookFile", "KnightOutpost"): return "PieceFeatures"
    if name == "EndgameWeights": return "Endgame"
    return name


def phase(name):
    if "MiddleGame" in name: return "MG"
    if "EndGame" in name: return "EG"
    return "Both/Scalar"


def bounds_and_scale(row, policy):
    value = int(row["current_value"]); name = row["name"]; family = row["family"]
    scale = max(policy["default_natural_scale_cp"], abs(value) * policy["scale_fraction_of_magnitude"])
    if "Permille" in name: scale = max(scale, 10.0)
    if "Percent" in name: scale = max(scale, 2.0)
    span = int(math.ceil(scale * policy["minimum_bound_span_scales"]))
    low, high = value - span, value + span
    if family in ("Attack", "KingSafety") or "Increment_" in name or name in {
            "BishopPairValue", "PassedPawnMiddleGameFileAmplitude"}:
        low = 0
    if "ScalePermille" in name: low, high = 0, 1000
    return scale, low, high


def build_manifest(config):
    inventory = read_tsv(INVENTORY); stats = {r["name"]: r for r in read_tsv(STATS)}
    family_stats = {r["family"]: r for r in read_tsv(FAMILY_STATS)}
    correlations = read_tsv(CORRELATIONS)
    strongest = {}
    for row in correlations:
        corr = abs(float(row["correlation"]))
        for name in (row["parameter_a"], row["parameter_b"]):
            strongest[name] = max(strongest.get(name, 0.0), corr)
    policy = config["parameter_policy"]
    manifest, decisions = [], []
    seen_signatures = {}
    for row in inventory:
        stat = stats[row["name"]]; support = int(stat["nonzero_positions"])
        variance = float(stat["standard_deviation"]) ** 2
        reason = ""
        if support <= policy["zero_support_threshold"]: reason = "zero_support"
        elif variance <= policy["near_zero_variance_threshold"]: reason = "near_zero_variance"
        signature = (support, stat["mean"], stat["standard_deviation"], stat["minimum"], stat["maximum"], stat["positive"], stat["negative"])
        if not reason and signature in seen_signatures and strongest.get(row["name"], 0) >= policy["exact_duplicate_correlation"]:
            reason = "exact_duplicate_of:" + seen_signatures[signature]
        seen_signatures.setdefault(signature, row["name"])
        # PST, mobility, and passed-pawn tables are already compact in the canonical
        # registry.  No further exact coordinate mapping is justified, so every
        # surviving canonical value remains an independent SPSA coordinate.
        status = "frozen" if reason else "free"
        scale, low, high = bounds_and_scale(row, policy)
        family = family_stats[row["family"]]
        manifest.append({"name": row["name"], "family": row["family"], "current_value": row["current_value"],
            "type": "integer", "phase": row["phase"], "source_location": row["source_location"],
            "natural_scale": f"{scale:.12g}", "minimum": low, "maximum": high,
            "corpus_support": support, "variance": f"{variance:.12g}",
            "strongest_within_family_correlation": f"{strongest.get(row['name'], 0.0):.12g}",
            "family_effective_rank": family["effective_rank"], "family_parameter_count": family["parameter_count"],
            "status": status, "optimization_coordinate": "" if status == "frozen" else row["name"],
            "coordinate_mapping": "frozen" if status == "frozen" else "identity_1_to_1",
            "block": family_block(row["family"])})
        decisions.append({"name": row["name"], "status": status, "reason": reason or "independent_canonical_coordinate"})
    if len(manifest) != 261 or len({r["name"] for r in manifest}) != 261:
        raise RuntimeError("canonical identifiability inventory is not 261 unique parameters")
    write_tsv(OUT / "parameter-manifest.tsv", manifest)
    write_tsv(OUT / "parameter-decisions.tsv", decisions)
    return manifest


def build_blocks(manifest):
    active = [r for r in manifest if r["status"] != "frozen"]
    blocks = {}
    for row in active:
        block = row["block"]
        if row["family"] == "PieceSquare":
            block = "PST-" + row["name"].split("PieceSquare")[0]
        elif row["family"] == "Attack":
            block = "Attack-" + re.match(r"(Pawn|Knight|Bishop|Rook|Queen|King)Attack", row["name"]).group(1)
        elif "Mobility" in row["family"]:
            block = row["family"]
        blocks.setdefault(block, []).append(row["name"])
    rows = []
    for order, (name, parameters) in enumerate(blocks.items()):
        for index, parameter in enumerate(parameters):
            rows.append({"block_order": order, "block": name, "raw_parameter_count": len(parameters),
                         "optimization_coordinate_count": len(parameters), "parameter_order": index,
                         "optimization_coordinate": parameter, "raw_parameters": parameter})
    write_tsv(OUT / "blocks.tsv", rows)
    mapping = [{"representation": family, "raw_parameter_count": len(items),
                "optimization_coordinate_count": len(items), "mapping": "identity_1_to_1",
                "reason": "canonical representation is already structural; no further justified compression"}
               for family, items in blocks.items()]
    write_tsv(OUT / "coordinate-mapping.tsv", mapping)
    return blocks


def replace_option_source(vector, destination):
    source = (ROOT / "Option.cpp").read_text(encoding="utf-8")
    aliases = {}
    victims = {"Pawn":1,"Knight":2,"Bishop":3,"Rook":4,"Queen":5}
    for name, value in vector.items():
        match = re.fullmatch(r"(Pawn|Knight|Bishop|Rook|Queen|King)PieceSquare(MiddleGame|EndGame)_(\d+)", name)
        if match: aliases[f"{match.group(1)}PieceSquare{match.group(2)}Parameters_{match.group(3)}"] = value; continue
        match = re.fullmatch(r"PassedPawn(MiddleGame|EndGame)(Base|Increment_([1-5]))", name)
        if match: aliases[f"PassedPawn{match.group(1)}Parameters_{0 if match.group(2)=='Base' else int(match.group(3))}"] = value; continue
        match = re.fullmatch(r"(Knight|Bishop|Rook|Queen)Mobility(MiddleGame|EndGame)(Base|Increment_([1-4]))", name)
        if match: aliases[f"{match.group(1)}Mobility{match.group(2)}Parameters_{0 if match.group(3)=='Base' else int(match.group(4))}"] = value; continue
        match = re.fullmatch(r"(Pawn|Knight|Bishop|Rook|Queen|King)Attack(Pawn|Knight|Bishop|Rook|Queen)_(MiddleGame|EndGame)", name)
        if match: aliases[f"{match.group(1)}AttackValue{match.group(3)}_{victims[match.group(2)]}"] = value; continue
        aliases[name] = value
    arrays = {}
    for name, value in aliases.items():
        if re.search(rf"int Option::{re.escape(name)}\s*=", source):
            source = re.sub(rf"(int Option::{re.escape(name)}\s*=\s*)-?\d+;", rf"\g<1>{int(round(value))};", source)
        elif re.search(r"_\d+$", name):
            array, index = name.rsplit("_", 1); arrays.setdefault(array, {})[int(index)] = int(round(value))
    for array, changes in arrays.items():
        pattern = rf"(int Option::{re.escape(array)}(?:\[\d*\])?\s*=\s*\{{)([^}}]+)(\}};)"
        match = re.search(pattern, source, re.S)
        if not match: raise RuntimeError(f"cannot inject parameter array {array}")
        values = [int(x.strip()) for x in match.group(2).split(",") if x.strip()]
        for index, value in changes.items(): values[index] = value
        body = "\n  " + ",\n  ".join(", ".join(f"{x:4d}" for x in values[i:i+8]) for i in range(0,len(values),8)) + "\n"
        source = source[:match.start()] + match.group(1) + body + match.group(3) + source[match.end():]
    destination.write_text(source, encoding="utf-8")


def build_variant(vector, key):
    directory = RUN / "state/build" / (key + "-objective"); directory.mkdir(parents=True, exist_ok=True)
    binary = directory / "howl"
    if binary.exists(): return binary
    generated = directory / "Option.cpp"; obj = directory / "Option.cpp.o"
    replace_option_source(vector, generated)
    subprocess.run(["c++","-O3","-DNDEBUG","-std=gnu++17",f"-I{ROOT}",f"-I{ROOT/'third_party/fathom/src'}","-c",str(generated),"-o",str(obj)], check=True)
    objects = [str(p) for p in (ROOT/"build/CMakeFiles/howl_search_tuner_objective.dir").rglob("*.o") if p.name != "Option.cpp.o"]
    subprocess.run(["c++","-O3","-DNDEBUG",*objects,str(obj),str(ROOT/"build/libfathom.a"),"-lpthread","-o",str(binary)], check=True)
    return binary


def vector_key(vector):
    return hashlib.sha256(json.dumps(vector, sort_keys=True).encode()).hexdigest()[:16]


def sample_rows(rows, epoch, size, seed):
    ordered = list(rows); random.Random(seed + epoch // len(rows)).shuffle(ordered)
    start = epoch % len(rows)
    return (ordered + ordered)[start:start+size]


def selected_moves(label):
    path = RUN / "iterations" / label / "results.tsv"
    return {row["position_id"]: row["move"] for row in read_tsv(path)}


def move_disagreement(label_plus, label_minus):
    plus, minus = selected_moves(label_plus), selected_moves(label_minus)
    if plus.keys() != minus.keys(): raise RuntimeError("paired objective identities differ")
    changed = sum(plus[key] != minus[key] for key in plus)
    return changed, changed / len(plus)


def calibrate_c(block_index, block, names, vector, rows, config, manifest):
    path = RUN / "state/calibration" / f"{block_index:02d}-{block}.json"
    if path.exists(): return json.loads(path.read_text(encoding="utf-8"))
    path.parent.mkdir(parents=True, exist_ok=True)
    rng = random.Random(config["seed"] + block_index * 100000 + 70000)
    delta = {name: rng.choice((-1, 1)) for name in names}
    by_name = {row["name"]: row for row in manifest}
    probes = []
    for scale in config["calibration"]["normalized_scales"]:
        plus, minus = dict(vector), dict(vector)
        for name in names:
            parameter = by_name[name]; step = scale * float(parameter["natural_scale"]) * delta[name]
            plus[name] = max(int(parameter["minimum"]), min(int(parameter["maximum"]), round(vector[name] + step)))
            minus[name] = max(int(parameter["minimum"]), min(int(parameter["maximum"]), round(vector[name] - step)))
        tag = str(scale).replace(".", "p")
        prefix = f"calibration/{block_index:02d}-{block}/c-{tag}"
        plus_result = evaluate(plus, rows, prefix + "-plus", config)
        minus_result = evaluate(minus, rows, prefix + "-minus", config)
        changed, fraction = move_disagreement(prefix + "-plus", prefix + "-minus")
        probes.append({"scale": scale, "loss_plus": plus_result["mean_regret"],
            "loss_minus": minus_result["mean_regret"],
            "absolute_loss_difference": abs(plus_result["mean_regret"] - minus_result["mean_regret"]),
            "different_root_moves": changed, "root_move_disagreement_fraction": fraction,
            "delta": delta, "position_ids": [row["position_id"] for row in rows]})
    low = config["calibration"]["minimum_root_move_disagreement"]
    high = config["calibration"]["maximum_root_move_disagreement"]
    inside = [probe for probe in probes if low <= probe["root_move_disagreement_fraction"] <= high]
    if inside:
        selected = min(inside, key=lambda probe: probe["scale"]); reason = "smallest_scale_inside_target_band"
    else:
        def distance(probe):
            fraction = probe["root_move_disagreement_fraction"]
            return low - fraction if fraction < low else fraction - high
        selected = min(probes, key=lambda probe: (distance(probe), probe["scale"]))
        reason = "closest_scale_to_target_band"
    result = {"block": block, "selected_c": selected["scale"], "selection_reason": reason,
              "target_disagreement_band": [low, high], "probes": probes}
    atomic_json(path, result)
    return result


def evaluate(vector, rows, label, config):
    key = vector_key(vector); directory = RUN / "iterations" / label; directory.mkdir(parents=True, exist_ok=True)
    summary_path = directory / "summary.json"; detail_path = directory / "results.tsv"
    if summary_path.exists(): return json.loads(summary_path.read_text())
    sample = directory / "sample.tsv"; write_tsv(sample, rows)
    binary = build_variant(vector, key)
    howl = directory / "howl.tsv"; howl_summary = directory / "howl-summary.json"
    subprocess.run([sys.executable,str(RUNNER),"--sample",str(sample),"--engine",str(binary),
        "--workers",str(config["howl_workers"]),"--nodes",str(config["howl_nodes"]),
        "--results-out",str(howl),"--summary-out",str(howl_summary)], check=True)
    howl_rows = {r["position_id"]:r for r in read_tsv(howl)}
    cache = json.loads(CACHE.read_text()); reference = json.loads(REFERENCE.read_text())
    tasks=[]
    for row in rows:
        pid=row["position_id"]; move=howl_rows[pid]["best_move"]
        position=cache["positions"].setdefault(pid,{"fen":row["fen"],"moves":{}})
        if move not in position["moves"]: tasks.append((pid,row["fen"],move))
    sf_nodes=0
    if tasks:
        with concurrent.futures.ProcessPoolExecutor(max_workers=config["stockfish_workers"], initializer=init_worker,
                initargs=(config["stockfish_path"],config["stockfish_nodes"],config["stockfish_hash_mb"])) as pool:
            for future in concurrent.futures.as_completed([pool.submit(analyze_move,t) for t in tasks]):
                pid,move,record,nodes=future.result(); cache["positions"][pid]["moves"][move]=record; sf_nodes+=nodes; atomic_json(CACHE,cache)
    detail=[]
    for row in rows:
        pid=row["position_id"]; move=howl_rows[pid]["best_move"]; ref=reference["positions"][pid]["best_move"]
        rq=float(cache["positions"][pid]["moves"][ref]["quality"]); q=float(cache["positions"][pid]["moves"][move]["quality"])
        detail.append({"position_id":pid,"fen":row["fen"],"move":move,"reference_move":ref,"quality":q,
            "reference_quality":rq,"raw_regret":rq-q,"regret":max(0,rq-q),"searched_score":howl_rows[pid]["searched_score"],"nodes":howl_rows[pid]["nodes"]})
    write_tsv(detail_path,detail); regrets=[r["regret"] for r in detail]
    result={"label":label,"vector_key":key,"positions":len(rows),"mean_regret":sum(regrets)/len(regrets),
            "howl_nodes":sum(int(r["nodes"]) for r in detail),"additional_stockfish_nodes":sf_nodes}
    atomic_json(summary_path,result); return result


def save_vector(name, vector): atomic_json(RUN/"vectors"/f"{name}.json",vector)


def run(args, config, manifest, blocks):
    for sub in ("state","vectors","iterations","train-checkpoints","validation-checkpoints","stockfish-cache","logs"):
        (RUN/sub).mkdir(parents=True,exist_ok=True)
    split=read_tsv(SPLIT); train=[r for r in split if r["split"]=="train"]; validation=[r for r in split if r["split"]=="validation"]
    if len(train)!=800 or len(validation)!=200: raise RuntimeError("frozen split mismatch")
    vector={r["name"]:int(r["current_value"]) for r in manifest}; initial=dict(vector)
    state_path=RUN/"state/state.json"
    small_test = args.pilot or args.structural_test
    if args.resume and state_path.exists():
        state=json.loads(state_path.read_text()); vector=state["current_vector"]
    else:
        frozen_train = [float(row["regret"]) for row in read_tsv(BASELINE) if row["split"] == "train"]
        baseline_train_loss = sum(frozen_train) / len(frozen_train)
        state={"mode":"structural-test" if args.structural_test else "pilot" if args.pilot else "full",
               "phase":"blocks","block_index":0,"iteration":0,
               "sample_cursor":0,"current_vector":vector,"accepted_vector":dict(vector),
               "accepted_train_loss":baseline_train_loss,"best_train_loss":baseline_train_loss,
               "best_train_vector":dict(vector),"completed":False}
        save_vector("initial-production",initial); save_vector("best-train",initial); atomic_json(state_path,state)
    selected_blocks=list(blocks.items())
    if small_test:
        selected_blocks=[("KnightOutpost",[r["name"] for r in manifest if r["family"]=="KnightOutpost" and r["status"]!="frozen"])]
    else:
        selected_blocks.append(("Global", [r["name"] for r in manifest if r["status"] != "frozen"]))
    batch_size=(config["structural_test"]["minibatch_size"] if args.structural_test else 20 if args.pilot else config["minibatch_size"])
    checkpoint_size=(config["structural_test"]["checkpoint_positions"] if args.structural_test else 100 if args.pilot else 800)
    howl_nodes=(config["structural_test"]["howl_nodes"] if args.structural_test else 10000 if args.pilot else config["howl_nodes"])
    config=dict(config); config["howl_nodes"]=howl_nodes
    if args.structural_test:
        config["howl_workers"] = config["structural_test"]["workers"]
        config["stockfish_workers"] = config["structural_test"]["workers"]
        config["stockfish_nodes"] = config["structural_test"]["stockfish_nodes"]
        config["early_stopping"] = dict(config["early_stopping"])
        config["early_stopping"].update({"patience":config["structural_test"]["patience"],
            "minimum_minibatch_improvement":1.0,"negligible_loss_separation":1.0,
            "essentially_unchanged_move_fraction":1.0})
    for bi in range(state["block_index"],len(selected_blocks)):
        block, names=selected_blocks[bi]; accepted=dict(state["accepted_vector"])
        iterations = (config["structural_test"]["iterations"] if args.structural_test else 2 if args.pilot else
                      config["global_iterations"] if block == "Global" else config["iterations_per_block"])
        incoming_train_loss = state["accepted_train_loss"]
        if block == "Global": state["phase"] = "global"
        start_iteration=state["iteration"] if bi==state["block_index"] else 0
        calibration_rows = sample_rows(train, state["sample_cursor"], batch_size, config["seed"])
        calibration = calibrate_c(bi, block, names, vector, calibration_rows, config, manifest)
        calibrated_c = calibration["selected_c"]
        local_best_loss = state.get("block_best_minibatch_loss") if start_iteration else None
        no_best_iterations = state.get("block_no_best_iterations", 0) if start_iteration else 0
        negligible_iterations = state.get("block_negligible_iterations", 0) if start_iteration else 0
        stop_reason = "completed_configured_iterations"
        last_iteration = start_iteration
        for k in range(start_iteration,iterations):
            rng=random.Random(config["seed"]+bi*100000+k); delta={n:rng.choice((-1,1)) for n in names}
            ck=calibrated_c/(k+1)**config["spsa"]["gamma"]
            batch=sample_rows(train,state["sample_cursor"],batch_size,config["seed"]); state["sample_cursor"]+=batch_size
            plus=dict(vector); minus=dict(vector); by_name={r["name"]:r for r in manifest}
            for n in names:
                p=by_name[n]; step=ck*float(p["natural_scale"])*delta[n]
                plus[n]=max(int(p["minimum"]),min(int(p["maximum"]),round(vector[n]+step)))
                minus[n]=max(int(p["minimum"]),min(int(p["maximum"]),round(vector[n]-step)))
            prefix=f"block-{bi:02d}-{block}/iteration-{k:03d}"
            lp=evaluate(plus,batch,prefix+"-plus",config)["mean_regret"]
            lm=evaluate(minus,batch,prefix+"-minus",config)["mean_regret"]
            changed_moves, changed_fraction = move_disagreement(prefix+"-plus", prefix+"-minus")
            minibatch_loss, minibatch_vector = (lp, plus) if lp <= lm else (lm, minus)
            if state.get("best_minibatch_loss") is None or minibatch_loss < state["best_minibatch_loss"]:
                state["best_minibatch_loss"] = minibatch_loss
                state["best_minibatch_vector"] = dict(minibatch_vector)
                save_vector("best-minibatch", minibatch_vector)
            threshold = config["early_stopping"]["minimum_minibatch_improvement"]
            if local_best_loss is None or minibatch_loss < local_best_loss - threshold:
                local_best_loss = minibatch_loss; no_best_iterations = 0
            else: no_best_iterations += 1
            negligible = (abs(lp-lm) <= config["early_stopping"]["negligible_loss_separation"] or
                          changed_fraction <= config["early_stopping"]["essentially_unchanged_move_fraction"])
            negligible_iterations = negligible_iterations + 1 if negligible else 0
            ak=config["spsa"]["a"]/(config["spsa"]["A"]+k+1)**config["spsa"]["alpha"]
            for n in names:
                p=by_name[n]; gradient=(lp-lm)/(2*ck*delta[n]); normalized=vector[n]/float(p["natural_scale"])-ak*gradient
                vector[n]=max(int(p["minimum"]),min(int(p["maximum"]),round(normalized*float(p["natural_scale"]))))
            iteration_record={"block":block,"iteration":k,"minibatch_ids":[r["position_id"] for r in batch],
                "plus_minibatch_ids":[r["position_id"] for r in batch],"minus_minibatch_ids":[r["position_id"] for r in batch],
                "delta":delta,"loss_plus":lp,"loss_minus":lm,"absolute_loss_separation":abs(lp-lm),
                "different_root_moves":changed_moves,"root_move_disagreement_fraction":changed_fraction,
                "a_k":ak,"c_k":ck,"calibrated_c":calibrated_c,"vector":vector}
            atomic_json(RUN/"iterations"/prefix/"iteration.json",iteration_record)
            state.update({"block_index":bi,"iteration":k+1,"current_vector":vector,
                "block_best_minibatch_loss":local_best_loss,"block_no_best_iterations":no_best_iterations,
                "block_negligible_iterations":negligible_iterations}); atomic_json(state_path,state)
            last_iteration = k + 1
            checkpoint_due = (k + 1 == iterations)
            if not small_test and block == "Global":
                checkpoint_due = ((k + 1) % config["global_full_train_checkpoint_frequency"] == 0 or
                                  k + 1 == iterations)
            if checkpoint_due:
                checkpoint=train[:checkpoint_size]
                result=evaluate(vector,checkpoint,f"../train-checkpoints/block-{bi:02d}-{block}-{k+1:03d}",config)
                if state["best_train_loss"] is None or result["mean_regret"]<state["best_train_loss"]:
                    state["best_train_loss"]=result["mean_regret"]; state["best_train_vector"]=dict(vector); save_vector("best-train",vector)
                elif result["mean_regret"]>state["best_train_loss"]+config["rollback_tolerance"]:
                    vector=dict(state["best_train_vector"])
                atomic_json(state_path,state)
            patience = config["early_stopping"]["patience"]
            if no_best_iterations >= patience and negligible_iterations >= patience:
                stop_reason = "no_minibatch_improvement_and_negligible_paired_signal"
                break
        # Every ordinary block receives exactly one authoritative checkpoint at
        # completion. An early-stopped global segment also checkpoints its final state.
        needs_final_checkpoint = block != "Global" or last_iteration % config["global_full_train_checkpoint_frequency"] != 0
        if needs_final_checkpoint:
            result=evaluate(vector,train[:checkpoint_size],f"../train-checkpoints/block-{bi:02d}-{block}-{last_iteration:03d}",config)
            if result["mean_regret"] < state["best_train_loss"]:
                state["best_train_loss"]=result["mean_regret"]; state["best_train_vector"]=dict(vector); save_vector("best-train",vector)
            atomic_json(state_path,state)
        improved = state.get("best_train_loss") is not None and state["best_train_loss"] < incoming_train_loss
        candidate=dict(state["best_train_vector"]) if improved else dict(accepted)
        accepted=candidate
        vector=dict(accepted)
        if improved: state["accepted_train_loss"] = state["best_train_loss"]
        stop_record={"block":block,"iterations_completed":last_iteration,"configured_iterations":iterations,
                     "stop_reason":stop_reason,"accepted":improved,"incoming_train_loss":incoming_train_loss,
                     "outgoing_train_loss":state["accepted_train_loss"]}
        atomic_json(RUN/"state"/"early-stops"/f"{bi:02d}-{block}.json",stop_record)
        state.update({"block_index":bi+1,"iteration":0,"current_vector":vector,"accepted_vector":accepted,
            "block_best_minibatch_loss":None,"block_no_best_iterations":0,"block_negligible_iterations":0})
        save_vector("train-accepted",accepted); atomic_json(state_path,state)
    final_vector = dict(state.get("best_train_vector", vector))
    if not small_test:
        result=evaluate(final_vector,validation,"../validation-checkpoints/final-best-train",config)
        baseline={r["position_id"]:float(r["regret"]) for r in read_tsv(BASELINE) if r["split"]=="validation"}
        detail=read_tsv(RUN/"validation-checkpoints/final-best-train/results.tsv")
        deltas=[float(r["regret"])-baseline[r["position_id"]] for r in detail]; rng=random.Random(config["seed"]+900000)
        boot=[sum(deltas[rng.randrange(200)] for _ in range(200))/200 for _ in range(config["validation_bootstrap_replicates"])]
        ordered=sorted(boot)
        result.update({"delta_vs_frozen_validation":sum(deltas)/200,
            "bootstrap_95_low":ordered[int(.025*len(ordered))],
            "bootstrap_95_high":ordered[int(.975*len(ordered))-1],
            "p_delta_lt_zero":sum(x<0 for x in boot)/len(boot)})
        atomic_json(RUN/"validation-checkpoints/final-best-train/result.json",result)
    state["completed"]=True; state["phase"]="complete"; state["current_vector"]=final_vector; save_vector("best-global",final_vector); atomic_json(state_path,state)
    print(json.dumps({"mode":state["mode"],"completed":True,"blocks":len(selected_blocks),"best_train_loss":state.get("best_train_loss")},indent=2))


def dry_run(config, manifest, blocks, pilot=False):
    active=sum(r["status"]!="frozen" for r in manifest)
    iterations = 2 if pilot else len(blocks)*config["iterations_per_block"] + config["global_iterations"]
    checkpoints=(1 if pilot else len(blocks) +
                 math.ceil(config["global_iterations"]/config["global_full_train_checkpoint_frequency"]))
    validations=0 if pilot else 1; batch=20 if pilot else config["minibatch_size"]; nodes=10000 if pilot else config["howl_nodes"]
    calibration_evaluations = (0 if pilot else (len(blocks) + 1) * len(config["calibration"]["normalized_scales"]) * 2)
    positions=iterations*2*batch+checkpoints*(100 if pilot else 800)+validations*200
    positions += calibration_evaluations * batch
    patience=config["early_stopping"]["patience"]
    earliest=patience+1
    possible_saved_iterations=max(0,config["iterations_per_block"]-earliest)*len(blocks)+max(0,config["global_iterations"]-earliest)
    report={"parameters":len(manifest),"statuses":{s:sum(r["status"]==s for r in manifest) for s in ("free","frozen","grouped","compressed")},
        "independent_spsa_coordinates":active,"blocks":len(blocks),"block_coordinate_sizes":{k:len(v) for k,v in blocks.items()},
        "calibration_objective_evaluations":calibration_evaluations,
        "maximum_spsa_objective_evaluations":iterations*2,
        "maximum_total_objective_evaluations":iterations*2+calibration_evaluations,
        "maximum_full_train_checkpoints":checkpoints,"validation_evaluations":validations,
        "maximum_howl_nodes":positions*nodes,
        "maximum_possible_early_stop_node_savings_not_guaranteed":possible_saved_iterations*2*batch*nodes}
    atomic_json(OUT/("pilot-plan.json" if pilot else "dry-run-plan.json"),report); print(json.dumps(report,indent=2))


def main():
    global RUN, CACHE
    parser=argparse.ArgumentParser(); parser.add_argument("--config",default=str(CONFIG)); parser.add_argument("--dry-run",action="store_true"); parser.add_argument("--pilot",action="store_true"); parser.add_argument("--structural-test",action="store_true"); parser.add_argument("--resume",action="store_true"); args=parser.parse_args()
    if sum((args.dry_run,args.pilot,args.structural_test)) > 1: parser.error("select only one mode")
    if args.dry_run and args.resume: parser.error("--dry-run and --resume are incompatible")
    config=json.loads(Path(args.config).read_text()); manifest=build_manifest(config); blocks=build_blocks(manifest)
    if args.dry_run: dry_run(config,manifest,blocks); return
    if args.structural_test:
        RUN = OUT / "structural-test"
        CACHE = RUN / "stockfish-cache/constrained.json"
        if not CACHE.exists(): CACHE.parent.mkdir(parents=True,exist_ok=True); shutil.copy2(ROOT/"move-quality-reference/constrained-move-cache-5m.json",CACHE)
    run(args,config,manifest,blocks)


if __name__=="__main__": main()
