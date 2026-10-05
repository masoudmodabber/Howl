# KnightOutpost Representation Redesign Report (Candidate 1)

## 1. Executive Summary
Following the roadmap established for **Attack**, **PieceSquare**, **QueenMobility**, **KingSafety**, **EndgameWeights**, **Inline**, and **RookFile**, the **KnightOutpost** family was subjected to systematic empirical analysis, geometric decomposition, and representation redesign.

- **Current Production Model:** 4 canonical parameters (`KnightOutpostMiddleGame`, `KnightOutpostEndGame`, `KnightSupportedOutpostMiddleGame`, `KnightSupportedOutpostEndGame`). Full numerical rank (4/4), condition number 2.18 (well-conditioned), 0 severe collinear pairs ($|r| \ge 0.90$).
- **Support Split Independence Analysis:**
  - Auditing across 1,000 reference game positions revealed 190 total outpost knights across 165 active positions.
  - Of all active outposts, **110 (57.9%) are unsupported** (occupying outposts without friendly pawn protection) and **80 (42.1%) are pawn-supported**.
  - The support status provides an independent, non-collinear chess signal (correlation between base and supported terms is sub-critical, $r = 0.748$ MG, $r = 0.560$ EG). Merging them would destroy essential discrimination between stable anchors and temporary tactical outposts.
- **Negative EG Outpost Analysis & Overlap:**
  - In endgames, an unsupported outpost knight receives -3 cp base value. Far from an error, this operates as a legitimate dampener against knights stranded on unanchored central squares away from passed pawns or king defense.
  - Overlap with `PieceSquare` minor rank bonuses ($r = 0.65 - 0.72$) and `KnightMobility` ($r = 0.71 - 0.78$) is healthy and complementary: PST provides static baseline centralization, mobility measures dynamic square control, and KnightOutpost rewards unchallengeable square immunity.
- **Candidate 1 (Exact-Preserving Canonical Model):** Preserves the healthy 4-parameter canonical formulation, maintaining **100.000% exact evaluation score identity**.

---

## 2. Geometry & Conditioning Comparison

| Metric | Production KnightOutpost | Candidate 1 Model | Improvement vs Prod |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 4 | **4** | 0 (Preserved canonical dimension) |
| **Effective Rank** | 4 | **4** | Full numerical rank (4/4) |
| **Rank / Parameter Ratio** | 1.000 | **1.000** | 1.000 (Zero null dimensions) |
| **Condition Number** | 2.18 | **2.18** | Well-conditioned (< 2.5) |
| **Severe Collinear Pairs (|r| >= 0.90)** | 0 | **0** | Clean orthogonal structure |
| **Dead Dimensions** | 0 | **0** | 0 (None) |
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
The Phase 9 through 13 audit confirmed that no structural redesign, parameter reduction, formula modification, or value change is justified. The existing 4-parameter formulation is full rank (4/4, condition number 2.18), supported vs unsupported outposts are independently activated (42.1% supported vs 57.9% unsupported), and overlap with `PieceSquare` and `KnightMobility` is healthy and complementary. The negative EG value is a tuning property rather than a representation defect. With 100.000% exact parity across all gates, there is no distinct candidate to test. Canonical evaluator parameter count remains 190.

