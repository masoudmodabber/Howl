#!/usr/bin/env python3
"""Small, resumable SPSA driver for fixed-node Howl matches."""
from __future__ import annotations

import argparse, csv, json, os, random, subprocess, time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path
from typing import Optional

import chess

PARAMETERS = {
    "LMRLogarithmicScale": (24.8, 18.0, 32.0, 0.5, False),
    "LMRNonImprovingThreshold": (1007, 500, 2000, 100, True),
    "LMRTtPvAdjustment": (2, 0, 4, 1, True),
    "LMRPreviousMoveCountThreshold": (14, 4, 24, 2, True),
    "LMRSingularAdjustment": (2, 0, 4, 1, True),
    "LMRCutNodeAdjustment": (2, 0, 4, 1, True),
    "LMRStatScoreDivisor": (16384, 8192, 32768, 2048, True),
    "NullMoveBase": (854, 600, 1100, 50, True),
    "NullMoveDepthSlope": (68, 40, 100, 5, True),
    "NullMoveStatScoreLimit": (23397, 12000, 32000, 2000, True),
    "NullMoveVerificationDepth": (13, 8, 18, 1, True),
    "ReverseFutilityDepthMargin": (217, 140, 320, 20, True),
    "ParentFutilityBaseMargin": (235, 150, 350, 20, True),
    "ParentFutilityDepthMargin": (172, 100, 260, 20, True),
    "ProbCutBetaMargin": (189, 120, 280, 20, True),
    "ProbCutImprovingMargin": (45, 0, 100, 10, True),
    "SEETacticalDepthMargin": (194, 120, 280, 20, True),
    "SEEQuietBase": (32, 16, 64, 4, True),
    "HistoryLmrStatOffset": (4926, 3000, 7000, 500, True),
}

class EngineCrashError(RuntimeError):
    pass

class UCIEngineProcess:
    def __init__(self, path: str, options: Optional[dict[str, object]] = None):
        self.path = os.path.abspath(path)
        self.p = subprocess.Popen([self.path], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                                  text=True, bufsize=1)
        self._send("uci")
        self._until("uciok")
        for name, value in (options or {}).items():
            self._send(f"setoption name {name} value {value}")
        self._send("isready")
        self._until("readyok")

    def _send(self, line: str) -> None:
        if self.p.poll() is not None:
            raise EngineCrashError("engine exited")
        assert self.p.stdin
        self.p.stdin.write(line + "\n")
        self.p.stdin.flush()

    def _until(self, wanted: str) -> None:
        assert self.p.stdout
        for line in self.p.stdout:
            if line.strip() == wanted:
                return
        raise EngineCrashError(f"missing {wanted}")

    def get_move(self, starting_fen: str, moves: list[str], *, nodes_per_move: int,
                 timeout_sec: float = 300.0, **_) -> Optional[str]:
        self._send("position startpos" if starting_fen == chess.STARTING_FEN
                   else f"position fen {starting_fen}")
        if moves:
            self._send("position " + ("startpos" if starting_fen == chess.STARTING_FEN
                       else f"fen {starting_fen}") + " moves " + " ".join(moves))
        self._send(f"go nodes {nodes_per_move}")
        start = time.monotonic()
        assert self.p.stdout
        while time.monotonic() - start < timeout_sec:
            line = self.p.stdout.readline()
            if not line:
                raise EngineCrashError("engine output closed")
            if line.startswith("bestmove"):
                move = line.split()[1]
                return move
        raise EngineCrashError("engine timeout")

    def close(self) -> None:
        if self.p.poll() is None:
            try:
                self._send("quit")
                self.p.wait(timeout=2)
            except Exception:
                self.p.kill()

def load_openings(path: str, count: int) -> list[tuple[str, str]]:
    with open(path, newline="", encoding="utf-8") as f:
        return [(r["id"], r["fen"]) for r in list(csv.DictReader(f, delimiter="\t"))[:count]]

def terminal_no_move_score(board: chess.Board, plus_white: bool) -> int:
    """Return the plus-perspective result for a no-legal-move position."""
    if board.is_check():
        plus_wins = board.turn == chess.BLACK
        return 1 if plus_wins == plus_white else -1
    return 0

def run_harness_tests():
    mate = chess.Board("7k/6Q1/6K1/8/8/8/8/8 b - - 0 1")
    stale = chess.Board("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1")
    assert not any(mate.legal_moves) and mate.is_checkmate()
    assert terminal_no_move_score(mate, True) == 1
    assert not any(stale.legal_moves) and stale.is_stalemate()
    assert terminal_no_move_score(stale, True) == 0
    assert any(chess.Board("7k/8/8/8/8/8/8/7R b - - 0 1").legal_moves)
    print("terminal/resume harness tests PASS")

def fmt_options(theta: dict[str, float]) -> dict[str, object]:
    out = {}
    for name, value in theta.items():
        meta = PARAMETERS[name]
        out[name] = int(round(value)) if meta[4] else value
    return out

def clamp_value(name: str, value: float) -> float:
    meta = PARAMETERS[name]
    value = max(meta[1], min(meta[2], value))
    return float(int(round(value))) if meta[4] else value

def build_probe_vectors(theta, delta, iteration, c, floor_ratio, exploration_every, exploration_scale):
    exploring = exploration_every > 0 and iteration % exploration_every == 0
    scale = exploration_scale if exploring else 1.0
    detail = {}
    plus, minus = {}, {}
    for name, meta in PARAMETERS.items():
        base = meta[3]
        effective = max(c * base, floor_ratio * base)
        if meta[4] and base <= 1:
            effective = max(effective, 1.0)
        effective *= scale
        plus[name] = clamp_value(name, theta[name] + effective * delta[name])
        minus[name] = clamp_value(name, theta[name] - effective * delta[name])
        detail[name] = {"theta": theta[name], "delta": delta[name],
                        "base_perturbation": base,
                        "effective_perturbation": effective,
                        "theta_plus": plus[name], "theta_minus": minus[name]}
    return plus, minus, detail, exploring, scale

def estimate_normalized_gradient(score, theta_plus, theta_minus, delta):
    """Estimate d(objective)/d(normalized theta) using actual projections."""
    gradient = {}
    for name, meta in PARAMETERS.items():
        base = meta[3]
        actual_separation = (theta_plus[name] - theta_minus[name]) / base
        gradient[name] = (score / actual_separation
                          if actual_separation != 0 else 0.0)
    return gradient

def update_from_normalized(theta, gradient, learning_rate):
    """Apply a normalized-coordinate update and map back to raw theta."""
    updated = {}
    for name, meta in PARAMETERS.items():
        base, theta0 = meta[3], meta[0]
        normalized = (theta[name] - theta0) / base
        updated[name] = clamp_value(name, theta0 + base *
                                    (normalized + learning_rate * gradient[name]))
    return updated

def run_math_tests():
    names = ["LMRTtPvAdjustment", "NullMoveStatScoreLimit"]
    theta = {k: v[0] for k, v in PARAMETERS.items()}
    def probes(d0, d1, p0=None, m0=None, p1=None, m1=None):
        plus = dict(theta); minus = dict(theta)
        plus[names[0]] = theta[names[0]] + d0 if p0 is None else p0
        minus[names[0]] = theta[names[0]] - d0 if m0 is None else m0
        plus[names[1]] = theta[names[1]] + d1 if p1 is None else p1
        minus[names[1]] = theta[names[1]] - d1 if m1 is None else m1
        return plus, minus

    delta = {k: 1 for k in PARAMETERS}
    plus, minus = probes(1, -2000)
    gradient = estimate_normalized_gradient(2.0, plus, minus, delta)
    assert gradient[names[0]] > 0 and gradient[names[1]] < 0

    plus, minus = probes(-1, 2000)
    gradient = estimate_normalized_gradient(2.0, plus, minus, delta)
    assert gradient[names[0]] < 0 and gradient[names[1]] > 0

    # Raw scales 1 and 2000 produce equal normalized separations and gradients.
    plus, minus = probes(1, 2000)
    gradient = estimate_normalized_gradient(2.0, plus, minus, delta)
    assert gradient[names[0]] == gradient[names[1]]

    # Asymmetric projected probes use the actual signed separation.
    plus, minus = probes(1, 1, p0=3, m0=2, p1=24000, m1=22000)
    gradient = estimate_normalized_gradient(2.0, plus, minus, delta)
    assert gradient[names[0]] == 2.0 and gradient[names[1]] == 2.0

    equal_plus, equal_minus = probes(1, 1, p0=3, m0=3, p1=24000, m1=24000)
    gradient = estimate_normalized_gradient(2.0, equal_plus, equal_minus, delta)
    assert gradient[names[0]] == 0.0 and gradient[names[1]] == 0.0

    updated = update_from_normalized(theta, {k: (1.0 if k in names else 0.0) for k in PARAMETERS}, 1.0)
    assert updated[names[0]] == 3 and updated[names[1]] == 25397
    print("synthetic SPSA sign/scaling tests PASS")

def play_pair(task):
    idx, oid, fen, plus, minus, engine, nodes = task
    results = []
    for plus_white in (True, False):
        opts_w = fmt_options(plus if plus_white else minus)
        opts_b = fmt_options(minus if plus_white else plus)
        engines = {chess.WHITE: UCIEngineProcess(engine, opts_w),
                   chess.BLACK: UCIEngineProcess(engine, opts_b)}
        board = chess.Board(fen); moves = []
        try:
            while not board.is_game_over(claim_draw=True) and len(moves) < 300:
                side = board.turn
                if not any(board.legal_moves):
                    # Defensive terminal handling for engines that emit a no-move
                    # response before python-chess reports game-over.
                    results.append(terminal_no_move_score(board, plus_white))
                    break
                move = engines[side].get_move(fen, moves, nodes_per_move=nodes)
                if move in ("0000", "(none)", None):
                    results.append(terminal_no_move_score(board, plus_white))
                    break
                try:
                    parsed = chess.Move.from_uci(move)
                except ValueError as exc:
                    raise EngineCrashError(
                        f"unexpected engine move {move!r}; FEN={board.fen()} side={'white' if side else 'black'}"
                    ) from exc
                if parsed not in board.legal_moves:
                    raise EngineCrashError(
                        f"illegal engine move {move!r}; FEN={board.fen()} side={'white' if side else 'black'}"
                    )
                board.push_uci(move); moves.append(move)
            else:
                # The loop ended by a normal game-over condition.
                pass
            if len(results) == 0:
                outcome = board.outcome(claim_draw=True)
                score = 0 if outcome is None or outcome.winner is None else (1 if outcome.winner == chess.WHITE else -1)
                results.append(score if plus_white else -score)
        finally:
            engines[chess.WHITE].close(); engines[chess.BLACK].close()
    return {"opening_id": oid, "results": results, "score": sum(results) / 2.0}

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--engine", required=True); ap.add_argument("--suite", default="tools/matches/suites/openings-200.tsv")
    ap.add_argument("--nodes", type=int, default=1_000_000); ap.add_argument("--workers", type=int, default=8)
    ap.add_argument("--iterations", type=int, default=1); ap.add_argument("--games-per-iteration", type=int, default=16)
    ap.add_argument("--seed", type=int, default=1); ap.add_argument("--state", default="spsa_state.jsonl")
    ap.add_argument("--a", type=float, default=0.25); ap.add_argument("--c", type=float, default=0.10)
    ap.add_argument("--alpha", type=float, default=0.602); ap.add_argument("--gamma", type=float, default=0.101)
    ap.add_argument("--perturbation-floor-ratio", type=float, default=0.5)
    ap.add_argument("--exploration-every", type=int, default=20)
    ap.add_argument("--exploration-scale", type=float, default=1.5)
    ap.add_argument("--dry-run", action="store_true"); ap.add_argument("--smoke-test", action="store_true")
    ap.add_argument("--math-test", action="store_true")
    ap.add_argument("--harness-test", action="store_true")
    args = ap.parse_args()
    if args.math_test:
        run_math_tests()
        return
    if args.harness_test:
        run_harness_tests()
        return
    if args.smoke_test: args.nodes = min(args.nodes, 1000); args.games_per_iteration = 2; args.workers = 1
    rng = random.Random(args.seed)
    theta = {k: v[0] for k, v in PARAMETERS.items()}
    completed = 0
    if Path(args.state).exists():
        lines = Path(args.state).read_text().splitlines()
        if lines:
            saved = json.loads(lines[-1]); theta = saved["theta"]; completed = saved["iteration"]
            for _ in range(completed * len(PARAMETERS)):
                rng.randrange(2)
    if args.games_per_iteration < 2 or args.games_per_iteration % 2:
        ap.error("--games-per-iteration must be an even number: two games per paired opening")
    openings = load_openings(args.suite, args.games_per_iteration // 2)
    for iteration in range(completed + 1, args.iterations + 1):
        ck = args.c / (iteration ** args.gamma); ak = args.a / ((iteration + 1) ** args.alpha)
        delta = {k: (1 if rng.randrange(2) else -1) for k in PARAMETERS}
        plus, minus, probe_detail, exploring, exploration_scale = build_probe_vectors(
            theta, delta, iteration, ck, args.perturbation_floor_ratio,
            args.exploration_every, args.exploration_scale)
        if args.dry_run:
            print(json.dumps({"iteration": iteration, "mode": "exploration" if exploring else "normal",
                              "exploration_scale": exploration_scale, "parameters": probe_detail,
                              "paired_openings": [oid for oid, _ in openings],
                              "engine_commands": [f"setoption name {k} value {v}" for k, v in fmt_options(plus).items()]
                              + [f"go nodes {args.nodes}"]}, indent=2)); return
        tasks = [(i, oid, fen, plus, minus, args.engine, args.nodes) for i, (oid, fen) in enumerate(openings)]
        with ProcessPoolExecutor(max_workers=args.workers) as pool:
            results = [f.result() for f in as_completed([pool.submit(play_pair, t) for t in tasks])]
        score = sum(r["score"] for r in results) / max(1, len(results))
        gradient = estimate_normalized_gradient(score, plus, minus, delta)
        theta_before = dict(theta)
        theta = update_from_normalized(theta, gradient, ak)
        record = {"iteration": iteration, "theta_before": theta_before, "theta": theta, "theta_plus": plus, "theta_minus": minus,
                  "delta": delta, "game_results": results, "score_difference": score,
                  "gradient": gradient, "learning_rate": ak, "perturbation_scale": ck,
                  "base_perturbation": {k: v[3] for k, v in PARAMETERS.items()},
                  "effective_perturbation": {k: v["effective_perturbation"] for k, v in probe_detail.items()},
                  "exploration": exploring, "exploration_scale": exploration_scale,
                  "perturbation_floor_ratio": args.perturbation_floor_ratio,
                  "seed": args.seed, "openings": [x[0] for x in openings], "timestamp": time.time()}
        with open(args.state, "a", encoding="utf-8") as f: f.write(json.dumps(record) + "\n")
        print(json.dumps(record))

if __name__ == "__main__": main()
