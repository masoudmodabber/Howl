# QueenMobility Representation Redesign Report (Candidate 3)

## 1. Executive Summary
Following the roadmap established for **Attack** (60 $\to$ 10 parameters) and **PieceSquare** (96 $\to$ 86 parameters), the **QueenMobility** family underwent systematic empirical evaluation and representation redesign.

- **Current Production QueenMobility Model:** 10 canonical parameters (5 MiddleGame, 5 EndGame), effective rank 10, condition number 3.14, with 8 of 10 parameters suffering from low empirical support (<7% activation in normal play) and adjacent saturation redundancy.
- **Candidate 3 (Exact-Preserving Canonical Tied Model):** 8 canonical parameters structured by eliminating adjacent saturation duplicates while preserving **100.000% exact mathematical table identity** with the production baseline across all 28 move counts in both phases.
- **Conditioning Improvement:** Parameter count reduced from 10 to **8** (-20.0%), achieving full numerical rank (8/8), condition number **2.95** (well-conditioned, < 3.0), zero null dimensions, and eliminating 2 redundant low-support parameters.
- **Chess Information Preservation:**
  - 100% of queen mobility behaviors (trapped queen penalties, active scope transitions, and endgame scaling) are strictly preserved.
  - Development cases (82 positions) and held-out cases (18 positions) demonstrate 100% ordering consistency with 0 regressions.
  - Frozen external move regret across the 1,000-position Stockfish reference corpus is exactly neutral (0.030108 baseline vs 0.030108 candidate).
  - Deterministic search regressions, benchmark suites, and tacticals are 100% intact.

---

## 2. Geometry & Conditioning Comparison

| Metric | Production QueenMobility | Candidate 3 Model | Improvement vs Prod |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 10 | **8** | -2 (-20.0%) |
| **Effective Rank** | 10 | **8** | Full numerical rank (8/8) |
| **Rank / Parameter Ratio** | 1.000 | **1.000** | 1.000 (Zero null dimensions) |
| **Condition Number** | 3.14 | **2.95** | Improved (-0.19, well-conditioned) |
| **Low Support Parameters** | 8 (80.0%) | **6 (75.0%)** | -2 (-25.0% reduction) |
| **Severe Collinear Pairs (|r| >= 0.90)** | 0 | **0** | Clean internal structure |
| **Table Parity with Baseline** | 100.0% | **100.0% (Exact match)** | 100.000% mathematical match |

---

## 3. Structural Mechanics and Redundancy Elimination
1. **MiddleGame (5 $\to$ 4 parameters):** In baseline Howl, $p_3 = 4$ and $p_4 = 4$ are identical adjacent saturation increments covering counts 9..12 and 13..15. Candidate 3 ties these into a single unified saturation parameter $p_{\text{sat}} = 4$, generating the exact 28-element MG table identical to baseline.
2. **EndGame (5 $\to$ 4 parameters):** The terminal endgame slope is maintained as an architectural constant while preserving full tuneability of the early and mid endgame activity progression.

---

## 4. Phase 10 through 13 Verification Results

### Phase 10 & 11: Development & Held-Out Failure Validation
- **Development Failure Cases (82 positions):** 100% ordering consistency, 0 regressions.
- **Held-Out Failure Cases (18 positions):** 100% ordering consistency, 0 regressions.

### Phase 12: Frozen External Move-Regret Corpus
- Evaluated against cached 5M-node Stockfish reference moves across the frozen 1,000-position corpus:
  - **Production Baseline Mean Regret:** 0.030108
  - **Candidate 3 Mean Regret:** 0.030108
  - **Regret Delta:** 0.000000 (Exact neutral, completely unimpaired)

### Phase 13: Deterministic Search Regression
- Correctness test suite (Perft depth 1-3 startpos, depth 1-2 kiwipete, position3): PASS (exact match).
- Benchmark suite (6 positions, 4,168 nodes aggregate): PASS (identical PVs and scores).
- Tactical integrity & mate regressions: PASS.

---

## 5. Qualification Verdict
**QUALIFIES FOR PHASE 14 SELF MATCH.**
Candidate 3 achieves full numerical rank (8/8), improves condition number to 2.95 (< 3.0), eliminates low-support saturation redundancies, and guarantees 100.000% exact table identity with zero information loss across all evaluation and search benchmarks.
