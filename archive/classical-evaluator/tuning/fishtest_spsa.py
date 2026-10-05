#!/usr/bin/env python3
"""Local paired-self-play implementation of Fishtest classic SPSA for Howl."""

from __future__ import annotations

import argparse
from concurrent.futures import ProcessPoolExecutor, as_completed
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import random
import shutil
import subprocess
import sys
from typing import Any

TOOLS = Path(__file__).resolve().parents[1]
ROOT = TOOLS.parent
sys.path.insert(0, str(TOOLS))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from tune_evaluator_move_regret import replace_option_source  # noqa: E402
from spsa_tune import (  # noqa: E402
    STARTING_POSITIONS,
    SingleGameTask,
    SingleGameOutput,
    _run_single_game_worker,
)


def atomic_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    os.replace(temporary, path)


def read_manifest(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream, delimiter="\t"))
    if not rows or len({row["name"] for row in rows}) != len(rows):
        raise RuntimeError("parameter manifest is empty or contains duplicate names")
    return rows


def vector_key(vector: dict[str, int]) -> str:
    return hashlib.sha256(json.dumps(vector, sort_keys=True).encode()).hexdigest()[:16]


def build_variant(repo: Path, build_dir: Path, state_dir: Path,
                  vector: dict[str, int], label: str) -> Path:
    key = vector_key(vector)
    directory = state_dir / "builds" / f"{key}-{label}"
    binary = directory / "howl"
    if binary.exists():
        return binary
    directory.mkdir(parents=True, exist_ok=True)
    generated = directory / "Option.cpp"
    obj = directory / "Option.cpp.o"
    replace_option_source(vector, generated)
    subprocess.run([
        "c++", "-O3", "-DNDEBUG", "-std=gnu++17", f"-I{repo}",
        f"-I{repo / 'third_party/fathom/src'}", "-c", str(generated), "-o", str(obj),
    ], check=True)
    objects = [str(p) for p in (build_dir / "CMakeFiles/howl.dir").rglob("*.o")
               if p.name != "Option.cpp.o"]
    if not objects:
        raise RuntimeError("Howl objects are unavailable; build the project first")
    subprocess.run([
        "c++", "-O3", "-DNDEBUG", *objects, str(obj), str(build_dir / "libfathom.a"),
        "-lpthread", "-o", str(binary),
    ], check=True)
    return binary


def stochastic_round(value: float, rng: random.Random) -> int:
    low = math.floor(value)
    return low + int(rng.random() < value - low)


class ClassicSPSA:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.repo = Path(args.repo_root).resolve()
        self.build_dir = (self.repo / args.build_dir).resolve()
        self.state_dir = (self.repo / args.state_dir).resolve()
        self.manifest_path = (self.repo / args.manifest).resolve()
        self.state_path = self.state_dir / "state.json"
        self.params = read_manifest(self.manifest_path)
        self.names = [row["name"] for row in self.params]
        self.games_per_batch = args.games_per_batch
        if self.games_per_batch <= 0 or self.games_per_batch % 2:
            raise ValueError("--games-per-batch must be a positive even number")
        self.pairs_per_batch = self.games_per_batch // 2
        self.num_iter = args.batches * self.pairs_per_batch
        self.A = int(args.A_ratio * self.num_iter)
        if self.state_path.exists():
            self.state = json.loads(self.state_path.read_text(encoding="utf-8"))
            self._validate_resume()
        else:
            theta = {row["name"]: float(row["current_value"]) for row in self.params}
            self.state = {
                "algorithm": "fishtest-classic", "version": 1, "batch": 0,
                "iter": 0, "num_iter": self.num_iter, "A": self.A,
                "alpha": args.alpha, "gamma": args.gamma, "seed": args.seed,
                "games_per_batch": self.games_per_batch, "theta": theta,
                "wdl": {"wins": 0, "draws": 0, "losses": 0},
                "pentanomial": [0, 0, 0, 0, 0], "history": [],
                "parameter_names": self.names,
            }
            self._save()

    def _validate_resume(self) -> None:
        expected = {
            "algorithm": "fishtest-classic", "num_iter": self.num_iter, "A": self.A,
            "alpha": self.args.alpha, "gamma": self.args.gamma, "seed": self.args.seed,
            "games_per_batch": self.games_per_batch, "parameter_names": self.names,
        }
        for key, value in expected.items():
            if self.state.get(key) != value:
                raise RuntimeError(f"resume state mismatch for {key}: {self.state.get(key)!r} != {value!r}")
        if set(self.state.get("theta", {})) != set(self.names):
            raise RuntimeError("resume state parameter names do not match the manifest")

    def _save(self) -> None:
        atomic_json(self.state_path, self.state)
        atomic_json(self.state_dir / "final-parameters.json", {
            name: int(round(self.state["theta"][name])) for name in self.names
        })

    def _parameter_step(self, row: dict[str, str], iter_value: int) -> tuple[float, float]:
        c_end = float(row["natural_scale"])
        r_end = self.args.r_end
        c_base = c_end * self.num_iter ** self.args.gamma
        a_end = r_end * c_end ** 2
        a_base = a_end * (self.A + self.num_iter) ** self.args.alpha
        local = iter_value + 1
        c = c_base / local ** self.args.gamma
        R = a_base / (self.A + local) ** self.args.alpha / c ** 2
        return c, R

    def perturb(self, batch: int) -> tuple[dict[str, int], dict[str, int], list[dict[str, Any]]]:
        rng = random.Random(self.args.seed + batch * 1_000_003)
        plus: dict[str, int] = {}
        minus: dict[str, int] = {}
        steps: list[dict[str, Any]] = []
        for row in self.params:
            name = row["name"]
            theta = float(self.state["theta"][name])
            low, high = float(row["minimum"]), float(row["maximum"])
            flip = rng.choice((-1, 1))
            c, R = self._parameter_step(row, int(self.state["iter"]))
            plus_real = min(max(theta + c * flip, low), high)
            minus_real = min(max(theta - c * flip, low), high)
            plus[name] = stochastic_round(plus_real, rng)
            minus[name] = stochastic_round(minus_real, rng)
            steps.append({"name": name, "flip": flip, "c": c, "R": R,
                          "plus": plus[name], "minus": minus[name]})
        return plus, minus, steps

    def tasks(self, batch: int, plus: Path, minus: Path) -> list[SingleGameTask]:
        tasks = []
        options = {"SyzygyPath": self.args.syzygy_path,
                   "SyzygyProbeLimit": self.args.syzygy_limit}
        for pair in range(self.pairs_per_batch):
            opening_index = (batch * self.pairs_per_batch + pair) % len(STARTING_POSITIONS)
            name, fen = STARTING_POSITIONS[opening_index]
            for color in range(2):
                game = pair * 2 + color
                tasks.append(SingleGameTask(
                    game_index=game, pos_name=name, starting_fen=fen,
                    white_role="plus" if color == 0 else "minus",
                    black_role="minus" if color == 0 else "plus",
                    plus_engine_path=str(plus), minus_engine_path=str(minus),
                    base_time_sec=self.args.base_time, inc_sec=self.args.inc,
                    cpu_core=game % self.args.concurrency, uci_options=options,
                    nodes_per_move=None if self.args.wall_clock else self.args.nodes_per_move,
                ))
        return tasks

    @staticmethod
    def summarize(results: list[SingleGameOutput]) -> tuple[dict[str, int], list[int]]:
        wins = sum(result.winner == "plus" for result in results)
        losses = sum(result.winner == "minus" for result in results)
        draws = len(results) - wins - losses
        pentanomial = [0, 0, 0, 0, 0]
        for index in range(0, len(results), 2):
            score = results[index].plus_score + results[index + 1].plus_score
            pentanomial[int(round(score * 2))] += 1
        return {"wins": wins, "draws": draws, "losses": losses}, pentanomial

    def apply(self, steps: list[dict[str, Any]], wdl: dict[str, int]) -> None:
        result = wdl["wins"] - wdl["losses"]
        by_name = {row["name"]: row for row in self.params}
        for step in steps:
            name = step["name"]
            row = by_name[name]
            value = self.state["theta"][name] + step["R"] * step["c"] * result * step["flip"]
            self.state["theta"][name] = min(max(value, float(row["minimum"])), float(row["maximum"]))
        self.state["iter"] += self.pairs_per_batch

    def run_batch(self) -> None:
        batch = int(self.state["batch"])
        plus_vector, minus_vector, steps = self.perturb(batch)
        plus = build_variant(self.repo, self.build_dir, self.state_dir, plus_vector, "plus")
        minus = build_variant(self.repo, self.build_dir, self.state_dir, minus_vector, "minus")
        tasks = self.tasks(batch, plus, minus)
        results: list[SingleGameOutput | None] = [None] * len(tasks)
        with ProcessPoolExecutor(max_workers=self.args.concurrency) as pool:
            futures = {pool.submit(_run_single_game_worker, task): task.game_index for task in tasks}
            for future in as_completed(futures):
                results[futures[future]] = future.result()
        completed = [result for result in results if result is not None]
        if len(completed) != len(tasks) or any(not result.is_valid for result in completed):
            raise RuntimeError("one or more paired games failed; SPSA state was not updated")
        for pair in range(self.pairs_per_batch):
            first, second = completed[2 * pair:2 * pair + 2]
            if (first.starting_fen != second.starting_fen or first.white_role != "plus" or
                    second.white_role != "minus"):
                raise RuntimeError("paired opening/color invariant failed")
        wdl, pentanomial = self.summarize(completed)
        before = dict(self.state["theta"])
        self.apply(steps, wdl)
        for key in wdl:
            self.state["wdl"][key] += wdl[key]
        for index, count in enumerate(pentanomial):
            self.state["pentanomial"][index] += count
        record = {
            "batch": batch, "iter_before": self.state["iter"] - self.pairs_per_batch,
            "iter_after": self.state["iter"], "plus_vector_key": vector_key(plus_vector),
            "minus_vector_key": vector_key(minus_vector), "wdl": wdl,
            "pentanomial": pentanomial, "result": wdl["wins"] - wdl["losses"],
            "theta_before": before, "theta_after": self.state["theta"], "steps": steps,
            "games": [{"opening": r.pos_name, "fen": r.starting_fen,
                       "white": r.white_role, "black": r.black_role, "result": r.result}
                      for r in completed],
        }
        atomic_json(self.state_dir / "batches" / f"batch-{batch:06d}.json", record)
        (self.state_dir / "pgn").mkdir(parents=True, exist_ok=True)
        (self.state_dir / "pgn" / f"batch-{batch:06d}.pgn").write_text(
            "\n\n".join(result.pgn_str for result in completed) + "\n", encoding="utf-8")
        self.state["history"].append({key: record[key] for key in
                                      ("batch", "iter_before", "iter_after", "wdl", "pentanomial", "result")})
        self.state["batch"] += 1
        self._save()
        print(f"batch {batch + 1}/{self.args.batches}: WDL {wdl['wins']}/{wdl['draws']}/{wdl['losses']} "
              f"pentanomial {pentanomial} iter {self.state['iter']}/{self.num_iter}")

    def run(self) -> None:
        while self.state["batch"] < self.args.batches:
            self.run_batch()

    def export_binary(self, destination: Path) -> None:
        vector = {name: int(round(self.state["theta"][name])) for name in self.names}
        source = build_variant(self.repo, self.build_dir, self.state_dir, vector, "final")
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Howl Fishtest-classic paired self-play SPSA")
    parser.add_argument("--batches", type=int, default=1000)
    parser.add_argument("--games-per-batch", type=int, default=8)
    parser.add_argument("--base-time", type=float, default=120.0)
    parser.add_argument("--inc", type=float, default=1.0)
    parser.add_argument("--nodes-per-move", type=int, default=1000)
    parser.add_argument("--wall-clock", action="store_true",
                        help="Use the legacy clock control instead of local fixed-node search")
    parser.add_argument("--concurrency", type=int, default=8)
    parser.add_argument("--A-ratio", type=float, default=0.1)
    parser.add_argument("--alpha", type=float, default=0.602)
    parser.add_argument("--gamma", type=float, default=0.101)
    parser.add_argument("--r-end", type=float, default=0.002)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--state-dir", default="fishtest-spsa-state")
    parser.add_argument("--manifest", default="move-quality-tuning/parameter-manifest.tsv")
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--build-dir", default="build")
    parser.add_argument("--syzygy-path", default="/home/masoud/syzygy/3-4-5-wdl")
    parser.add_argument("--syzygy-limit", type=int, default=5)
    parser.add_argument("--export-binary")
    parser.add_argument("--smoke-test", action="store_true")
    args = parser.parse_args()
    if args.smoke_test:
        args.batches = 1
        args.games_per_batch = 2
        args.base_time = 0.05
        args.inc = 0.01
        args.concurrency = min(args.concurrency, 2)
    return args


def main() -> int:
    args = parse_args()
    tuner = ClassicSPSA(args)
    if args.export_binary:
        tuner.export_binary(Path(args.export_binary).resolve())
        return 0
    before = json.loads(json.dumps(tuner.state, sort_keys=True))
    tuner.run()
    if args.smoke_test:
        resumed = ClassicSPSA(args)
        if json.dumps(resumed.state, sort_keys=True) != json.dumps(tuner.state, sort_keys=True):
            raise RuntimeError("resume state is not exact")
        if tuner.state["batch"] != before["batch"] + 1:
            raise RuntimeError("smoke test did not apply exactly one SPSA update")
        print("smoke test: perturbation, pairing, update, and exact resume passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
