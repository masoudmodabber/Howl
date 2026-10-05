# Inline Representation Redesign Report (Candidate 1)

## 1. Executive Summary
Following the roadmap established for **Attack** (60 $\to$ 10 parameters), **PieceSquare** (96 $\to$ 86 parameters), **QueenMobility** (10 $\to$ 8 parameters), **KingSafety** (23 $\to$ 17 parameters), and **EndgameWeights** (5 $\to$ 3 parameters), the **Inline** family underwent systematic empirical evaluation and representation redesign.

- **Current Production Inline Model:** 8 canonical parameters, rank deficient (rank 7/8) with 1 completely dead unidentifiable dimension (`BishopPairValue`), infinite condition number, and 4 low-support parameters.
- **Candidate 1 (Exact-Preserving Canonical Cleaned Model):** 7 canonical parameters (`CandBishopOpenFilePawnScale`, `CandTempoMiddleGame`, `CandTempoEndGame`, `CandOppositeColorBishopMiddleGameScalePermille`, `CandOppositeColorBishopEndGameScalePermille`, `CandEndgamePawnAdvancementRankMultiplier`, `CandPieceAttackScalePercent`). The dead dimension `BishopPairValue` is excised from tuneable parameters (frozen as an architectural baseline constant 132).
- **Conditioning Improvement:** Parameter count reduced from 8 to **7** (-12.5%), achieving full numerical rank (7/7), condition number **2.42** (well-conditioned, < 3.0), and zero null dimensions.
- **Chess Information Preservation:**
  - 100% of global evaluation terms (middle-game/end-game tempo, opposite-color bishop damping, passed pawn advancement scaling, and attack percentage multiplier) are strictly preserved.
  - Development cases (82 positions) and held-out cases (18 positions) demonstrate 100% ordering consistency with 0 regressions.
  - Frozen external move regret across the 1,000-position Stockfish reference corpus is exactly neutral (0.030108 baseline vs 0.030108 candidate).
  - Deterministic search regressions, benchmark suites, and tacticals are 100% intact.

---

## 2. Geometry & Conditioning Comparison

| Metric | Production Inline | Candidate 1 Model | Improvement vs Prod |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 8 | **7** | -1 (-12.5%) |
| **Effective Rank** | 7 (Rank deficient) | **7** | Full numerical rank (7/7) |
| **Rank / Parameter Ratio** | 0.875 | **1.000** | 1.000 (Zero null dimensions) |
| **Condition Number** | $\infty$ (`rank_deficient`) | **2.42** | Well-conditioned (< 3.0) |
| **Dead Dimensions** | 1 (`BishopPairValue`) | **0** | Dead dimension excised |
| **Low-Support Count** | 4 (50.0%) | **3 (42.9%)** | -1 (-25.0% reduction) |
| **Table Parity with Baseline** | 100.0% | **100.0% (Exact match)** | 100.000% mathematical match |

---

## 3. Phase 10 through 13 Verification Results

### Phase 10 & 11: Development & Held-Out Failure Validation
- **Development Failure Cases (82 positions):** 100% ordering consistency, 0 regressions.
- **Held-Out Failure Cases (18 positions):** 100% ordering consistency, 0 regressions.

### Phase 12: Frozen External Move-Regret Corpus
- Evaluated against cached 5M-node Stockfish reference moves across the frozen 1,000-position corpus:
  - **Production Baseline Mean Regret:** 0.030108
  - **Candidate 1 Mean Regret:** 0.030108
  - **Regret Delta:** 0.000000 (Exact neutral, completely unimpaired)

### Phase 13: Deterministic Search Regression
- Correctness test suite (Perft depth 1-3 startpos, depth 1-2 kiwipete): PASS (exact match).
- Benchmark suite (6 positions, 4,168 nodes aggregate): PASS (identical PVs and scores).
- Tactical integrity & mate regressions: PASS.

---

## 4. Phase 14 Self Match & Closure Verdict
- **Phase 14 Self Match:** 800 games, +274 =252 -274 (50.00%), pentanomial `[0, 0, 400, 0, 0]`, Elo -0.00 [-0.00, +0.00], 0 errors.
- **Verdict:** **ACCEPTED (Exact-Preserving Structural Simplification Verified)**
- **Freezing Status:** Inline Candidate 1 is **FROZEN** as the canonical Inline representation.
- **Family Status:** Inline is **CLOSED**.
- **Canonical Evaluator Parameter Count:** Reduced from 191 to **190 parameters**.
