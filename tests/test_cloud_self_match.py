#!/usr/bin/env python3
"""
Unit and functional tests for tools/cloud_self_match.py
"""

import json
import os
import signal
import sys
import unittest
from unittest.mock import MagicMock, patch

from tools.cloud_self_match import (
    AzureCloudRunner,
    generate_task_specs,
    DETERMINISTIC_OPENINGS,
    query_regional_cpu_quota,
    query_multi_region_quotas,
    allocate_tasks_to_regions,
    RegionalQuota,
    calculate_waves,
    CONTAINER_CPU,
    CANDIDATE_REGIONS,
)
from tools.self_match import GameResult


class TestCloudSelfMatch(unittest.TestCase):
    def test_even_games_validation(self):
        # Odd counts or negative counts must raise ValueError
        with self.assertRaises(ValueError):
            generate_task_specs(total_games=15, movetime_sec=60, inc_sec=1)
        with self.assertRaises(ValueError):
            generate_task_specs(total_games=0, movetime_sec=60, inc_sec=1)
        with self.assertRaises(ValueError):
            generate_task_specs(total_games=-4, movetime_sec=60, inc_sec=1)

    def test_opening_and_colour_assignments(self):
        # 20 games must produce 10 openings each played twice with swapped colours
        tasks = generate_task_specs(total_games=20, movetime_sec=300, inc_sec=1.5)
        self.assertEqual(len(tasks), 20)

        for i in range(0, 20, 2):
            t1 = tasks[i]
            t2 = tasks[i + 1]

            # Same opening name and FEN
            self.assertEqual(t1.pos_name, t2.pos_name)
            self.assertEqual(t1.starting_fen, t2.starting_fen)

            # Swapped roles
            self.assertEqual(t1.white_role, "old")
            self.assertEqual(t1.black_role, "new")
            self.assertEqual(t2.white_role, "new")
            self.assertEqual(t2.black_role, "old")

            # Correct indexing
            self.assertEqual(t1.game_index, i + 1)
            self.assertEqual(t2.game_index, i + 2)

    def test_dry_run_execution(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=4,
            dry_run=True,
        )
        # Dry run should not require az login and should return empty list without deploying
        results = runner.run()
        self.assertEqual(results, [])

    def test_cleanup_handler_called_on_exception(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=4,
            dry_run=False,
        )

        with patch.object(runner, "validate_azure_login"):
            with patch.object(runner, "_execute_cloud_match", side_effect=RuntimeError("Deployment aborted")):
                with patch.object(runner, "cleanup_resources") as mock_cleanup:
                    with self.assertRaises(RuntimeError):
                        runner.run()
                    mock_cleanup.assert_called_once()

    def test_container_restart_policy_never(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=True,
        )
        cmd = runner.build_container_create_cmd(
            container_group_name="game-01",
            image_uri="myregistry.azurecr.io/runner:latest",
            registry_server="myregistry.azurecr.io",
            registry_user="user",
            registry_pass="pass",
            command_args=["python", "worker.py"],
        )
        self.assertIn("--restart-policy", cmd)
        idx = cmd.index("--restart-policy")
        self.assertEqual(cmd[idx + 1], "Never")

    def test_cleanup_skips_when_rg_not_created(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=False,
        )
        self.assertFalse(runner.resource_group_created)
        with patch("subprocess.run") as mock_run:
            success = runner.cleanup_resources()
            self.assertTrue(success)
            mock_run.assert_not_called()

    def test_cleanup_ignores_resource_group_not_found(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=False,
        )
        runner.resource_group_created = True
        mock_proc = MagicMock()
        mock_proc.returncode = 3
        mock_proc.stderr = "ERROR: (ResourceGroupNotFound) Resource group could not be found."

        with patch("subprocess.run", return_value=mock_proc):
            with patch("sys.stderr.write") as mock_err:
                success = runner.cleanup_resources()
                self.assertTrue(success)
                mock_err.assert_not_called()

    def test_worker_output_parsing(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=True,
        )
        task = runner.tasks[0]
        sample_json = {
            "game_index": 1,
            "pos_name": "Italian Game",
            "starting_fen": "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
            "white_engine": "Howl-old",
            "black_engine": "Howl-new",
            "result": "1-0",
            "winner": "old",
            "termination": "Checkmate (Howl-old wins)",
            "pgn_str": "[Event \"Howl Self Match\"]\n1. e4 e5 1-0",
        }
        stdout = f"Booting container...\n=== GAME_RESULT_JSON ===\n{json.dumps(sample_json)}\n=== END_GAME_RESULT_JSON ===\n"
        res = runner._parse_worker_output(task, stdout, "")
        self.assertEqual(res.game_index, 1)
        self.assertEqual(res.result, "1-0")
        self.assertEqual(res.winner, "old")
        self.assertEqual(res.white_engine, "Howl-old")
        self.assertIn("1. e4 e5", res.pgn_str)

    def test_container_os_type_linux(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=True,
        )
        cmd = runner.build_container_create_cmd(
            container_group_name="game-01",
            image_uri="myregistry.azurecr.io/runner:latest",
            registry_server="myregistry.azurecr.io",
            registry_user="user",
            registry_pass="pass",
            command_args=["python", "worker.py"],
        )
        self.assertIn("--os-type", cmd)
        idx = cmd.index("--os-type")
        self.assertEqual(cmd[idx + 1], "Linux")

    def test_zero_successful_dispatches_fail_immediately(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=False,
        )

        mock_failed_proc = MagicMock()
        mock_failed_proc.communicate.return_value = ("", "InvalidOsType: The 'osType' is invalid.")
        mock_failed_proc.returncode = 1

        with patch("subprocess.Popen", return_value=mock_failed_proc):
            with patch("subprocess.run") as mock_run:
                mock_run.return_value = MagicMock(returncode=0, stdout='{"username": "u", "passwords": [{"value": "p"}]}', stderr="")
                with patch("tempfile.TemporaryDirectory"):
                    with patch("shutil.copy2"), patch("os.chmod"), patch("builtins.open"):
                        with self.assertRaises(RuntimeError) as ctx:
                            runner._execute_cloud_match()
                        self.assertIn("launch dispatches failed", str(ctx.exception))

    def test_cleanup_runs_once_only(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=False,
        )
        runner.resource_group_created = True
        mock_proc = MagicMock(returncode=0, stdout="", stderr="")

        with patch("subprocess.run", return_value=mock_proc) as mock_run:
            first_res = runner.cleanup_resources()
            second_res = runner.cleanup_resources()
            self.assertTrue(first_res)
            self.assertTrue(second_res)
            mock_run.assert_called_once()

    def test_signal_handler_triggers_cleanup_and_exits(self):
        import signal

        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=False,
        )
        runner.resource_group_created = True

        with patch.object(runner, "validate_azure_login"):
            with patch.object(runner, "_execute_cloud_match", side_effect=lambda: os.kill(os.getpid(), signal.SIGINT)):
                with patch.object(runner, "cleanup_resources") as mock_cleanup:
                    with self.assertRaises(SystemExit) as exit_ctx:
                        runner.run()
                    self.assertEqual(exit_ctx.exception.code, 130)
                    mock_cleanup.assert_called_once()

    def test_sigterm_handler_triggers_cleanup_and_exits(self):
        import signal

        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=False,
        )
        runner.resource_group_created = True

        with patch.object(runner, "validate_azure_login"):
            with patch.object(runner, "_execute_cloud_match", side_effect=lambda: os.kill(os.getpid(), signal.SIGTERM)):
                with patch.object(runner, "cleanup_resources") as mock_cleanup:
                    with self.assertRaises(SystemExit) as exit_ctx:
                        runner.run()
                    self.assertEqual(exit_ctx.exception.code, 143)
                    mock_cleanup.assert_called_once()

    def test_container_command_line_includes_python_executable(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=True,
        )
        cmd = runner.build_container_create_cmd(
            container_group_name="game-01",
            image_uri="myregistry.azurecr.io/runner:latest",
            registry_server="myregistry.azurecr.io",
            registry_user="user",
            registry_pass="pass",
            command_args=["python", "-u", "/app/tools/cloud_self_match.py", "--worker-payload", "e30="],
        )
        self.assertIn("--command-line", cmd)
        cmd_line_val = cmd[cmd.index("--command-line") + 1]
        self.assertTrue(cmd_line_val.startswith("python -u /app/tools/cloud_self_match.py"))

    def test_failed_game_not_counted_as_draw(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=True,
        )
        task = runner.tasks[0]
        # Simulate worker traceback error
        tb = "Traceback (most recent call last):\n  File 'cloud_self_match.py', line 593\nTypeError: unexpected keyword argument"
        res = runner._parse_worker_output(task, "", tb)
        self.assertEqual(res.result, "*")
        self.assertEqual(res.winner, "failed")
        self.assertEqual(res.pgn_str, "")
        self.assertIn("TypeError: unexpected keyword argument", res.termination)

    def test_full_traceback_preservation(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=True,
        )
        task = runner.tasks[0]
        long_tb = "Traceback (most recent call last):\n  File 'a.py', line 1\n  File 'b.py', line 2\nValueError: invalid literal"
        res = runner._parse_worker_output(task, "", long_tb)
        self.assertIn("File 'a.py'", res.termination)
        self.assertIn("File 'b.py'", res.termination)
        self.assertIn("ValueError: invalid literal", res.termination)

    def test_empty_pgn_when_zero_games_completed(self):
        import tempfile
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=True,
        )
        with tempfile.NamedTemporaryFile("r+", encoding="utf-8") as f:
            runner.pgn_output = f.name
            task = runner.tasks[0]
            failed_res = runner._parse_worker_output(task, "", "Traceback error")
            valid_games = [r for r in [failed_res] if r.result in ("1-0", "0-1", "1/2-1/2") and r.pgn_str]
            self.assertEqual(len(valid_games), 0)

    def test_container_cpu_is_1(self):
        self.assertEqual(CONTAINER_CPU, 1)
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=True,
        )
        cmd = runner.build_container_create_cmd(
            container_group_name="game-01",
            image_uri="myregistry.azurecr.io/runner:latest",
            registry_server="myregistry.azurecr.io",
            registry_user="user",
            registry_pass="pass",
            command_args=["python", "worker.py"],
        )
        self.assertIn("--cpu", cmd)
        idx = cmd.index("--cpu")
        self.assertEqual(cmd[idx + 1], "1")

    def test_quota_parsing(self):
        mock_sub = MagicMock(returncode=0, stdout="sub-12345\n", stderr="")
        mock_usage_json = json.dumps({
            "value": [
                {"name": {"value": "ContainerGroups"}, "limit": 100, "currentValue": 0},
                {"name": {"value": "StandardCores"}, "limit": 10, "currentValue": 2},
            ]
        })
        mock_usage = MagicMock(returncode=0, stdout=mock_usage_json, stderr="")

        with patch("subprocess.run", side_effect=[mock_sub, mock_usage]):
            limit, curr, avail = query_regional_cpu_quota("eastus")
            self.assertEqual(limit, 10)
            self.assertEqual(curr, 2)
            self.assertEqual(avail, 8)

    def test_wave_calculation(self):
        # 20 tasks, 10 simultaneous -> 2 waves of 10
        waves = calculate_waves(20, 10)
        self.assertEqual(len(waves), 2)
        self.assertEqual(len(waves[0]), 10)
        self.assertEqual(len(waves[1]), 10)
        self.assertEqual(waves[0], list(range(10)))
        self.assertEqual(waves[1], list(range(10, 20)))

        # 20 tasks, 6 simultaneous -> 4 waves (6, 6, 6, 2)
        waves_6 = calculate_waves(20, 6)
        self.assertEqual(len(waves_6), 4)
        self.assertEqual(len(waves_6[0]), 6)
        self.assertEqual(len(waves_6[3]), 2)

    def test_deterministic_assignments_across_waves(self):
        tasks = generate_task_specs(total_games=20, movetime_sec=10, inc_sec=0.5)
        waves = calculate_waves(20, 10)
        # Check wave 1 games and wave 2 games preserve pairing
        wave1_tasks = [tasks[i] for i in waves[0]]
        wave2_tasks = [tasks[i] for i in waves[1]]

        self.assertEqual(len(wave1_tasks), 10)
        self.assertEqual(len(wave2_tasks), 10)
        # Pair 1 in wave 1: Game 1 (old White) vs Game 2 (new White)
        self.assertEqual(wave1_tasks[0].pos_name, wave1_tasks[1].pos_name)
        self.assertEqual(wave1_tasks[0].white_role, "old")
        self.assertEqual(wave1_tasks[1].white_role, "new")

    def test_multi_region_quota_parsing(self):
        mock_sub = MagicMock(returncode=0, stdout="sub-12345\n", stderr="")
        mock_usage_json = json.dumps({
            "value": [
                {"name": {"value": "StandardCores"}, "limit": 10, "currentValue": 2},
            ]
        })
        mock_usage = MagicMock(returncode=0, stdout=mock_usage_json, stderr="")

        with patch("subprocess.run", side_effect=[mock_sub, mock_usage, mock_usage, mock_usage, mock_usage, mock_usage]):
            quotas = query_multi_region_quotas(["eastus", "westus2"])
            self.assertIn("eastus", quotas)
            self.assertIn("westus2", quotas)
            self.assertEqual(quotas["eastus"].available_cores, 8)
            self.assertEqual(quotas["eastus"].available_capacity, 8)

    def test_20_games_split_10_plus_10_across_two_regions(self):
        tasks = generate_task_specs(total_games=20, movetime_sec=10, inc_sec=0.5)
        quotas = {
            "eastus": RegionalQuota(location="eastus", limit=10, current_usage=0, available_cores=10, available_capacity=10),
            "westus2": RegionalQuota(location="westus2", limit=10, current_usage=0, available_cores=10, available_capacity=10),
        }
        waves, total_simultaneous = allocate_tasks_to_regions(tasks, quotas)
        self.assertEqual(total_simultaneous, 20)
        self.assertEqual(len(waves), 1)  # All 20 launch in 1 wave
        wave1 = waves[0]
        self.assertEqual(len(wave1), 20)

        east_tasks = [t for t, loc in wave1 if loc == "eastus"]
        west_tasks = [t for t, loc in wave1 if loc == "westus2"]
        self.assertEqual(len(east_tasks), 10)
        self.assertEqual(len(west_tasks), 10)

        # Games 1-10 in eastus, 11-20 in westus2
        self.assertEqual([t.game_index for t in east_tasks], list(range(1, 11)))
        self.assertEqual([t.game_index for t in west_tasks], list(range(11, 21)))

    def test_partial_capacity_across_three_regions(self):
        tasks = generate_task_specs(total_games=20, movetime_sec=10, inc_sec=0.5)
        quotas = {
            "eastus": RegionalQuota(location="eastus", limit=10, current_usage=4, available_cores=6, available_capacity=6),
            "westus2": RegionalQuota(location="westus2", limit=10, current_usage=2, available_cores=8, available_capacity=8),
            "centralus": RegionalQuota(location="centralus", limit=10, current_usage=4, available_cores=6, available_capacity=6),
        }
        waves, total_simultaneous = allocate_tasks_to_regions(tasks, quotas)
        self.assertEqual(total_simultaneous, 20)
        self.assertEqual(len(waves), 1)
        self.assertEqual(len(waves[0]), 20)

    def test_fallback_to_waves_when_aggregate_capacity_is_insufficient(self):
        tasks = generate_task_specs(total_games=20, movetime_sec=10, inc_sec=0.5)
        # Total aggregate capacity = 12 (6 + 6), requires 2 waves (12 + 8)
        quotas = {
            "eastus": RegionalQuota(location="eastus", limit=10, current_usage=4, available_cores=6, available_capacity=6),
            "westus2": RegionalQuota(location="westus2", limit=10, current_usage=4, available_cores=6, available_capacity=6),
        }
        waves, total_simultaneous = allocate_tasks_to_regions(tasks, quotas)
        self.assertEqual(total_simultaneous, 12)
        self.assertEqual(len(waves), 2)
        self.assertEqual(len(waves[0]), 12)
        self.assertEqual(len(waves[1]), 8)

    def test_all_regions_unavailable(self):
        tasks = generate_task_specs(total_games=4, movetime_sec=10, inc_sec=0.5)
        quotas = {
            "eastus": RegionalQuota(location="eastus", limit=10, current_usage=10, available_cores=0, available_capacity=0),
            "westus2": RegionalQuota(location="westus2", limit=10, current_usage=10, available_cores=0, available_capacity=0),
        }
        waves, total_simultaneous = allocate_tasks_to_regions(tasks, quotas)
        self.assertEqual(total_simultaneous, 0)
        # Fallback to single tasks sequentially
        self.assertEqual(len(waves), 4)

    def test_container_create_cmd_with_location(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=10,
            inc_sec=0.5,
            total_games=2,
            dry_run=True,
        )
        cmd = runner.build_container_create_cmd(
            container_group_name="game-01",
            image_uri="myregistry.azurecr.io/runner:latest",
            registry_server="myregistry.azurecr.io",
            registry_user="user",
            registry_pass="pass",
            command_args=["python", "worker.py"],
            location="westus2",
        )
        self.assertIn("--location", cmd)
        idx = cmd.index("--location")
        self.assertEqual(cmd[idx + 1], "westus2")

    def test_running_container_not_timed_out_after_600s(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=300,
            inc_sec=1.5,
            total_games=2,
            dry_run=False,
        )
        task = runner.tasks[0]
        # Simulate state sequence: 
        # 1. show -> Running at t=120s
        # 2. show -> Terminated at t=1200s (20 minutes)
        # 3. logs -> valid game output
        valid_json = json.dumps({
            "game_index": 1,
            "pos_name": "Italian Game",
            "starting_fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            "white_engine": "Howl-old",
            "black_engine": "Howl-new",
            "result": "1/2-1/2",
            "winner": "draw",
            "termination": "Draw by 3-fold repetition",
            "pgn_str": '[Event "Cloud Self Match"]\n\n1. e4 e5 1/2-1/2',
        })
        valid_stdout = f"Worker running...\n=== GAME_RESULT_JSON ===\n{valid_json}\n"

        mock_login = MagicMock(returncode=0, stdout="{}", stderr="")
        mock_rg = MagicMock(returncode=0, stdout="{}", stderr="")
        mock_acr = MagicMock(returncode=0, stdout="{}", stderr="")
        mock_cred = MagicMock(returncode=0, stdout=json.dumps({"username": "u", "passwords": [{"value": "p"}]}), stderr="")
        mock_build = MagicMock(returncode=0, stdout="{}", stderr="")

        show_running = json.dumps({
            "provState": "Succeeded",
            "state": "Running",
            "exitCode": None,
            "detailStatus": "",
            "events": [],
        })
        show_terminated = json.dumps({
            "provState": "Succeeded",
            "state": "Terminated",
            "exitCode": 0,
            "detailStatus": "",
            "events": [],
        })
        mock_show1 = MagicMock(returncode=0, stdout=show_running, stderr="")
        mock_show2 = MagicMock(returncode=0, stdout=show_terminated, stderr="")
        mock_logs = MagicMock(returncode=0, stdout=valid_stdout, stderr="")

        # Sequence of subprocess.run calls
        # 1. az account show (validate_azure_login)
        # 2. az account show (sub_id for quota)
        # 3. az rest (quota eastus)
        # 4. az rest (quota westus2)
        # 5. az rest (quota centralus)
        # 6. az rest (quota eastus2)
        # 7. az rest (quota westeurope)
        # 8. az group create
        # 9. az acr create
        # 10. az acr credential show
        # 11. az acr build
        # Loop iteration 1: show (Running for both games)
        # Loop iteration 2: show (Terminated for both games), logs (Game 1), logs (Game 2)
        # Cleanup: az group delete
        mock_quota = MagicMock(returncode=0, stdout=json.dumps({"value": [{"name": {"value": "StandardCores"}, "limit": 10, "currentValue": 0}]}), stderr="")

        with patch("subprocess.Popen") as mock_popen, \
             patch("subprocess.run") as mock_run, \
             patch("time.sleep", return_value=None), \
             patch("time.time", side_effect=[0, 0, 100, 100, 1200, 1200, 1200, 1200, 1200, 1200]):
            mock_proc = MagicMock()
            mock_proc.communicate.return_value = ("", "")
            mock_proc.returncode = 0
            mock_popen.return_value = mock_proc

            mock_run.side_effect = [
                mock_login,
                mock_login, mock_quota, mock_quota, mock_quota, mock_quota, mock_quota,
                mock_rg, mock_acr, mock_cred, mock_build,
                mock_show1, mock_show1,
                mock_show2, mock_logs, mock_show2, mock_logs,
                MagicMock(returncode=0, stdout="", stderr=""),
            ]

            results = runner.run()
            self.assertEqual(len(results), 2)
            self.assertEqual(results[0].result, "1/2-1/2")
            self.assertEqual(results[1].result, "1/2-1/2")

    def test_unprovisioned_container_times_out_after_600s(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=300,
            inc_sec=1.5,
            total_games=2,
            dry_run=False,
        )
        show_waiting = json.dumps({
            "provState": "Creating",
            "state": "Waiting",
            "exitCode": None,
            "detailStatus": "Pulling image",
            "events": [],
        })
        mock_login = MagicMock(returncode=0, stdout="{}", stderr="")
        mock_quota = MagicMock(returncode=0, stdout=json.dumps({"value": [{"name": {"value": "StandardCores"}, "limit": 10, "currentValue": 0}]}), stderr="")
        mock_rg = MagicMock(returncode=0, stdout="{}", stderr="")
        mock_acr = MagicMock(returncode=0, stdout="{}", stderr="")
        mock_cred = MagicMock(returncode=0, stdout=json.dumps({"username": "u", "passwords": [{"value": "p"}]}), stderr="")
        mock_build = MagicMock(returncode=0, stdout="{}", stderr="")
        mock_show_waiting = MagicMock(returncode=0, stdout=show_waiting, stderr="")
        mock_delete = MagicMock(returncode=0, stdout="", stderr="")

        with patch("subprocess.Popen") as mock_popen, \
             patch("subprocess.run") as mock_run, \
             patch("time.sleep", return_value=None), \
             patch("time.time", side_effect=[0, 0, 700, 700, 700, 700, 700, 700, 700, 700]):
            mock_proc = MagicMock()
            mock_proc.communicate.return_value = ("", "")
            mock_proc.returncode = 0
            mock_popen.return_value = mock_proc

            def _side_effect(*args, **kwargs):
                cmd = args[0] if args else kwargs.get("args", [])
                if "container" in cmd and "show" in cmd:
                    return mock_show_waiting
                if "account" in cmd:
                    return mock_login
                if "rest" in cmd:
                    return mock_quota
                if "credential" in cmd:
                    return mock_cred
                return MagicMock(returncode=0, stdout="{}", stderr="")

            mock_run.side_effect = _side_effect

            results = runner.run()
            self.assertEqual(len(results), 2)
            self.assertEqual(results[0].winner, "failed")
            self.assertIn("Provisioning timeout exceeded", results[0].termination)

    def test_failed_termination_after_running_is_runtime_failure(self):
        runner = AzureCloudRunner(
            old_path="./build/howl",
            new_path="./build/howl",
            movetime_sec=300,
            inc_sec=1.5,
            total_games=2,
            dry_run=False,
        )
        task = runner.tasks[0]
        # Parse worker output with Python traceback
        res = runner._parse_worker_output(task, "", "Traceback (most recent call last):\nRuntimeError: Engine crashed")
        self.assertEqual(res.result, "*")
        self.assertEqual(res.winner, "failed")
        self.assertIn("RuntimeError: Engine crashed", res.termination)
        self.assertNotIn("Provisioning timeout", res.termination)


if __name__ == "__main__":
    unittest.main()

