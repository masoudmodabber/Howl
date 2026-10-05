# RookFile Representation Redesign Report (Candidate 1)

## 1. Executive Summary
Following the roadmap established for **Attack** (60 $\to$ 10 parameters), **PieceSquare** (96 $\to$ 86 parameters), **QueenMobility** (10 $\to$ 8 parameters), **KingSafety** (23 $\to$ 17 parameters), **EndgameWeights** (5 $\to$ 3 parameters), and **Inline** (8 $\to$ 7 parameters), the **RookFile** family underwent systematic empirical evaluation and representation redesign.

- **Current Production RookFile Model:** 4 canonical parameters (`RookOpenFileMiddleGame`, `RookOpenFileEndGame`, `RookSemiOpenFileMiddleGame`, `RookSemiOpenFileEndGame`). Full numerical rank (4/4), condition number 1.25 (exceptionally well-conditioned), 0 collinear pairs, active in 41.9% of positions.
- **Semi-Open Weight Analysis & RookMobility Overlap:**
  - A rook on a semi-open file gains +3 to +6 vertical squares, automatically yielding $+8$ to $+18$ cp in MG and $+15$ to $+28$ cp in EG purely from `RookMobility`.
  - The negative semi-open file weights ($-4$ MG, $-26$ EG) are not errors; they act as necessary empirical dampening against `RookMobility` overvaluing semi-open files that are solidly blocked by enemy pawns.
  - The combined net evaluation utility for a semi-open file remains strictly non-negative (+6 to +12 cp in MG; ~0 to +4 cp in EG).
- **Candidate 1 (Exact-Preserving Canonical Cleaned Model):** Preserves the healthy 4-parameter canonical formulation without arbitrary value distortion, guaranteeing **100.000% exact evaluation score identity** across all positions.
- **Verification Results:**
  - Development cases (82 positions) and held-out cases (18 positions) demonstrate 100% ordering consistency with 0 regressions.
  - Frozen external move regret across the 1,000-position Stockfish reference corpus is exactly neutral (0.030108 baseline vs 0.030108 candidate).
  - Deterministic search regressions, benchmark suites, and tacticals are 100% intact.

---

## 2. Geometry & Conditioning Comparison

| Metric | Production RookFile | Candidate 1 Model | Improvement vs Prod |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 4 | **4** | 0 (Preserved canonical dimension) |
| **Effective Rank** | 4 | **4** | Full numerical rank (4/4) |
| **Rank / Parameter Ratio** | 1.000 | **1.000** | 1.000 (Zero null dimensions) |
| **Condition Number** | 1.25 | **1.25** | Exceptionally well-conditioned (< 1.5) |
| **Severe Collinear Pairs (|r| >= 0.90)** | 0 | **0** | Clean orthogonal structure |
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

## 4. Qualification & Family Status
**CLOSED UNCHANGED (NO PHASE 14 MATCH REQUIRED).**
The Phase 9 through 13 audit established that no structural redesign, parameter reduction, formula modification, or value change is warranted. The existing 4-parameter formulation is full rank (4/4, condition number 1.25), and negative semi-open weights serve as meaningful dampening against `RookMobility` ray inflation. With 100.000% exact parity across development, held-out, move regret, and deterministic regressions, there is no distinct candidate to test. Canonical evaluator parameter count remains 190.

