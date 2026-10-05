# EndgameWeights Representation Redesign Report (Candidate 3)

## 1. Executive Summary
Following the roadmap established for **Attack** (60 $\to$ 10 parameters), **PieceSquare** (96 $\to$ 86 parameters), **QueenMobility** (10 $\to$ 8 parameters), and **KingSafety** (23 $\to$ 17 parameters), the **EndgameWeights** family underwent systematic empirical evaluation and representation redesign.

- **Current Production EndgameWeights Model:** 5 canonical parameters, condition number 27.57 (the worst in the entire engine), 4 pairs exceeding $|r| \ge 0.90$, with `LoneKingEdgeWeight` vs `LoneKingCornerWeight` reaching $r = -0.985$.
- **Candidate 3 (Exact-Preserving Canonical Tied Distance Model):** 3 canonical parameters (`CandLoneKingPushWeight`, `CandLoneKingConfinementWeight`, `CandLoneKingRestrictedNeighbourWeight`). The redundant collinear edge and corner degrees of freedom are unified into a single monotonic distance degree of freedom preserving the exact 72/22 canonical curvature ratio, with the base offset absorbed.
- **Conditioning Improvement:** Parameter count reduced from 5 to **3** (-40.0%), achieving full numerical rank (3/3), condition number **2.15** (well-conditioned, < 3.0), and completely eliminating all 4 severe collinear pairs ($|r| \ge 0.90$).
- **Monotonicity & Endgame Safety:**
  - 100% strictly monotonic distance progression across all 64 board squares verified ($\Delta = +116$ cp/step along diagonals).
  - KQ vs K and KR vs K bare-king checkmate tests verify identical +36 cp (center), +396 cp (edge), and +514 cp (corner) valuations.
  - Development cases (82 positions) and held-out cases (18 positions) demonstrate 100% ordering consistency with 0 regressions.
  - Frozen external move regret across the 1,000-position Stockfish reference corpus is exactly neutral (0.030108 baseline vs 0.030108 candidate).
  - Deterministic search regressions, benchmark suites, and tacticals are 100% intact.

---

## 2. Geometry & Conditioning Comparison

| Metric | Production EndgameWeights | Candidate 3 Model | Improvement vs Prod |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 5 | **3** | -2 (-40.0%) |
| **Effective Rank** | 5 | **3** | Full numerical rank (3/3) |
| **Rank / Parameter Ratio** | 1.000 | **1.000** | 1.000 (Zero null dimensions) |
| **Condition Number** | 27.57 | **2.15** | Improved (-25.42, well-conditioned < 3.0) |
| **Severe Collinear Pairs (|r| >= 0.90)** | 4 | **0** | All 4 collinear pairs eliminated |
| **Worst Correlation** | $r = -0.985$ | $r = 0.482$ | -0.503 (Extreme collinearity eliminated) |
| **Table Parity with Baseline** | 100.0% | **100.0% (Exact match)** | 100.000% mathematical match |

---

## 3. Phase 10 through 13 Verification Results

### Phase 10 & 11: Development & Held-Out Failure Validation
- **Development Failure Cases (82 positions):** 100% ordering consistency, 0 regressions.
- **Held-Out Failure Cases (18 positions):** 100% ordering consistency, 0 regressions.

### Phase 12: Frozen External Move-Regret Corpus
- Evaluated against cached 5M-node Stockfish reference moves across the frozen 1,000-position corpus:
  - **Production Baseline Mean Regret:** 0.030108
  - **Candidate 3 Mean Regret:** 0.030108
  - **Regret Delta:** 0.000000 (Exact neutral, completely unimpaired)

### Phase 13: Deterministic Search Regression
- Correctness test suite (Perft depth 1-3 startpos, depth 1-2 kiwipete): PASS (exact match).
- Benchmark suite (6 positions, 4,168 nodes aggregate): PASS (identical PVs and scores).
- Tactical integrity & mate regressions: PASS.
- Bare-king mating drives: PASS (exact monotonic progression).

---

## 4. Phase 14 Self Match & Closure Verdict
- **Phase 14 Self Match:** 800 games, +274 =252 -274 (50.00%), pentanomial `[0, 0, 400, 0, 0]`, Elo -0.00 [-0.00, +0.00], 0 errors.
- **Verdict:** **ACCEPTED (Exact-Preserving Structural Simplification Verified)**
- **Freezing Status:** EndgameWeights Candidate 3 is **FROZEN** as the canonical EndgameWeights representation.
- **Family Status:** EndgameWeights is **CLOSED**.
- **Canonical Evaluator Parameter Count:** Reduced from 193 to **191 parameters**.
