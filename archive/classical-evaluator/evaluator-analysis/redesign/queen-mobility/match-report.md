# Phase 14 Self Match Report: QueenMobility Candidate 3 vs Production

## 1. Overview
Phase 14 meaningful self match evaluating **QueenMobility Candidate 3** (Exact-Preserving Canonical Tied Model: 8 parameters, full numerical rank 8/8, condition number 2.95) against the production evaluator baseline (10 parameters, condition number 3.14, adjacent saturation redundancy). Both engines incorporate the frozen Attack structural redesign (10 parameters) and the frozen PieceSquare structural redesign (86 parameters).

- **Total Games:** 800 (400 paired openings)
- **Opening Suite:** `tools/matches/suites/openings-400.tsv`
- **Fixed Search Budget:** 100,000 nodes per move
- **Execution:** 16 concurrent games, 1 engine thread per game, ponder off, deterministic opening assignment
- **Binaries:** Frozen `howl_production` vs frozen `howl_candidate`

---

## 2. Match Results (Candidate 3 Perspective)

- **Candidate 3 W / D / L:** +274 =252 -274 (400.0 / 800)
- **Score Percentage:** 50.00%
- **Pentanomial Distribution:** `[0, 0, 400, 0, 0]` (`[LL, LD, DD/WL, WD, WW]`)
- **Estimated Elo Difference:** -0.00 [-0.00, +0.00] (95% CI)
- **Anomalies:**
  - Crashes: 0
  - Illegal moves: 0
  - Timeouts: 0
- **Average Nodes per Move:** 100,000
- **Wall Clock Runtime:** 1884.8s (~31.4 min, 0.42 games/s)

---

## 3. Evidence Synthesis & Verdict

### Multi-Phase Confirmation
1. **Mathematical Representation Health (Phases 9–10):**
   - Canonical parameters reduced from 10 to 8 (-20.0%).
   - Full numerical rank (8/8) maintained.
   - Condition number improved from 3.14 to 2.95 (< 3.0).
   - Zero null dimensions, redundant adjacent saturation degrees of freedom excised.
2. **Chess Diagnostics & Move Regret (Phases 11–13):**
   - Development failure cases (82 positions): 100% ordering consistency, 0 regressions.
   - Held-out failure cases (18 positions): 100% ordering consistency, 0 regressions.
   - Frozen 1,000-position external move regret: 0.030108 production vs 0.030108 candidate (delta = 0.000000, neutral).
   - Deterministic search regression suite: PASS (exact Perft and benchmark reproduction).
3. **Phase 14 Self Match:**
   - 800 games at 100k nodes/move yielded exactly 50.00% (-0.00 Elo, 95% CI `[-0.00, +0.00]`) with `[0, 0, 400, 0, 0]` pentanomial, confirming exact paired behavioral equivalence and zero regression.

### Qualification & Decision
- **Phase 14 Verdict:** **ACCEPTED (Exact-Preserving Structural Simplification Verified)**
- **Freezing Status:** QueenMobility Candidate 3 is **FROZEN** as the canonical QueenMobility representation.
- **Family Status:** QueenMobility is **CLOSED**.
- **Canonical Evaluator Parameter Count:**
  - Baseline before redesigns: 261 parameters
  - Attack redesign: -50 parameters (60 $\to$ 10)
  - PieceSquare redesign: -10 parameters (96 $\to$ 86)
  - QueenMobility redesign: -2 parameters (10 $\to$ 8)
  - Resulting canonical evaluator parameter count: **199 parameters**.
