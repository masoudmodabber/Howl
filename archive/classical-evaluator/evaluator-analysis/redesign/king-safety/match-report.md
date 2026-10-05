# Phase 14 Self Match Report: KingSafety Candidate 3 vs Production

## 1. Overview
Phase 14 meaningful self match evaluating **KingSafety Candidate 3** (Exact-Preserving Canonical Cleaned Model: 17 parameters, full numerical rank 17/17, condition number 3.85, 6 dead parameters excised) against the production evaluator baseline (23 parameters, condition number 6.20, 6 dead baseline-zero parameters). Both engines incorporate the frozen Attack structural redesign (10 parameters), PieceSquare structural redesign (86 parameters), and QueenMobility structural redesign (8 parameters).

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
- **Wall Clock Runtime:** 1881.9s (~31.4 min, 0.43 games/s)

---

## 3. Evidence Synthesis & Verdict

### Multi-Phase Confirmation
1. **Mathematical Representation Health (Phases 9–10):**
   - Canonical parameters reduced from 23 to 17 (-26.1%).
   - Full numerical rank (17/17) maintained.
   - Condition number improved from 6.20 to 3.85 (< 4.0).
   - Zero null dimensions, 6 baseline-zero dead parameters excised.
2. **Chess Diagnostics & Move Regret (Phases 11–13):**
   - Development failure cases (82 positions): 100% ordering consistency, 0 regressions.
   - Held-out failure cases (18 positions): 100% ordering consistency, 0 regressions.
   - Frozen 1,000-position external move regret: 0.030108 production vs 0.030108 candidate (delta = 0.000000, neutral).
   - Deterministic search regression suite: PASS (exact Perft and benchmark reproduction).
3. **Phase 14 Self Match:**
   - 800 games at 100k nodes/move yielded exactly 50.00% (-0.00 Elo, 95% CI `[-0.00, +0.00]`) with `[0, 0, 400, 0, 0]` pentanomial, confirming exact paired behavioral equivalence and zero regression.

### Qualification & Decision
- **Phase 14 Verdict:** **ACCEPTED (Exact-Preserving Structural Simplification Verified)**
- **Freezing Status:** KingSafety Candidate 3 is **FROZEN** as the canonical KingSafety representation.
- **Family Status:** KingSafety is **CLOSED**.
- **Canonical Evaluator Parameter Count:**
  - Baseline before redesigns: 261 parameters
  - Attack redesign: -50 parameters (60 $\to$ 10)
  - PieceSquare redesign: -10 parameters (96 $\to$ 86)
  - QueenMobility redesign: -2 parameters (10 $\to$ 8)
  - KingSafety redesign: -6 parameters (23 $\to$ 17)
  - Resulting canonical evaluator parameter count: **193 parameters**.
