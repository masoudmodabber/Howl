#!/usr/bin/env python3
"""
Disposable Azure Cloud Self-Match Runner for Howl.
Runs old vs new Howl games in parallel using Azure Container Instances (ACI),
collects game results and combined PGN, and cleanly deletes all temporary cloud resources.
"""

from __future__ import annotations

import argparse
import base64
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
import uuid
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Tuple

# Ensure tools/ and repository root are on path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from tools.self_match import (
    EngineConfig,
    GameResult,
    MatchConfig,
    play_single_game,
    print_summary,
)

# 10 diverse starting opening positions played with both colours (20 games default)
DETERMINISTIC_OPENINGS: List[Tuple[str, str]] = [
    ("Italian Game", "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3"),
    ("Sicilian Defense Open", "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq - 1 5"),
    ("Queen's Gambit Declined", "rnbqkb1r/ppp2ppp/4pn2/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 2 4"),
    ("French Defense Advance", "rnbqkbnr/ppp2ppp/4p3/3pP3/3P4/8/PPP2PPP/RNBQKBNR b KQkq - 0 3"),
    ("Ruy Lopez", "r1bqkbnr/1ppp1ppp/p1n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 4"),
    ("Caro-Kann Classical", "rnbqkbnr/pp2pppp/2p5/3p4/3PP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3"),
    ("King's Indian Defense", "rnbqkb1r/pppppp1p/5np1/8/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3"),
    ("Nimzo-Indian Defense", "rnbqk2r/pppp1ppp/4pn2/8/1bPP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 2 4"),
    ("English Opening", "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq - 0 1"),
    ("Scandinavian Defense", "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"),
]

# Fixed container compute specifications per game instance
CONTAINER_CPU = 1
CONTAINER_MEMORY_GB = 2
DEFAULT_PROVISIONING_TIMEOUT_SEC = 600.0  # 10 minutes


@dataclass
class CloudTaskSpec:
    game_index: int
    total_games: int
    pos_name: str
    starting_fen: str
    white_role: str  # "old" or "new"
    black_role: str  # "new" or "old"
    movetime_sec: float
    inc_sec: float


# Candidate Azure regions suitable for ACI container execution
CANDIDATE_REGIONS: List[str] = [
    "eastus",
    "westus2",
    "centralus",
    "eastus2",
    "westeurope",
]


@dataclass
class RegionalQuota:
    location: str
    limit: int
    current_usage: int
    available_cores: int
    available_capacity: int  # games = available_cores // CONTAINER_CPU


def query_multi_region_quotas(
    candidate_regions: Optional[List[str]] = None,
) -> Dict[str, RegionalQuota]:
    """Queries Microsoft.ContainerInstance standardCores quota and usage for all candidate regions."""
    regions = candidate_regions or CANDIDATE_REGIONS
    quotas: Dict[str, RegionalQuota] = {}

    sub_id = None
    try:
        sub_res = subprocess.run(
            ["az", "account", "show", "--query", "id", "-o", "tsv"],
            capture_output=True,
            text=True,
        )
        if sub_res.returncode == 0 and sub_res.stdout.strip():
            sub_id = sub_res.stdout.strip()
    except Exception:
        pass

    for loc in regions:
        limit, curr, avail = 10, 0, 10
        if sub_id:
            try:
                url = f"/subscriptions/{sub_id}/providers/Microsoft.ContainerInstance/locations/{loc}/usages?api-version=2023-05-01"
                usage_res = subprocess.run(
                    ["az", "rest", "--method", "get", "--url", url, "-o", "json"],
                    capture_output=True,
                    text=True,
                )
                if usage_res.returncode == 0 and usage_res.stdout.strip():
                    data = json.loads(usage_res.stdout)
                    for item in data.get("value", []):
                        name_val = item.get("name", {}).get("value", "")
                        if name_val.lower() == "standardcores":
                            limit = int(item.get("limit", 10))
                            curr = int(item.get("currentValue", 0))
                            avail = max(0, limit - curr)
                            break
            except Exception:
                pass
        cap = avail // CONTAINER_CPU
        quotas[loc] = RegionalQuota(
            location=loc,
            limit=limit,
            current_usage=curr,
            available_cores=avail,
            available_capacity=cap,
        )

    return quotas


def allocate_tasks_to_regions(
    tasks: List[CloudTaskSpec],
    regional_quotas: Dict[str, RegionalQuota],
) -> Tuple[List[List[Tuple[CloudTaskSpec, str]]], int]:
    """
    Allocates tasks across regions concurrently up to aggregate simultaneous capacity.
    Returns (waves_of_task_region_pairs, aggregate_simultaneous_capacity).
    """
    # Order regions matching candidate order with capacity > 0
    active_regions = [
        (loc, q.available_capacity)
        for loc, q in regional_quotas.items()
        if q.available_capacity > 0
    ]
    aggregate_capacity = sum(cap for _, cap in active_regions)

    if aggregate_capacity <= 0:
        # Fallback to 1 game at a time in first region if all reported 0
        first_loc = list(regional_quotas.keys())[0] if regional_quotas else "eastus"
        waves = [[(t, first_loc)] for t in tasks]
        return waves, 0

    waves: List[List[Tuple[CloudTaskSpec, str]]] = []
    task_idx = 0
    total_tasks = len(tasks)

    while task_idx < total_tasks:
        current_wave: List[Tuple[CloudTaskSpec, str]] = []
        for loc, cap in active_regions:
            for _ in range(cap):
                if task_idx < total_tasks:
                    current_wave.append((tasks[task_idx], loc))
                    task_idx += 1
                else:
                    break
            if task_idx >= total_tasks:
                break
        waves.append(current_wave)

    return waves, aggregate_capacity


def query_regional_cpu_quota(location: str) -> Tuple[int, int, int]:
    """Backward compatibility helper for single region quota query."""
    res = query_multi_region_quotas([location])
    q = res.get(location, RegionalQuota(location, 10, 0, 10, 10))
    return q.limit, q.current_usage, q.available_cores


def calculate_waves(total_tasks: int, max_simultaneous: int) -> List[List[int]]:
    """Partitions task indices into sequential wave batches."""
    batch_size = max(1, max_simultaneous)
    return [list(range(i, min(i + batch_size, total_tasks))) for i in range(0, total_tasks, batch_size)]


def generate_task_specs(
    total_games: int,
    movetime_sec: float,
    inc_sec: float,
) -> List[CloudTaskSpec]:
    """
    Deterministically assigns openings and colours for an even number of games.
    Each opening is played exactly twice with colours swapped.
    """
    if total_games <= 0 or total_games % 2 != 0:
        raise ValueError(f"Requested game count ({total_games}) must be a positive even integer.")

    tasks: List[CloudTaskSpec] = []
    num_openings = len(DETERMINISTIC_OPENINGS)
    game_idx = 1

    for pair_idx in range(total_games // 2):
        pos_name, fen = DETERMINISTIC_OPENINGS[pair_idx % num_openings]

        # Game 1 of pair: old White vs new Black
        tasks.append(
            CloudTaskSpec(
                game_index=game_idx,
                total_games=total_games,
                pos_name=pos_name,
                starting_fen=fen,
                white_role="old",
                black_role="new",
                movetime_sec=movetime_sec,
                inc_sec=inc_sec,
            )
        )
        game_idx += 1

        # Game 2 of pair: new White vs old Black (colours reversed)
        tasks.append(
            CloudTaskSpec(
                game_index=game_idx,
                total_games=total_games,
                pos_name=pos_name,
                starting_fen=fen,
                white_role="new",
                black_role="old",
                movetime_sec=movetime_sec,
                inc_sec=inc_sec,
            )
        )
        game_idx += 1

    return tasks


def run_worker_game(task: CloudTaskSpec, old_engine_path: str, new_engine_path: str) -> GameResult:
    """
    Executes a single game worker run (used inside Docker / ACI or local simulation).
    """
    return play_single_game(
        game_idx=task.game_index,
        pos_name=task.pos_name,
        starting_fen=task.starting_fen,
        white_role=task.white_role,
        black_role=task.black_role,
        old_engine_path=old_engine_path,
        new_engine_path=new_engine_path,
        base_time_sec=task.movetime_sec,
        inc_sec=task.inc_sec,
    )


class AzureCloudRunner:
    def __init__(
        self,
        old_path: str,
        new_path: str,
        movetime_sec: float,
        inc_sec: float,
        total_games: int,
        pgn_output: Optional[str] = None,
        location: str = "eastus",
        candidate_regions: Optional[List[str]] = None,
        dry_run: bool = False,
    ):
        self.old_path = os.path.abspath(old_path)
        self.new_path = os.path.abspath(new_path)
        self.movetime_sec = movetime_sec
        self.inc_sec = inc_sec
        self.total_games = total_games
        self.pgn_output = pgn_output or time.strftime("self-match-%Y-%m-%d-%H%M%S.pgn")
        self.pgn_output_is_default = pgn_output is None
        self.location = location
        self.candidate_regions = candidate_regions or CANDIDATE_REGIONS
        self.dry_run = dry_run

        match_id = uuid.uuid4().hex[:8]
        self.rg_name = f"howl-match-rg-{match_id}"
        self.acr_name = f"howlcr{match_id}"
        self.image_name = "howl-match-runner:latest"
        self.tasks = generate_task_specs(total_games, movetime_sec, inc_sec)
        self.resource_group_created = False
        self.resource_group_cleaned = False

    def validate_azure_login(self) -> None:
        """Verifies active Azure CLI login session and account."""
        if self.dry_run:
            return
        res = subprocess.run(["az", "account", "show"], capture_output=True, text=True)
        if res.returncode != 0:
            raise RuntimeError(
                "Azure CLI login validation failed. Please login with 'az login' before running cloud matches."
            )

    def print_dry_run_plan(self) -> None:
        quotas = query_multi_region_quotas(self.candidate_regions)
        waves, total_simultaneous = allocate_tasks_to_regions(self.tasks, quotas)

        # Region assignment distribution for first wave / total
        region_counts: Dict[str, int] = {}
        for wave in waves:
            for _, loc in wave:
                region_counts[loc] = region_counts.get(loc, 0) + 1

        print("\n" + "=" * 60)
        print("AZURE CLOUD SELF-MATCH DRY-RUN PLAN")
        print("=" * 60)
        print(f"Temporary Resource Group:   {self.rg_name} ({self.location})")
        print(f"Temporary Container Reg:    {self.acr_name}.azurecr.io")
        print(f"Docker Image:               {self.image_name}")
        print(f"Requested Games:            {self.total_games}")
        print(f"Cores per Game:             {CONTAINER_CPU} vCPU ({CONTAINER_MEMORY_GB} GiB RAM)")
        print(f"Candidate Regions:          {', '.join(self.candidate_regions)}")
        print("-" * 60)
        print("REGIONAL CAPACITY & ALLOCATION:")
        print("-" * 60)
        for loc in self.candidate_regions:
            q = quotas.get(loc, RegionalQuota(loc, 10, 0, 10, 10))
            assigned = region_counts.get(loc, 0)
            print(f"  {loc:<12} | Limit: {q.limit:>2} | Used: {q.current_usage:>2} | Avail Cores: {q.available_cores:>2} | Capacity: {q.available_capacity:>2} | Assigned: {assigned:>2}")
        print("-" * 60)
        print(f"Total Simultaneous Capacity: {total_simultaneous} games")
        print(f"Number of Required Waves:    {len(waves)}")
        print(f"Time Control:                {self.movetime_sec}s + {self.inc_sec}s")
        print(f"Combined PGN Output:         {self.pgn_output}")
        print("-" * 60)
        print("DETERMINISTIC GAME PAIRINGS & OPENING ASSIGNMENTS:")
        print("-" * 60)
        for t in self.tasks:
            w_eng = "Howl-old" if t.white_role == "old" else "Howl-new"
            b_eng = "Howl-new" if t.black_role == "new" else "Howl-old"
            print(f"  Game {t.game_index:02d}/{t.total_games:02d}: {t.pos_name:<26} | White: {w_eng:<9} Black: {b_eng:<9}")
        print("=" * 60 + "\n")

    def run(self) -> List[GameResult]:
        self.validate_azure_login()

        if self.dry_run:
            self.print_dry_run_plan()
            return []

        self._initialize_pgn_output()

        # Setup explicit signal handlers for SIGINT and SIGTERM
        import signal

        interrupted = False

        def _handle_signal(signum, frame):
            nonlocal interrupted
            interrupted = True
            sig_name = "SIGINT (Ctrl+C)" if signum == signal.SIGINT else f"signal {signum}"
            print(f"\nReceived {sig_name}. Stopping cloud match and cleaning up resources...", file=sys.stderr)
            self.cleanup_resources()
            sys.exit(130 if signum == signal.SIGINT else 143)

        old_sigint = signal.signal(signal.SIGINT, _handle_signal)
        old_sigterm = signal.signal(signal.SIGTERM, _handle_signal)

        results: List[GameResult] = []
        try:
            results = self._execute_cloud_match()
        finally:
            signal.signal(signal.SIGINT, old_sigint)
            signal.signal(signal.SIGTERM, old_sigterm)
            if not interrupted:
                self.cleanup_resources()

        return results

    def _initialize_pgn_output(self) -> None:
        mode = "x" if self.pgn_output_is_default else "w"
        with open(self.pgn_output, mode, encoding="utf-8"):
            pass

    def _append_completed_game(self, result: GameResult) -> None:
        if result.result not in ("1-0", "0-1", "1/2-1/2") or not result.pgn_str:
            return
        with open(self.pgn_output, "a", encoding="utf-8") as pgn_file:
            pgn_file.write(result.pgn_str + "\n\n")

    def build_container_create_cmd(
        self,
        container_group_name: str,
        image_uri: str,
        registry_server: str,
        registry_user: str,
        registry_pass: str,
        command_args: List[str],
        location: Optional[str] = None,
    ) -> List[str]:
        """Builds the exact az container create command line with restart policy Never and Linux OS."""
        cmd = [
            "az", "container", "create",
            "--resource-group", self.rg_name,
            "--name", container_group_name,
            "--image", image_uri,
            "--os-type", "Linux",
            "--cpu", str(CONTAINER_CPU),
            "--memory", str(CONTAINER_MEMORY_GB),
            "--restart-policy", "Never",
            "--registry-login-server", registry_server,
            "--registry-username", registry_user,
            "--registry-password", registry_pass,
            "--command-line", " ".join(command_args),
        ]
        if location:
            cmd.extend(["--location", location])
        return cmd

    def _execute_cloud_match(self) -> List[GameResult]:
        # Query quotas across all candidate regions before creating anything
        quotas = query_multi_region_quotas(self.candidate_regions)
        waves, total_simultaneous = allocate_tasks_to_regions(self.tasks, quotas)

        # Region assignment distribution for all waves
        region_counts: Dict[str, int] = {}
        for wave in waves:
            for _, loc in wave:
                region_counts[loc] = region_counts.get(loc, 0) + 1

        print("\n" + "=" * 60)
        print("AZURE CLOUD EXECUTION PLAN")
        print("=" * 60)
        print(f"Requested Games:            {self.total_games}")
        print(f"Cores per Game:             {CONTAINER_CPU} vCPU ({CONTAINER_MEMORY_GB} GiB RAM)")
        print(f"Candidate Regions:          {', '.join(self.candidate_regions)}")
        print("-" * 60)
        print("REGIONAL CAPACITY & ALLOCATION:")
        print("-" * 60)
        for loc in self.candidate_regions:
            q = quotas.get(loc, RegionalQuota(loc, 10, 0, 10, 10))
            assigned = region_counts.get(loc, 0)
            print(f"  {loc:<12} | Limit: {q.limit:>2} | Used: {q.current_usage:>2} | Avail Cores: {q.available_cores:>2} | Capacity: {q.available_capacity:>2} | Assigned: {assigned:>2}")
        print("-" * 60)
        print(f"Total Simultaneous Capacity: {total_simultaneous} games")
        print(f"Number of Required Waves:    {len(waves)}")
        print("=" * 60 + "\n")

        print(f"Creating temporary resource group '{self.rg_name}' in {self.location}...")
        res = subprocess.run(
            ["az", "group", "create", "--name", self.rg_name, "--location", self.location],
            capture_output=True,
            text=True,
        )
        if res.returncode != 0:
            raise RuntimeError(f"Failed to create resource group '{self.rg_name}':\n{res.stderr}")
        self.resource_group_created = True

        print(f"Creating temporary Azure Container Registry '{self.acr_name}'...")
        res = subprocess.run(
            ["az", "acr", "create", "--resource-group", self.rg_name, "--name", self.acr_name, "--sku", "Basic", "--admin-enabled", "true"],
            capture_output=True,
            text=True,
        )
        if res.returncode != 0:
            raise RuntimeError(f"Failed to create ACR '{self.acr_name}':\n{res.stderr}")

        # Get ACR credentials
        res = subprocess.run(
            ["az", "acr", "credential", "show", "--name", self.acr_name],
            capture_output=True,
            text=True,
        )
        if res.returncode != 0:
            raise RuntimeError(f"Failed to retrieve credentials for ACR '{self.acr_name}':\n{res.stderr}")
        cred_info = json.loads(res.stdout)
        login_server = f"{self.acr_name}.azurecr.io"
        acr_user = cred_info["username"]
        acr_pass = cred_info["passwords"][0]["value"]
        full_image_uri = f"{login_server}/{self.image_name}"

        # Prepare Docker context with binaries and worker script
        with tempfile.TemporaryDirectory() as build_ctx:
            shutil.copy2(self.old_path, os.path.join(build_ctx, "howl-old"))
            shutil.copy2(self.new_path, os.path.join(build_ctx, "howl-new"))
            os.chmod(os.path.join(build_ctx, "howl-old"), 0o755)
            os.chmod(os.path.join(build_ctx, "howl-new"), 0o755)

            # Copy self_match.py and cloud_self_match.py as worker module
            tools_dir = os.path.join(build_ctx, "tools")
            os.makedirs(tools_dir, exist_ok=True)
            shutil.copy2(os.path.abspath(__file__), os.path.join(tools_dir, "cloud_self_match.py"))
            self_match_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "self_match.py")
            shutil.copy2(self_match_path, os.path.join(tools_dir, "self_match.py"))

            dockerfile_content = """FROM python:3.11-slim
WORKDIR /app
RUN pip install --no-cache-dir chess
COPY howl-old /app/howl-old
COPY howl-new /app/howl-new
COPY tools /app/tools
ENV PYTHONPATH=/app
ENTRYPOINT ["python", "-u", "/app/tools/cloud_self_match.py"]
"""
            with open(os.path.join(build_ctx, "Dockerfile"), "w") as f:
                f.write(dockerfile_content)

            print(f"Building and pushing image to '{full_image_uri}' via ACR build...")
            res = subprocess.run(
                ["az", "acr", "build", "--registry", self.acr_name, "--image", self.image_name, build_ctx],
                capture_output=True,
                text=True,
            )
            if res.returncode != 0:
                raise RuntimeError(f"ACR build failed for image '{full_image_uri}':\n{res.stderr}")

        # Execute games wave by wave
        results: List[GameResult] = []
        poll_interval = 5.0

        for wave_idx, wave_items in enumerate(waves, 1):
            print(f"\n--- Launching Wave {wave_idx}/{len(waves)} ({len(wave_items)} game instances across regions) ---")

            launch_processes = []
            for task, task_location in wave_items:
                cg_name = f"howl-game-{task.game_index:02d}"
                task_json = json.dumps({
                    "game_index": task.game_index,
                    "total_games": task.total_games,
                    "pos_name": task.pos_name,
                    "starting_fen": task.starting_fen,
                    "white_role": task.white_role,
                    "black_role": task.black_role,
                    "movetime_sec": task.movetime_sec,
                    "inc_sec": task.inc_sec,
                    "old_engine": "/app/howl-old",
                    "new_engine": "/app/howl-new",
                })
                b64_payload = base64.b64encode(task_json.encode("utf-8")).decode("utf-8")
                cmd_args = ["python", "-u", "/app/tools/cloud_self_match.py", "--worker-payload", b64_payload]
                create_cmd = self.build_container_create_cmd(
                    container_group_name=cg_name,
                    image_uri=full_image_uri,
                    registry_server=login_server,
                    registry_user=acr_user,
                    registry_pass=acr_pass,
                    command_args=cmd_args,
                    location=task_location,
                )
                create_cmd.append("--no-wait")
                p = subprocess.Popen(create_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
                launch_processes.append((task, task_location, cg_name, p))

            # Check launch dispatches for this wave
            pending_wave_tasks: Dict[int, Tuple[CloudTaskSpec, str, str, float]] = {}
            wave_launch_time = time.time()
            failed_dispatches = 0

            for task, task_location, cg_name, p in launch_processes:
                out, err = p.communicate()
                if p.returncode != 0:
                    failed_dispatches += 1
                    print(f"Error: launch dispatch failed for {cg_name} in {task_location} (code {p.returncode}): {err.strip()}", file=sys.stderr)
                    white_name = "Howl-old" if task.white_role == "old" else "Howl-new"
                    black_name = "Howl-new" if task.black_role == "new" else "Howl-old"
                    failed_res = GameResult(
                        game_index=task.game_index,
                        pos_name=task.pos_name,
                        starting_fen=task.starting_fen,
                        white_engine=white_name,
                        black_engine=black_name,
                        result="*",
                        winner="failed",
                        termination=f"Launch dispatch failed in {task_location}: {err.strip()}",
                        pgn_str="",
                    )
                    results.append(failed_res)
                    self._append_completed_game(failed_res)
                else:
                    pending_wave_tasks[task.game_index] = (task, task_location, cg_name, wave_launch_time)

            if not pending_wave_tasks and len(waves) == 1:
                raise RuntimeError(f"All {len(wave_items)} container launch dispatches failed in Wave {wave_idx}.")

            if failed_dispatches > 0:
                print(f"Dispatched {len(pending_wave_tasks)}/{len(wave_items)} game instances in Wave {wave_idx} ({failed_dispatches} failed). Monitoring progress...")
            else:
                print(f"All {len(pending_wave_tasks)} game instances dispatched in Wave {wave_idx}. Monitoring progress...")

            # Monitor this wave to completion before moving to next wave
            # Track provisioned state per task: provisioned_tasks set contains indices of tasks that reached Running or Succeeded
            provisioned_tasks: set = set()

            while pending_wave_tasks:
                time.sleep(poll_interval)
                finished_indices = []

                for idx, (task, task_location, cg_name, start_t) in pending_wave_tasks.items():
                    show_res = subprocess.run(
                        [
                            "az", "container", "show",
                            "--resource-group", self.rg_name,
                            "--name", cg_name,
                            "--query", "{provState:provisioningState,state:containers[0].instanceView.currentState.state,exitCode:containers[0].instanceView.currentState.exitCode,detailStatus:containers[0].instanceView.currentState.detailStatus,events:containers[0].instanceView.events}",
                            "-o", "json",
                        ],
                        capture_output=True,
                        text=True,
                    )
                    if show_res.returncode != 0:
                        if "ContainerGroupDeploymentNotReady" in show_res.stderr or "ResourceNotFound" in show_res.stderr:
                            if idx not in provisioned_tasks and (time.time() - start_t > DEFAULT_PROVISIONING_TIMEOUT_SEC):
                                diag = f"Provisioning timeout exceeded ({DEFAULT_PROVISIONING_TIMEOUT_SEC}s) in {task_location}"
                                game_res = self._parse_worker_output(task, "", diag)
                                results.append(game_res)
                                self._append_completed_game(game_res)
                                finished_indices.append(idx)
                                subprocess.run(["az", "container", "delete", "--resource-group", self.rg_name, "--name", cg_name, "--yes", "--no-wait"], capture_output=True)
                            continue
                        continue

                    try:
                        state_info = json.loads(show_res.stdout)
                        prov_state = state_info.get("provState")
                        state = state_info.get("state")
                        exit_code = state_info.get("exitCode")
                        detail_status = state_info.get("detailStatus")
                        events = state_info.get("events") or []
                    except Exception:
                        continue

                    # Mark provisioned once currentState is Running or provisioningState Succeeded with active container
                    if state == "Running" or (prov_state == "Succeeded" and state != "Waiting"):
                        provisioned_tasks.add(idx)

                    # 1. Terminal provisioning failure (only if never provisioned)
                    if prov_state == "Failed" and idx not in provisioned_tasks:
                        diag = f"Container provisioningState Failed (detail: {detail_status})"
                        game_res = self._parse_worker_output(task, "", diag)
                        results.append(game_res)
                        self._append_completed_game(game_res)
                        finished_indices.append(idx)
                        subprocess.run(["az", "container", "delete", "--resource-group", self.rg_name, "--name", cg_name, "--yes", "--no-wait"], capture_output=True)
                        print(f"[Game {game_res.game_index:02d}/{self.total_games:02d}] {game_res.pos_name:<24} {game_res.white_engine} vs {game_res.black_engine} -> FAILED (Provisioning Failed)")
                        continue

                    # 2. Check provisioning timeout (only if never provisioned)
                    if idx not in provisioned_tasks and state != "Terminated" and (time.time() - start_t > DEFAULT_PROVISIONING_TIMEOUT_SEC):
                        diag = f"Provisioning timeout exceeded ({DEFAULT_PROVISIONING_TIMEOUT_SEC}s)"
                        game_res = self._parse_worker_output(task, "", diag)
                        results.append(game_res)
                        self._append_completed_game(game_res)
                        finished_indices.append(idx)
                        subprocess.run(["az", "container", "delete", "--resource-group", self.rg_name, "--name", cg_name, "--yes", "--no-wait"], capture_output=True)
                        print(f"[Game {game_res.game_index:02d}/{self.total_games:02d}] {game_res.pos_name:<24} {game_res.white_engine} vs {game_res.black_engine} -> FAILED (Provisioning Timeout)")
                        continue

                    # 3. Check for container startup failure events (before successful running)
                    failed_event_msg = None
                    if idx not in provisioned_tasks:
                        for ev in events:
                            if ev.get("type") == "Warning" and ev.get("name") == "Failed":
                                failed_event_msg = ev.get("message", "Container startup failed")
                                break

                    if failed_event_msg:
                        log_res = subprocess.run(
                            ["az", "container", "logs", "--resource-group", self.rg_name, "--name", cg_name],
                            capture_output=True,
                            text=True,
                        )
                        diag = f"Container startup failure: {failed_event_msg}"
                        if log_res.stdout or log_res.stderr:
                            diag += f"\nStdout: {log_res.stdout}\nStderr: {log_res.stderr}"
                        game_res = self._parse_worker_output(task, log_res.stdout, diag)
                        results.append(game_res)
                        self._append_completed_game(game_res)
                        finished_indices.append(idx)

                        summary_term = game_res.termination.splitlines()[0] if "\n" in game_res.termination else game_res.termination
                        print(
                            f"[Game {game_res.game_index:02d}/{self.total_games:02d}] {game_res.pos_name:<24} "
                            f"{game_res.white_engine} vs {game_res.black_engine} -> FAILED ({summary_term})"
                        )
                    elif state == "Terminated":
                        # Container finished
                        log_res = subprocess.run(
                            ["az", "container", "logs", "--resource-group", self.rg_name, "--name", cg_name],
                            capture_output=True,
                            text=True,
                        )
                        if log_res.returncode != 0 and "ContainerGroupDeploymentNotReady" in log_res.stderr:
                            continue

                        game_res = self._parse_worker_output(task, log_res.stdout, log_res.stderr)
                        results.append(game_res)
                        self._append_completed_game(game_res)
                        finished_indices.append(idx)

                        summary_term = game_res.termination.splitlines()[0] if "\n" in game_res.termination else game_res.termination
                        res_display = game_res.result if game_res.result != "*" else "FAILED"
                        print(
                            f"[Game {game_res.game_index:02d}/{self.total_games:02d}] {game_res.pos_name:<24} "
                            f"{game_res.white_engine} vs {game_res.black_engine} -> {res_display:<7} ({summary_term})"
                        )

                for idx in finished_indices:
                    del pending_wave_tasks[idx]

        results.sort(key=lambda r: r.game_index)

        # The PGN is updated as each valid game completes.
        valid_games = [r for r in results if r.result in ("1-0", "0-1", "1/2-1/2") and r.pgn_str]
        if valid_games:
            print(f"\nPGN ({len(valid_games)} completed games) written to {self.pgn_output}")
        else:
            print(f"\nNo completed games. Empty PGN written to {self.pgn_output}")

        self._print_cloud_summary(results)
        return results

    def _parse_worker_output(self, task: CloudTaskSpec, stdout: str, stderr: str) -> GameResult:
        """Parses machine-readable result payload emitted by worker."""
        marker = "=== GAME_RESULT_JSON ==="
        if marker in stdout:
            try:
                raw_json = stdout.split(marker)[1].strip().split("\n")[0]
                d = json.loads(raw_json)
                return GameResult(
                    game_index=d["game_index"],
                    pos_name=d["pos_name"],
                    starting_fen=d["starting_fen"],
                    white_engine=d["white_engine"],
                    black_engine=d["black_engine"],
                    result=d["result"],
                    winner=d["winner"],
                    termination=d["termination"],
                    pgn_str=d["pgn_str"],
                )
            except Exception as e:
                pass

        # Error result on worker failure, startup failure, or exception traceback
        white_name = "Howl-old" if task.white_role == "old" else "Howl-new"
        black_name = "Howl-new" if task.black_role == "new" else "Howl-old"
        full_diag = stderr.strip() if stderr.strip() else (stdout.strip() if stdout.strip() else "Unknown worker termination")
        termination = f"Worker failure: {full_diag}"
        return GameResult(
            game_index=task.game_index,
            pos_name=task.pos_name,
            starting_fen=task.starting_fen,
            white_engine=white_name,
            black_engine=black_name,
            result="*",
            winner="failed",
            termination=termination,
            pgn_str="",
        )

    def _print_cloud_summary(self, results: List[GameResult]) -> None:
        completed = [r for r in results if r.result in ("1-0", "0-1", "1/2-1/2") and r.winner != "failed"]
        failed = [r for r in results if r.winner == "failed" or r.result == "*"]

        old_wins = sum(1 for r in completed if r.winner == "old")
        new_wins = sum(1 for r in completed if r.winner == "new")
        draws = sum(1 for r in completed if r.winner is None and r.result == "1/2-1/2")

        old_score = old_wins + 0.5 * draws
        new_score = new_wins + 0.5 * draws

        print("\n" + "=" * 60)
        print("AZURE CLOUD MATCH SUMMARY")
        print("=" * 60)
        print(f"Total Games Requested: {len(results)}")
        print(f"Completed Games:       {len(completed)}")
        print(f"Failed Games:          {len(failed)}")
        print("-" * 60)
        print(f"Overall Old Wins:      {old_wins}")
        print(f"Overall New Wins:      {new_wins}")
        print(f"Draws:                 {draws}")
        if completed:
            print(f"Old Score:             {old_score:.1f} / {len(completed)}")
            print(f"New Score:             {new_score:.1f} / {len(completed)}")
        else:
            print("Old Score:             N/A (0 completed games)")
            print("New Score:             N/A (0 completed games)")
        print("-" * 60)
        if failed:
            print("FAILED GAMES / DIAGNOSTICS:")
            print("-" * 60)
            for f_res in failed:
                print(f"  Game {f_res.game_index:02d}: {f_res.pos_name}")
                print(f"    Reason: {f_res.termination}")
            print("-" * 60)

    def cleanup_resources(self) -> bool:
        """Deletes the temporary Azure resource group containing all match resources."""
        if self.dry_run or not self.resource_group_created or self.resource_group_cleaned:
            return True
        self.resource_group_cleaned = True
        print(f"\nDeleting temporary Azure resource group '{self.rg_name}'...")
        res = subprocess.run(
            ["az", "group", "delete", "--name", self.rg_name, "--yes", "--no-wait"],
            capture_output=True,
            text=True,
        )
        if res.returncode != 0:
            if "ResourceGroupNotFound" in res.stderr:
                return True
            print(
                f"WARNING: Automatic deletion of resource group '{self.rg_name}' failed (code {res.returncode}):\n"
                f"{res.stderr}\nPlease delete '{self.rg_name}' manually in the Azure Portal or via 'az group delete'.",
                file=sys.stderr,
            )
            return False
        return True


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Disposable Azure Cloud Self-Match Runner for Howl UCI engine."
    )
    parser.add_argument("old", nargs="?", default=None, help="Path to the 'old' Howl executable")
    parser.add_argument("new", nargs="?", default=None, help="Path to the 'new' Howl executable")
    parser.add_argument(
        "--time",
        type=float,
        default=300.0,
        help="Base time per side in seconds (default: 300.0)",
    )
    parser.add_argument(
        "--inc",
        type=float,
        default=1.5,
        help="Time increment per move in seconds (default: 1.5)",
    )
    parser.add_argument(
        "--games",
        type=int,
        default=20,
        help="Total number of games to play across parallel instances (default: 20, must be even)",
    )
    parser.add_argument(
        "--pgn",
        type=str,
        default=None,
        help="Output file for this run's PGN games (default: self-match-YYYY-MM-DD-HHMMSS.pgn)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print planned cloud resources and game assignments without deploying anything to Azure",
    )
    parser.add_argument(
        "--worker-payload",
        type=str,
        default=None,
        help=argparse.SUPPRESS,
    )

    args = parser.parse_args()

    if args.worker_payload:
        # Running inside container as game worker
        task_data = json.loads(base64.b64decode(args.worker_payload.encode("utf-8")).decode("utf-8"))
        task = CloudTaskSpec(
            game_index=task_data["game_index"],
            total_games=task_data["total_games"],
            pos_name=task_data["pos_name"],
            starting_fen=task_data["starting_fen"],
            white_role=task_data["white_role"],
            black_role=task_data["black_role"],
            movetime_sec=task_data["movetime_sec"],
            inc_sec=task_data["inc_sec"],
        )
        res = run_worker_game(task, task_data["old_engine"], task_data["new_engine"])
        print("\n=== GAME_RESULT_JSON ===")
        print(json.dumps({
            "game_index": res.game_index,
            "pos_name": res.pos_name,
            "starting_fen": res.starting_fen,
            "white_engine": res.white_engine,
            "black_engine": res.black_engine,
            "result": res.result,
            "winner": res.winner,
            "termination": res.termination,
            "pgn_str": res.pgn_str,
        }))
        print("=== END_GAME_RESULT_JSON ===\n")
        return 0

    if not args.old or not args.new:
        parser.error("Both 'old' and 'new' engine paths are required.")

    if args.games <= 0 or args.games % 2 != 0:
        print(f"Error: --games must be a positive even integer (received {args.games}).", file=sys.stderr)
        return 1

    runner = AzureCloudRunner(
        old_path=args.old,
        new_path=args.new,
        movetime_sec=args.time,
        inc_sec=args.inc,
        total_games=args.games,
        pgn_output=args.pgn,
        dry_run=args.dry_run,
    )

    results = runner.run()
    if args.dry_run:
        return 0

    failed_games = [r for r in results if r.winner == "failed" or r.result == "*"]
    if failed_games:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
