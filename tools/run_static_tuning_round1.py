#!/usr/bin/env python3
"""Run the five accepted/rejected static-evaluation blocks sequentially."""

import argparse
import json
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
ARTIFACTS = ROOT / ".static-tuning-round1"
OPTION_CPP = ROOT / "Option.cpp"

BLOCKS = [
    ("Base Position", ["BaseScalars", "PST"]),
    ("Pawns", ["Pawns"]),
    ("Pieces", ["Pieces"]),
    ("King", ["King"]),
    ("Threats", ["Threats"]),
    ("Endgame", ["Endgame"]),
]


def run(command, *, capture=False):
    print("+", " ".join(map(str, command)), flush=True)
    return subprocess.run(command, cwd=ROOT, check=True, text=True,
                          capture_output=capture)


def build(*targets):
    run(["cmake", "--build", str(BUILD), "--target", *targets, "-j2"])


def parse_metrics(output):
    labels = {
        "Baseline training loss": "baseline_training_loss",
        "Baseline validation loss": "baseline_validation_loss",
        "Tuned training loss": "tuned_training_loss",
        "Tuned validation loss": "tuned_validation_loss",
        "Changed parameters": "changed_parameters",
    }
    metrics = {}
    for line in output.splitlines():
        for label, key in labels.items():
            if line.startswith(label + ":"):
                value = line.split(":", 1)[1].strip()
                metrics[key] = int(value) if key == "changed_parameters" else float(value)
    missing = [key for key in labels.values() if key not in metrics]
    if missing:
        raise RuntimeError("Missing tuner result fields: " + ", ".join(missing))
    return metrics


def load_state(path):
    values = {}
    for line in path.read_text().splitlines():
        name, value = line.split("\t")
        if name in values:
            raise RuntimeError(f"Duplicate parameter in state: {name}")
        values[name] = int(value)
    return values


def location(name):
    direct = {
        "PawnValue", "KnightValue", "BishopValue", "RookValue", "QueenValue",
        "DoubledPawnValue", "IsolatedPawnMiddleGame", "IsolatedPawnEndGame",
        "RookOpenFileMiddleGame", "RookOpenFileEndGame",
        "RookSemiOpenFileMiddleGame", "RookSemiOpenFileEndGame",
        "KnightOutpostMiddleGame", "KnightOutpostEndGame",
        "KnightSupportedOutpostMiddleGame", "KnightSupportedOutpostEndGame",
        "RookBehindPassedPawnMiddleGame", "RookBehindPassedPawnEndGame",
        "BishopPairValue", "BishopOpenFilePawnScale", "TempoMiddleGame", "TempoEndGame",
        "OppositeColorBishopMiddleGameScalePermille", "OppositeColorBishopEndGameScalePermille",
        "MaterialBalanceOffset", "PawnDeficitZeroPawnMultiplierPermille",
        "PawnDeficitOnePawnMultiplierPermille", "EndgamePawnAdvancementRankMultiplier",
        "PieceAttackScalePercent", "LoneKingBase", "LoneKingEdgeWeight",
        "LoneKingCornerWeight", "LoneKingConfinementWeight",
        "LoneKingRestrictedNeighbourWeight", "LowMaterialScalePermille",
        "KingAttackerPawnWeight", "KingAttackerMinorWeight", "KingAttackerRookWeight",
        "KingAttackerQueenWeight", "KingDefenderPawnWeight", "KingDefenderMinorWeight",
        "KingDefenderRookWeight", "KingDefenderQueenWeight",
        "KingShelterSecondRankDanger", "KingShelterAdvancedPawnDanger",
        "KingShelterMissingPawnDanger", "KingShelterOpenFileDanger",
        "KingUndefendedZoneDanger", "KingAdditionalZoneAttackerDanger",
        "KingSemiOpenLineDanger", "KingOpenLineDanger", "KingDiagonalLineDanger",
        "KingControlledEscapeDanger", "KingBlockedEscapeDanger", "KingTrappedEscapeDanger",
        "KingHeavyBatteryDanger", "CentralKingInnerMinorPressure",
        "CentralKingOuterMinorPressure", "CentralKingReadinessLagWeight",
        "CentralKingPressureScale", "KingUnreadyCoordinationWeight",
        "KingLatentActivationWeight", "KingFutureShelterWingWeight",
        "KingPinnedShelterPawnWeight", "KingInfiltratedQueenWeight",
        "PassedPawnMiddleGameFileAmplitude",
    }
    if name in direct:
        return name, None

    match = re.fullmatch(r"PassedPawn(MiddleGame|EndGame)(Base|Increment_(\d+))", name)
    if match:
        return f"PassedPawn{match.group(1)}Parameters", 0 if match.group(2) == "Base" else int(match.group(3))

    match = re.fullmatch(r"(Pawn|Knight|Bishop|Rook|Queen|King)PieceSquare(MiddleGame|EndGame)_(\d+)", name)
    if match:
        return f"{match.group(1)}PieceSquare{match.group(2)}Parameters", int(match.group(3))

    match = re.fullmatch(r"(Knight|Bishop|Rook|Queen)Mobility(MiddleGame|EndGame)(Base|Increment_(\d+))", name)
    if match:
        return f"{match.group(1)}Mobility{match.group(2)}Parameters", 0 if match.group(3) == "Base" else int(match.group(4))

    match = re.fullmatch(r"(Pawn|Knight|Bishop|Rook|Queen|King)Attack(Pawn|Knight|Bishop|Rook|Queen)_(MiddleGame|EndGame)", name)
    if match:
        victim = {"Pawn": 1, "Knight": 2, "Bishop": 3, "Rook": 4, "Queen": 5}[match.group(2)]
        return f"{match.group(1)}AttackValue{match.group(3)}", victim
    raise RuntimeError(f"No production mapping for parameter: {name}")


def apply_state(path):
    text = OPTION_CPP.read_text()
    arrays = {}
    scalars = {}
    for name, value in load_state(path).items():
        symbol, index = location(name)
        if index is None:
            scalars[symbol] = value
        else:
            arrays.setdefault(symbol, {})[index] = value

    for symbol, value in scalars.items():
        pattern = rf"(int Option::{re.escape(symbol)}\s*=\s*)-?\d+(\s*;)"
        text, count = re.subn(pattern, rf"\g<1>{value}\g<2>", text, count=1)
        if count != 1:
            raise RuntimeError(f"Could not update scalar Option::{symbol}")

    for symbol, updates in arrays.items():
        pattern = rf"int Option::{re.escape(symbol)}\s*\[[^]]*\]\s*=\s*\{{([^}}]*)\}};"
        match = re.search(pattern, text, re.S)
        if not match:
            raise RuntimeError(f"Could not update array Option::{symbol}")
        values = [int(value) for value in re.findall(r"-?\d+", match.group(1))]
        for index, value in updates.items():
            if index >= len(values):
                raise RuntimeError(f"Index {index} outside Option::{symbol}")
            values[index] = value
        replacement = f"int Option::{symbol}[] = {{{', '.join(map(str, values))}}};"
        text = text[:match.start()] + replacement + text[match.end():]
    OPTION_CPP.write_text(text)


def selection_info(groups, state_path=None):
    command = [str(BUILD / "howl_tuner"), "--list-selection"]
    if state_path:
        command += ["--state-out", str(state_path)]
    result = run(command + groups, capture=True)
    print(result.stdout, end="")
    fields = dict(re.findall(r"^([^:]+):\s*(.+)$", result.stdout, re.M))
    return int(fields["Canonical parameters"]), int(fields["Selected parameters"]), fields["Duplicate parameters"]


def main():
    global ARTIFACTS
    parser = argparse.ArgumentParser()
    parser.add_argument("--round", type=int, default=1,
                        help="numbered tuning round (default: 1)")
    parser.add_argument("--blocks", nargs="+", metavar="BLOCK",
                        help="run only selected conceptual blocks")
    parser.add_argument("--dry-run", action="store_true",
                        help="build and validate orchestration without tuning or changing production values")
    args = parser.parse_args()
    if args.round < 1:
        parser.error("--round must be a positive integer")

    selected_blocks = BLOCKS
    if args.blocks:
        block_by_key = {
            name.lower().replace(" ", "").replace("-", ""): (name, groups)
            for name, groups in BLOCKS
        }
        selected_blocks = []
        seen = set()
        for requested in args.blocks:
            key = requested.lower().replace(" ", "").replace("-", "")
            if key not in block_by_key:
                parser.error(f"unknown conceptual block: {requested}")
            block = block_by_key[key]
            if block[0] not in seen:
                selected_blocks.append(block)
                seen.add(block[0])

    ARTIFACTS = ROOT / f".static-tuning-round{args.round}"

    ARTIFACTS.mkdir(exist_ok=True)
    build("howl", "howl_tuner")
    initial_state = ARTIFACTS / "initial-state.tsv"
    canonical, _, duplicate = selection_info(
        [group for _, groups in selected_blocks for group in groups], initial_state)
    if duplicate != "no":
        raise RuntimeError("Canonical registry contains duplicate parameter names")

    summary = {"round": args.round, "starting_canonical_parameter_count": canonical, "blocks": []}
    for block_name, groups in selected_blocks:
        block_canonical, selected, block_duplicate = selection_info(groups)
        if block_canonical != canonical or block_duplicate != "no":
            raise RuntimeError(f"Registry validation failed for {block_name}")
        summary["blocks"].append({"name": block_name, "groups": groups,
                                  "selected_parameters": selected})

    if args.dry_run:
        summary.update({"dry_run": True, "final_canonical_parameter_count": canonical,
                        "final_build_result": "passed"})
        (ARTIFACTS / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
        return

    before = ROOT / "match-engines" / f"howl-before-static-round{args.round}"
    after = ROOT / "match-engines" / f"howl-static-round{args.round}"
    before.parent.mkdir(exist_ok=True)
    shutil.copy2(BUILD / "howl", before)

    summary["blocks"] = []
    for slug, (block_name, groups) in enumerate(selected_blocks, 1):
        state_path = ARTIFACTS / f"{slug}-{block_name.lower().replace(' ', '-')}.tsv"
        result = run([str(BUILD / "howl_tuner"), "--state-out", str(state_path), *groups], capture=True)
        print(result.stdout, end="")
        metrics = parse_metrics(result.stdout)
        accepted = metrics["tuned_validation_loss"] < metrics["baseline_validation_loss"]
        metrics.update({"name": block_name, "groups": groups, "accepted": accepted})
        summary["blocks"].append(metrics)
        if accepted:
            apply_state(state_path)
            build("howl", "howl_tuner")

    build("howl", "howl_tuner")
    shutil.copy2(BUILD / "howl", after)
    final_canonical, _, final_duplicate = selection_info(
        [group for _, groups in selected_blocks for group in groups])
    if final_duplicate != "no":
        raise RuntimeError("Final canonical registry contains duplicate parameter names")
    summary.update({"dry_run": False, "final_canonical_parameter_count": final_canonical,
                    "final_production_files_changed": ["Option.cpp"],
                    "final_build_result": "passed",
                    "before_binary": str(before.relative_to(ROOT)),
                    "after_binary": str(after.relative_to(ROOT))})
    (ARTIFACTS / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")


if __name__ == "__main__":
    main()
