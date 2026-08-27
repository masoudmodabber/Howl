#!/usr/bin/env python3
"""
Unit tests for the self-match harness logic.
Runs fast mock-based tests without launching full engine matches.
"""

import os
import sys
import unittest
from unittest.mock import MagicMock, patch

import chess

# Ensure tools directory is on path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "tools")))
import self_match


class TestSelfMatchHarness(unittest.TestCase):
    def test_starting_positions_validity(self):
        """Ensure all defined starting positions have valid FEN strings and are varied."""
        self.assertEqual(len(self_match.STARTING_POSITIONS), 4)
        for name, fen in self_match.STARTING_POSITIONS:
            board = chess.Board(fen)
            self.assertTrue(board.is_valid(), f"Position '{name}' FEN is invalid: {fen}")

    def test_summary_calculation(self):
        """Test summary counting logic."""
        mock_results = [
            self_match.GameResult(
                game_index=1,
                pos_name="Test Pos 1",
                starting_fen=chess.STARTING_FEN,
                white_engine="Howl-old",
                black_engine="Howl-new",
                result="1-0",
                winner="old",
                termination="Checkmate (Howl-old wins)",
                pgn_str="[Event \"Test\"]",
            ),
            self_match.GameResult(
                game_index=2,
                pos_name="Test Pos 1",
                starting_fen=chess.STARTING_FEN,
                white_engine="Howl-new",
                black_engine="Howl-old",
                result="1/2-1/2",
                winner=None,
                termination="Stalemate",
                pgn_str="[Event \"Test\"]",
            ),
        ]

        old_wins = sum(1 for r in mock_results if r.winner == "old")
        new_wins = sum(1 for r in mock_results if r.winner == "new")
        draws = sum(1 for r in mock_results if r.winner is None)
        old_score = old_wins + 0.5 * draws
        new_score = new_wins + 0.5 * draws

        self.assertEqual(old_wins, 1)
        self.assertEqual(new_wins, 0)
        self.assertEqual(draws, 1)
        self.assertEqual(old_score, 1.5)
        self.assertEqual(new_score, 0.5)

    def test_illegal_move_adjudication(self):
        """Test that an illegal move from an engine causes immediate forfeiture."""
        class MockEngine:
            def __init__(self, moves):
                self.moves = list(moves)

            def get_move(self, **kwargs):
                if self.moves:
                    return self.moves.pop(0)
                return None

            def close(self):
                pass

        # White plays e2e4, Black plays illegal move e7e2
        mock_white = MockEngine(["e2e4"])
        mock_black = MockEngine(["e7e2"])

        with patch("self_match.UCIEngineProcess", side_effect=[mock_white, mock_black]):
            res = self_match.play_single_game(
                game_idx=1,
                pos_name="Standard",
                starting_fen=chess.STARTING_FEN,
                white_role="old",
                black_role="new",
                old_engine_path="/fake/old",
                new_engine_path="/fake/new",
                base_time_sec=10.0,
                inc_sec=0.1,
            )

            self.assertEqual(res.result, "1-0")
            self.assertEqual(res.winner, "old")
            self.assertIn("Illegal move", res.termination)

    def test_checkmate_adjudication(self):
        """Test Fool's mate flow where Black checkmates White."""
        class MockEngine:
            def __init__(self, moves):
                self.moves = list(moves)

            def get_move(self, **kwargs):
                if self.moves:
                    return self.moves.pop(0)
                return None

            def close(self):
                pass

        # Fool's mate: 1. f3 e5 2. g4 Qh4#
        mock_white = MockEngine(["f2f3", "g2g4"])
        mock_black = MockEngine(["e7e5", "d8h4"])

        with patch("self_match.UCIEngineProcess", side_effect=[mock_white, mock_black]):
            res = self_match.play_single_game(
                game_idx=1,
                pos_name="Standard",
                starting_fen=chess.STARTING_FEN,
                white_role="old",
                black_role="new",
                old_engine_path="/fake/old",
                new_engine_path="/fake/new",
                base_time_sec=10.0,
                inc_sec=0.1,
            )

            self.assertEqual(res.result, "0-1")
            self.assertEqual(res.winner, "new")
            self.assertIn("Checkmate", res.termination)

    def test_crash_diagnostics_formatting(self):
        """Test crash diagnostics reporting fields and PGN header embedding."""
        class CrashEngine:
            def get_move(self, **kwargs):
                raise self_match.EngineCrashError(
                    message="Engine standard output closed unexpectedly",
                    returncode=-11,
                    last_uci_command="go wtime 4000 btime 4000 winc 100 binc 100",
                    last_bestmove="d7d5",
                    stderr_lines=["Segmentation fault (core dumped)", "Fatal error"],
                )

            def close(self):
                pass

        class NormalEngine:
            def get_move(self, **kwargs):
                return "e2e4"

            def close(self):
                pass

        with patch("self_match.UCIEngineProcess", side_effect=[NormalEngine(), CrashEngine()]):
            res = self_match.play_single_game(
                game_idx=1,
                pos_name="French Defense Advance",
                starting_fen=chess.STARTING_FEN,
                white_role="new",
                black_role="old",
                old_engine_path="/fake/old",
                new_engine_path="/fake/new",
                base_time_sec=5.0,
                inc_sec=0.1,
            )

            self.assertEqual(res.result, "1-0")
            self.assertEqual(res.winner, "new")
            self.assertIn("Engine crash (old)", res.termination)
            self.assertIn("Return code: -11", res.termination)
            self.assertIn("Last UCI sent: go wtime 4000 btime 4000 winc 100 binc 100", res.termination)
            self.assertIn("Last bestmove received: d7d5", res.termination)
            self.assertIn("Segmentation fault", res.termination)
            self.assertIn("CrashDetails", res.pgn_str)


if __name__ == "__main__":
    unittest.main()
