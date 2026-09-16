#!/usr/bin/env python3
"""Run Howl's existing SPSA workflow on one persistent Azure VM."""

from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import time
from datetime import datetime, timedelta, timezone
from pathlib import Path
from typing import Any, Dict, List, Optional


DEFAULT_RESOURCE_GROUP = "howl-spsa-rg"
DEFAULT_VM_NAME = "howl-spsa-vm"
DEFAULT_LOCATION = "eastus"
DEFAULT_VM_SIZE = "Standard_F8s_v2"
DEFAULT_RUNTIME_HOURS = 12.0
DEFAULT_ADMIN_USER = "masoud"
DEFAULT_REMOTE_REPO = "/home/masoud/Howl"
DEFAULT_REMOTE_SYZYGY = "/home/masoud/syzygy/3-4-5-wdl"
DEFAULT_METADATA = ".cloud_spsa.json"
TMUX_SESSION = "howl-spsa"
WATCHDOG_SESSION = "howl-spsa-watchdog"


class Runner:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.repo = Path(args.repo_root).resolve()
        self.metadata_path = self.repo / args.metadata
        self.metadata = self._load_metadata()

    def _load_metadata(self) -> Dict[str, Any]:
        if self.metadata_path.is_file():
            with self.metadata_path.open("r", encoding="utf-8") as stream:
                return json.load(stream)
        return {}

    def _save_metadata(self) -> None:
        if self.args.dry_run:
            return
        temporary = self.metadata_path.with_suffix(self.metadata_path.suffix + ".tmp")
        with temporary.open("w", encoding="utf-8") as stream:
            json.dump(self.metadata, stream, indent=2, sort_keys=True)
            stream.write("\n")
        os.replace(temporary, self.metadata_path)

    def _run(
        self,
        command: List[str],
        *,
        capture: bool = False,
        check: bool = True,
    ) -> subprocess.CompletedProcess[str]:
        if self.args.dry_run:
            print("DRY-RUN:", shlex.join(command))
            return subprocess.CompletedProcess(command, 0, "", "")
        return subprocess.run(command, check=check, text=True, capture_output=capture)

    def _az(self, *arguments: str, capture: bool = False, check: bool = True) -> subprocess.CompletedProcess[str]:
        return self._run(["az", *arguments], capture=capture, check=check)

    def _validate_local(self) -> None:
        for executable in ("az", "ssh", "rsync"):
            if shutil.which(executable) is None:
                raise RuntimeError(f"Required executable not found: {executable}")
        if not (self.repo / "tools" / "spsa_tune.py").is_file():
            raise RuntimeError(f"Not a Howl repository: {self.repo}")
        if not self.args.dry_run:
            result = self._az("account", "show", capture=True, check=False)
            if result.returncode != 0:
                raise RuntimeError("Azure CLI login is required; run 'az login'.")

    @property
    def resource_group(self) -> str:
        return self.metadata.get("resource_group", self.args.resource_group)

    @property
    def vm_name(self) -> str:
        return self.metadata.get("vm_name", self.args.vm_name)

    @property
    def admin_user(self) -> str:
        return self.metadata.get("admin_user", self.args.admin_user)

    @property
    def location(self) -> str:
        return self.metadata.get("location", self.args.location)

    def _configure_runtime_guards(self, vm_id: str) -> None:
        watchdog_seconds = max(1, int(self.args.max_runtime_hours * 3600))
        watchdog = (
            f"sleep {watchdog_seconds}; "
            f"if tmux has-session -t {TMUX_SESSION} 2>/dev/null; then "
            f"tmux send-keys -t {TMUX_SESSION} C-c; "
            f"for i in $(seq 1 30); do tmux has-session -t {TMUX_SESSION} 2>/dev/null || break; sleep 1; done; "
            f"tmux has-session -t {TMUX_SESSION} 2>/dev/null && tmux kill-session -t {TMUX_SESSION} || true; fi; "
            "sync; "
            "token=$(curl -fsS -H Metadata:true "
            "'http://169.254.169.254/metadata/identity/oauth2/token?api-version=2018-02-01&resource=https%3A%2F%2Fmanagement.azure.com%2F' "
            "| python3 -c 'import json,sys; print(json.load(sys.stdin)[\"access_token\"])'); "
            f"curl -fsS -X POST -H \"Authorization: Bearer $token\" -H 'Content-Length: 0' "
            f"'https://management.azure.com{vm_id}/deallocate?api-version=2024-11-01'"
        )
        self._ssh(
            f"tmux kill-session -t {WATCHDOG_SESSION} 2>/dev/null || true; "
            f"tmux new-session -d -s {WATCHDOG_SESSION} bash -lc {shlex.quote(watchdog)}"
        )

        shutdown_at = datetime.now(timezone.utc) + timedelta(hours=self.args.max_runtime_hours)
        auto_shutdown = self._az(
            "vm", "auto-shutdown", "--resource-group", self.resource_group,
            "--name", self.vm_name, "--location", self.location,
            "--time", shutdown_at.strftime("%H%M"), check=False,
        )
        if auto_shutdown.returncode != 0:
            print("Warning: Azure auto-shutdown setup failed; the independent VM watchdog remains active.", file=sys.stderr)
        print(f"Runtime watchdog expires at {shutdown_at.isoformat()}.")

    def _vm_exists(self) -> bool:
        if self.args.dry_run:
            return False
        result = self._az(
            "vm", "show", "--resource-group", self.resource_group,
            "--name", self.vm_name, "--query", "id", "-o", "tsv",
            capture=True, check=False,
        )
        return result.returncode == 0 and bool(result.stdout.strip())

    def _public_ip(self) -> str:
        if self.args.dry_run:
            return "DRY_RUN_VM_IP"
        result = self._az(
            "vm", "show", "--show-details", "--resource-group", self.resource_group,
            "--name", self.vm_name, "--query", "publicIps", "-o", "tsv",
            capture=True,
        )
        address = result.stdout.strip()
        if not address:
            raise RuntimeError("Azure VM has no public IP address.")
        return address

    def _ssh_base(self) -> List[str]:
        key_args = ["-i", self.args.ssh_key] if self.args.ssh_key else []
        return [
            "ssh", *key_args,
            "-o", "StrictHostKeyChecking=accept-new",
            "-o", "ServerAliveInterval=30",
            f"{self.admin_user}@{self._public_ip()}",
        ]

    def _ssh(self, remote_command: str, *, tty: bool = False, capture: bool = False,
             check: bool = True) -> subprocess.CompletedProcess[str]:
        command = self._ssh_base()
        if tty:
            command.insert(1, "-t")
        command.extend(["bash", "-lc", shlex.quote(remote_command)])
        return self._run(command, capture=capture, check=check)

    def _wait_for_ssh(self) -> None:
        if self.args.dry_run:
            self._ssh("true")
            return
        deadline = time.monotonic() + 600
        while time.monotonic() < deadline:
            result = self._ssh("true", capture=True, check=False)
            if result.returncode == 0:
                return
            time.sleep(5)
        raise RuntimeError("Timed out waiting for SSH access to the VM.")

    def _rsync(self, source: str, destination: str, extra: Optional[List[str]] = None) -> None:
        key_args = f" -i {shlex.quote(self.args.ssh_key)}" if self.args.ssh_key else ""
        ssh_transport = f"ssh{key_args} -o StrictHostKeyChecking=accept-new"
        command = ["rsync", "-az", "--delete", "-e", ssh_transport]
        if extra:
            command.extend(extra)
        command.extend([source, f"{self.admin_user}@{self._public_ip()}:{destination}"])
        self._run(command)

    def _download_rsync(self, source: str, destination: str) -> None:
        key_args = f" -i {shlex.quote(self.args.ssh_key)}" if self.args.ssh_key else ""
        ssh_transport = f"ssh{key_args} -o StrictHostKeyChecking=accept-new"
        self._run([
            "rsync", "-az", "--delete", "-e", ssh_transport,
            f"{self.admin_user}@{self._public_ip()}:{source}", destination,
        ])

    def launch(self) -> None:
        self._validate_local()
        group_result = self._az(
            "group", "exists", "--name", self.resource_group,
            capture=True, check=False,
        )
        group_existed = group_result.stdout.strip().lower() == "true" if not self.args.dry_run else False
        if not group_existed:
            self._az(
                "group", "create", "--name", self.resource_group,
                "--location", self.args.location, "-o", "none",
            )

        vm_existed = self._vm_exists()
        if not vm_existed:
            ssh_key = self.args.ssh_key + ".pub" if self.args.ssh_key else ""
            create = [
                "vm", "create", "--resource-group", self.resource_group,
                "--name", self.vm_name, "--location", self.args.location,
                "--image", "Ubuntu2204", "--size", self.args.vm_size,
                "--admin-username", self.args.admin_user,
                "--storage-sku", "Premium_LRS", "--os-disk-size-gb", "64",
                "--tags", "managed-by=howl-cloud-spsa", "--output", "none",
            ]
            if ssh_key:
                create.extend(["--ssh-key-values", ssh_key])
            else:
                create.append("--generate-ssh-keys")
            self._az(*create)
        else:
            self._az("vm", "start", "--resource-group", self.resource_group, "--name", self.vm_name)

        identity = self._az(
            "vm", "identity", "assign", "--resource-group", self.resource_group,
            "--name", self.vm_name, "--query", "systemAssignedIdentity", "-o", "tsv",
            capture=True,
        )
        principal_id = identity.stdout.strip() or "DRY_RUN_PRINCIPAL_ID"
        vm_id_result = self._az(
            "vm", "show", "--resource-group", self.resource_group, "--name", self.vm_name,
            "--query", "id", "-o", "tsv", capture=True,
        )
        vm_id = vm_id_result.stdout.strip() or "/DRY_RUN_VM_RESOURCE_ID"
        self._az(
            "role", "assignment", "create", "--assignee-object-id", principal_id,
            "--assignee-principal-type", "ServicePrincipal",
            "--role", "Virtual Machine Contributor", "--scope", vm_id,
            "--output", "none",
        )

        self.metadata.update({
            "resource_group": self.resource_group,
            "resource_group_created": not group_existed,
            "vm_name": self.vm_name,
            "admin_user": self.args.admin_user,
            "location": self.args.location,
            "vm_size": self.args.vm_size,
            "remote_repo": DEFAULT_REMOTE_REPO,
            "started_at": datetime.now(timezone.utc).isoformat(),
            "maximum_runtime_hours": self.args.max_runtime_hours,
        })
        self._save_metadata()
        self._wait_for_ssh()

        running = self._ssh(f"tmux has-session -t {TMUX_SESSION}", capture=True, check=False)
        if running.returncode == 0 and not self.args.dry_run:
            self._ssh("sudo env DEBIAN_FRONTEND=noninteractive apt-get install -y curl")
            self._configure_runtime_guards(vm_id)
            print(f"SPSA session '{TMUX_SESSION}' is already running; runtime guards were refreshed.")
            return

        packages = "git cmake g++ build-essential python3 python3-pip rsync tmux curl"
        self._ssh(
            "sudo env DEBIAN_FRONTEND=noninteractive apt-get update && "
            f"sudo env DEBIAN_FRONTEND=noninteractive apt-get install -y {packages} && "
            "python3 -m pip install --user chess"
        )
        self._ssh(f"mkdir -p {DEFAULT_REMOTE_REPO} {DEFAULT_REMOTE_SYZYGY}")
        self._rsync(
            f"{self.repo}/", f"{DEFAULT_REMOTE_REPO}/",
            ["--exclude", ".git/", "--exclude", "build/", "--exclude", "spsa_state/",
             "--exclude", self.args.metadata],
        )
        local_state = self.repo / "spsa_state"
        if local_state.is_dir():
            self._rsync(f"{local_state}/", f"{DEFAULT_REMOTE_REPO}/spsa_state/")
        local_syzygy = Path(self.args.local_syzygy).expanduser().resolve()
        if local_syzygy.is_dir():
            self._rsync(f"{local_syzygy}/", f"{DEFAULT_REMOTE_SYZYGY}/")
        elif not self.args.dry_run:
            raise RuntimeError(f"Syzygy directory not found: {local_syzygy}")

        self._ssh(
            f"cd {DEFAULT_REMOTE_REPO} && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && "
            "cmake --build build -j8"
        )
        spsa = (
            f"cd {DEFAULT_REMOTE_REPO} && "
            "python3 -u tools/spsa_tune.py --concurrency 8 "
            f"--syzygy-path {DEFAULT_REMOTE_SYZYGY} --state-dir spsa_state "
            "--repo-root . --build-dir build 2>&1 | tee -a spsa_cloud.log"
        )
        self._ssh(f"tmux new-session -d -s {TMUX_SESSION} bash -lc {shlex.quote(spsa)}")

        self._configure_runtime_guards(vm_id)
        print(f"SPSA launched on {self.vm_name}.")

    def status(self) -> None:
        state = self._az(
            "vm", "get-instance-view", "--resource-group", self.resource_group,
            "--name", self.vm_name,
            "--query", "instanceView.statuses[?starts_with(code, 'PowerState/')].displayStatus | [0]",
            "-o", "tsv", capture=True,
        ).stdout.strip()
        normalized_state = state.removeprefix("VM ").lower() or "unknown"
        remote = ""
        if normalized_state == "running" or self.args.dry_run:
            remote = self._ssh(
                f"cd {DEFAULT_REMOTE_REPO} && "
                f"printf '%s\\n' '---SPSA---'; tmux has-session -t {TMUX_SESSION} 2>/dev/null && echo running || echo stopped; "
                "printf '%s\\n' '---CHECKPOINT---' && "
                "test -f spsa_state/spsa_checkpoint.json && cat spsa_state/spsa_checkpoint.json || true; "
                "printf '%s\\n' '---LATEST---' && "
                "grep -E '\\[SPSA [0-9]+/[0-9]+\\].*Plus .*theta dL1=' spsa_cloud.log 2>/dev/null | tail -1 || true",
                capture=True, check=False,
            ).stdout
        spsa_section, _, checkpoint_and_latest = remote.partition("---CHECKPOINT---")
        spsa_state = "running" if "running" in spsa_section else "stopped"
        checkpoint_text, _, latest = checkpoint_and_latest.partition("---LATEST---")
        checkpoint_text = checkpoint_text.strip()
        checkpoint: Dict[str, Any] = {}
        if checkpoint_text:
            try:
                checkpoint = json.loads(checkpoint_text)
            except json.JSONDecodeError:
                pass
        completed = checkpoint.get("completed_iterations", 0)
        current = checkpoint.get("current_iteration", completed + 1)
        started = self.metadata.get("started_at")
        elapsed = "unknown"
        if started:
            delta = datetime.now(timezone.utc) - datetime.fromisoformat(started)
            elapsed = str(timedelta(seconds=int(delta.total_seconds())))
        score = "unavailable"
        l1 = "unavailable"
        match = re.search(r"Plus ([0-9.]+) Minus ([0-9.]+).*theta dL1=([0-9.]+)", latest)
        if match:
            score = f"plus {match.group(1)}, minus {match.group(2)}"
            l1 = match.group(3)
        print(f"VM state: {normalized_state}")
        print(f"SPSA state: {spsa_state}")
        print(f"Current SPSA iteration: {current}")
        print(f"Completed iterations: {completed}")
        print(f"Elapsed runtime: {elapsed}")
        print(f"Most recent score: {score}")
        print(f"Most recent parameter L1 change: {l1}")

    def logs(self) -> None:
        self._ssh(
            f"cd {DEFAULT_REMOTE_REPO} && "
            f"if tmux has-session -t {TMUX_SESSION} 2>/dev/null; then tmux attach -t {TMUX_SESSION}; "
            "else touch spsa_cloud.log && tail -n 100 -f spsa_cloud.log; fi",
            tty=True,
        )

    def download_state(self, *, tolerate_failure: bool = False) -> bool:
        destination = self.repo / "spsa_state"
        destination.mkdir(parents=True, exist_ok=True)
        try:
            self._download_rsync(f"{DEFAULT_REMOTE_REPO}/spsa_state/", f"{destination}/")
            return True
        except subprocess.CalledProcessError:
            if not tolerate_failure:
                raise
            print("Warning: could not download SPSA state before cleanup.", file=sys.stderr)
            return False

    def stop(self) -> None:
        self._ssh(
            f"if tmux has-session -t {TMUX_SESSION} 2>/dev/null; then "
            f"tmux send-keys -t {TMUX_SESSION} C-c; sleep 3; "
            f"tmux has-session -t {TMUX_SESSION} 2>/dev/null && tmux kill-session -t {TMUX_SESSION} || true; fi; "
            f"tmux kill-session -t {WATCHDOG_SESSION} 2>/dev/null || true; sync"
        )
        self._az(
            "vm", "deallocate", "--resource-group", self.resource_group,
            "--name", self.vm_name,
        )
        print("SPSA stopped with checkpoint state preserved; Azure VM deallocated.")

    def destroy(self) -> None:
        self.download_state(tolerate_failure=True)
        if self.metadata.get("resource_group_created", True):
            self._az("group", "delete", "--name", self.resource_group, "--yes", "--no-wait")
        else:
            self._az(
                "vm", "delete", "--resource-group", self.resource_group,
                "--name", self.vm_name, "--yes",
            )
        if not self.args.dry_run and self.metadata_path.exists():
            self.metadata_path.unlink()
        print("Azure SPSA resources scheduled for deletion.")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Persistent Azure VM runner for Howl SPSA")
    parser.add_argument("--resource-group", default=DEFAULT_RESOURCE_GROUP)
    parser.add_argument("--vm-name", default=DEFAULT_VM_NAME)
    parser.add_argument("--location", default=DEFAULT_LOCATION)
    parser.add_argument("--vm-size", default=DEFAULT_VM_SIZE)
    parser.add_argument("--admin-user", default=DEFAULT_ADMIN_USER)
    parser.add_argument("--max-runtime-hours", type=float, default=DEFAULT_RUNTIME_HOURS)
    parser.add_argument("--repo-root", default=str(Path(__file__).resolve().parent.parent))
    parser.add_argument("--local-syzygy", default="~/syzygy/3-4-5-wdl")
    parser.add_argument("--ssh-key", default=None, help="Private SSH key; its .pub file is used for VM creation")
    parser.add_argument("--metadata", default=DEFAULT_METADATA)
    parser.add_argument("--dry-run", action="store_true", help="Print commands without creating Azure resources")
    subparsers = parser.add_subparsers(dest="command", required=True)
    for command in ("launch", "status", "logs", "download-state", "stop", "destroy"):
        subparsers.add_parser(command)
    return parser


def main() -> int:
    args = build_parser().parse_args()
    if args.max_runtime_hours <= 0 or args.max_runtime_hours > 24:
        raise SystemExit("--max-runtime-hours must be greater than 0 and at most 24")
    runner = Runner(args)
    actions = {
        "launch": runner.launch,
        "status": runner.status,
        "logs": runner.logs,
        "download-state": runner.download_state,
        "stop": runner.stop,
        "destroy": runner.destroy,
    }
    try:
        actions[args.command]()
        return 0
    except (RuntimeError, subprocess.CalledProcessError) as error:
        print(f"cloud_spsa: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
