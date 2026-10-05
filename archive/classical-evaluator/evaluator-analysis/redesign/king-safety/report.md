# KingSafety Representation Redesign Report (Candidate 3)

## 1. Executive Summary
Following the roadmap established for **Attack** (60 $\to$ 10 parameters), **PieceSquare** (96 $\to$ 86 parameters), and **QueenMobility** (10 $\to$ 8 parameters), the **KingSafety** family underwent systematic empirical evaluation and representation redesign.

- **Current Production KingSafety Model:** 23 canonical parameters across 4 sub-groups, condition number 6.20 (ill-conditioned), 11 low-support parameters (47.8%), and 6 baseline-zero parameters (26.1%).
- **Candidate 3 (Exact-Preserving Canonical Cleaned Model):** 17 canonical parameters structured by excising the 6 baseline-zero dead parameters while preserving **100.000% exact mathematical evaluation score identity** with the production baseline across all positions.
- **Conditioning Improvement:** Parameter count reduced from 23 to **17** (-26.1%), achieving full numerical rank (17/17), condition number **3.85** (well-conditioned, < 4.0), zero null dimensions, and eliminating 6 low-support dead parameters.
- **Chess Information Preservation:**
  - 100% of king safety behaviors (attacker convergence, defender dampening, shelter defects, line open/semi-open pressure, and escape squares) are strictly preserved.
  - Development cases (82 positions) and held-out cases (18 positions) demonstrate 100% ordering consistency with 0 regressions.
  - Frozen external move regret across the 1,000-position Stockfish reference corpus is exactly neutral (0.030108 baseline vs 0.030108 candidate).
  - Deterministic search regressions, benchmark suites, and tacticals are 100% intact.

---

## 2. Geometry & Conditioning Comparison

| Metric | Production KingSafety | Candidate 3 Model | Improvement vs Prod |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 23 | **17** | -6 (-26.1%) |
| **Effective Rank** | 23 | **17** | Full numerical rank (17/17) |
| **Rank / Parameter Ratio** | 1.000 | **1.000** | 1.000 (Zero null dimensions) |
| **Condition Number** | 6.20 | **3.85** | Improved (-2.35, well-conditioned < 4.0) |
| **Baseline Zero Parameters** | 6 (26.1%) | **0 (0.0%)** | -6 (-100% dead dimensions excised) |
| **Low Support Parameters** | 11 (47.8%) | **5 (29.4%)** | -6 (-54.5% reduction) |
| **Severe Collinear Pairs (|r| >= 0.90)** | 0 | **0** | Clean internal structure |
| **Table Parity with Baseline** | 100.0% | **100.0% (Exact match)** | 100.000% mathematical match |

---

## 3. Low-Support and Dead Parameter Elimination
The 6 baseline-zero parameters excised in Candidate 3 are:
1. `KingAttackerPawnWeight` ($p_0 = 0$): Category C (Pawn attacks near king are handled by pawn storms and undefended zone hits).
2. `KingShelterOpenFileDanger` ($p_{11} = 0$): Category C (Shelter file openness is captured by `KingShelterMissingPawnDanger`).
3. `KingBlockedEscapeDanger` ($p_{18} = 0$): Category C (Own occupancy restriction is effectively subsumed by escape counts).
4. `KingTrappedEscapeDanger` ($p_{19} = 0$): Category B/C (Severely trapped king situations are dominated by direct mating threats).
5. `KingHeavyBatteryDanger` ($p_{20} = 0$): Category D (Major piece convergence along files/diagonals is captured by `KingOpenLineDanger` and queen infiltration).
6. `KingPinnedShelterPawnWeight` ($p_{21} = 0$): Category D (Pinned shelter pawns are scored by tactical pin evaluation in Attack/PieceSquare).

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
- Correctness test suite (Perft depth 1-3 startpos, depth 1-2 kiwipete): PASS (exact match).
- Benchmark suite (6 positions, 4,168 nodes aggregate): PASS (identical PVs and scores).
- Tactical integrity & mate regressions: PASS.

---

## 5. Phase 14 Self Match & Closure Verdict
- **Phase 14 Self Match:** 800 games, +274 =252 -274 (50.00%), pentanomial `[0, 0, 400, 0, 0]`, Elo -0.00 [-0.00, +0.00], 0 errors.
- **Verdict:** **ACCEPTED (Exact-Preserving Structural Simplification Verified)**
- **Freezing Status:** KingSafety Candidate 3 is **FROZEN** as the canonical KingSafety representation.
- **Family Status:** KingSafety is **CLOSED**.
- **Canonical Evaluator Parameter Count:** Reduced from 199 to **193 parameters**.
