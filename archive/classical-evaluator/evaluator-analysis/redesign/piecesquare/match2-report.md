# Phase 14 Self Match Report: PieceSquare Candidate 2 vs Production

## Overview
Phase 14 meaningful self match evaluating **PieceSquare Candidate 2** (Conservative Quadratic & Dead Dimension Cleaned Model: 86 parameters, full numerical rank 86/86, condition number 4.38, 0 severe collinear pairs) against the production evaluator baseline (96 parameters, rank 94, condition number $\infty$, 6 collinear pairs). Both engines incorporate the frozen Attack structural redesign (10 parameters).

- **Total Games:** 800 (400 paired openings)
- **Opening Suite:** `tools/matches/suites/openings-400.tsv`
- **Fixed Search Budget:** 100,000 nodes per move
- **Execution:** 16 concurrent games, 1 engine thread per game, ponder off, deterministic opening assignment
- **Binaries:** Frozen `howl_production` vs frozen `howl_candidate`

---

## Match Results (Candidate 2 Perspective)

- **Candidate 2 W / D / L:** +274 =252 -274 (400.0 / 800)
- **Score Percentage:** 50.00%
- **Pentanomial Distribution:** `[0, 0, 400, 0, 0]` (`[LL, LD, DD/WL, WD, WW]`)
- **Estimated Elo Difference:** -0.00 [-0.00, +0.00] (95% CI)
- **Anomalies:**
  - Crashes: 0
  - Illegal moves: 0
  - Timeouts: 0
- **Average Nodes per Move:** 100,000
- **Wall Clock Runtime:** 1875.8s (~31.3 min, 0.43 games/s)

---

## Roadmap Decision & Verdict

### Decision Rule Application
- The 95% confidence interval spans neutral ([ -0.00, +0.00 ]), score percentage is exactly 50.00%, and every opening pair resulted in a symmetric draw / 1-1 split ([0, 0, 400, 0, 0]).
- Combined with:
  1. Full numerical rank (86 / 86 vs 94 / 96) and condition number 4.38 (< 4.5).
  2. Elimination of all 6 severe collinear pairs (|r| >= 0.90) and 2 null dimensions.
  3. Exact 100.000% baseline PST table identity preserving Bishop quadratic file centrality ($p_3 = -1$).
  4. Zero regressions across all 82 development and 18 held-out positions.
  5. Neutral frozen move regret (0.030108 baseline vs 0.030108 Candidate 2).
  6. Passing deterministic search regression suite (Perft, benchmark).

### Conclusion
- **Phase 14 Verdict:** **ACCEPTED (Structurally Neutral / Verified Simplification)**
- **Freezing Status:** PieceSquare Candidate 2 is **FROZEN** as the canonical PieceSquare representation.
- **Family Status:** PieceSquare closed.
- **Canonical Evaluator Parameter Count:**
  - Initial baseline: 261 parameters
  - Attack redesign: -50 parameters (60 $\to$ 10)
  - PieceSquare redesign: -10 parameters (96 $\to$ 86)
  - Resulting canonical evaluator parameters: **201 parameters**.
