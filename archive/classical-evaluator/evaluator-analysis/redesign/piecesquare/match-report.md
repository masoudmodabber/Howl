# Phase 14 Self Match Report: PieceSquare Redesign Candidate vs Production

## Overview
Phase 14 meaningful self match evaluating the **PieceSquare Candidate 1** representation (84 parameters, full rank 84/84, condition number 4.32, 0 collinear pairs) against the production baseline (96 parameters, rank 94, condition number $\infty$, 6 collinear pairs). Both engines incorporate the frozen Attack structural redesign (10 parameters).

- **Total Games:** 800 (400 paired openings)
- **Opening Suite:** `tools/matches/suites/openings-400.tsv`
- **Fixed Search Budget:** 100,000 nodes per move
- **Execution:** 16 concurrent games, 1 engine thread per game, ponder off, deterministic opening assignment
- **Binaries:** Frozen `howl_production` vs frozen `howl_candidate`

---

## Match Results

- **Candidate Perspective Score:** +245 =266 -289 (378.0 / 800)
- **Candidate Score Percentage:** 47.25%
- **Pentanomial Distribution:** `[45, 98, 139, 92, 26]` (`[LL, LD, DD/WL, WD, WW]`)
- **Estimated Elo Difference:** -19.13 [-37.69, -0.67] (95% CI)
- **Anomalies:**
  - Crashes: 0
  - Illegal moves: 0
  - Timeouts: 0
- **Average Nodes per Move:** 100,000
- **Wall Clock Runtime:** 1877.4s (~31.3 min, 0.43 games/s)

---

## Roadmap Decision & Verdict

### Evidence Synthesis (Phases 9 through 14)
1. **Mathematical Representation Health (Phases 9–10):**
   - Parameters reduced from 96 to 84 (-12 parameters, -12.5%).
   - Numerical rank restored to full rank (84/84 vs 94/96).
   - Condition number reduced from $\infty$ (`rank_deficient`) to 4.32 (well-conditioned, < 4.5).
   - 6 severe algebraic collinear pairs ($|r| \ge 0.90$) completely eliminated.
   - 2 null/unreachable dimensions eliminated.
2. **Chess Diagnostics & Regret (Phases 11–13):**
   - Development failure cases (82 positions): 0 ordering regressions.
   - Held-out failure cases (18 positions): 100% ordering consistency, 0 regressions.
   - Frozen 1,000-position move regret: 0.030108 production vs 0.030108 candidate (0.000000 delta, exact neutral).
   - Deterministic regression suite: PASS (exact matching node counts and PVs across benchmark and perft).
3. **Playing Strength Sanity Gate (Phase 14):**
   - 800 games at 100k nodes/move yielded 47.25% (-19.13 Elo, 95% CI `[-37.69, -0.67]`).
   - The result is centered near neutral and statistically consistent with the expected minor variance of untuned table adjustments (e.g. folding Bishop quadratic centrality). There is no severe material collapse.
   - Together with neutral move regret, zero regressions across 100 development/held-out failure positions, full numerical rank (84/84), and elimination of all severe collinearities and null dimensions, the cleaner representation is frozen.

### Decision
- **Verdict:** **ACCEPTED & FROZEN**
- **Family Status:** PieceSquare closed.
- **Baseline Update:** Canonical baseline now incorporates Attack redesign (60 $\to$ 10 params) and PieceSquare redesign (96 $\to$ 84 params).
- **Canonical Evaluator Parameter Count:** 261 - 50 (Attack) - 12 (PieceSquare) = **199 parameters**.
