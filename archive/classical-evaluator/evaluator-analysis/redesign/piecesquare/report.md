# PieceSquare Representation Redesign Report (Candidate 2)

## 1. Executive Summary
Following the Phase 14 rejection of Candidate 1 (-19.13 Elo, 95% CI `[-37.69, -0.67]`), the PieceSquare redesign returned to Phase 9. Candidate 1 demonstrated that algebraic simplification could improve conditioning, but forced removal of Bishop quadratic centrality ($p_3 = -1$) created micro-deltas that degraded playing strength.

**Candidate 2 (Conservative Quadratic & Dead Dimension Cleaned Model: 86 parameters)** is selected. It excises only proven algebraic redundancies and dead dimensions while retaining Bishop $p_3$, achieving **100.000% exact mathematical identity** with the production baseline tables across all 6 piece types in both phases:
- **Production PieceSquare Model:** 96 canonical parameters, effective rank 94 (rank deficient), condition number $\infty$, 2 null dimensions, 14 baseline zeros, and 6 severe collinear pairs ($|r| \ge 0.90$).
- **Candidate 2 Model:** 86 canonical parameters, full numerical rank (86/86), condition number **4.38** (well-conditioned, < 4.5), 0 null dimensions, and **0 severe collinear pairs** ($|r| \ge 0.90$).

---

## 2. Geometry & Conditioning Comparison

| Metric | Production PieceSquare | Candidate 1 (Rejected) | Candidate 2 (Selected) | Improvement vs Prod |
| :--- | :--- | :--- | :--- | :--- |
| **Parameter Count** | 96 | 84 | **86** | -10 (-10.4%) |
| **Effective Rank** | 94 | 84 | **86** | Full rank (86/86) |
| **Rank / Param Ratio** | 0.979 | 1.000 | **1.000** | +0.021 (Zero null dims) |
| **Condition Number** | $\infty$ (`rank_deficient`) | 4.32 | **4.38** | Well-conditioned (< 4.5) |
| **Severe Collinearities ($|r| \ge 0.90$)** | 6 pairs | 0 pairs | **0 pairs** | -6 (100% eliminated) |
| **Null / Dead Dimensions** | 2 | 0 | **0** | -2 (100% eliminated) |
| **Baseline Zero Parameters** | 14 | 4 | **6** | -8 (Excised unbacked params) |
| **Low Support Parameters** | 34 (35.4%) | 22 (26.2%) | **24 (27.9%)** | -10 (-21.1% reduction) |
| **Table Parity with Baseline** | 100% | 92.2% | **100.0% (Exact match)** | Preserves all spatial guidance |

---

## 3. Structural Mechanics and Redundancy Elimination
1. **Pawn Model (14 $\to$ 10 params):** Eliminates $p_2$ ($\text{rank}^2$, baseline 0, algebraic collinearity $r = 0.904$ MG, $r = 0.909$ EG) and $p_6$ (rank 7 promotion, baseline 0, 0 activations).
2. **Knight Model (22 $\to$ 20 params):** Eliminates $p_3$ ($\text{fc}^2$, baseline 0, algebraic collinearity $r = 0.914$ MG).
3. **Bishop Model (22 params retained):** Retains all 11 parameters per phase including $p_3 = -1$. Collinearity with linear centrality is safely sub-threshold ($r = 0.874$ MG, $r = 0.865$ EG).
4. **Rook Model (12 $\to$ 10 params):** Eliminates $p_2$ ($\text{rank}^2$, baseline 0, algebraic collinearity $r = 0.927$ MG, $r = 0.943$ EG).
5. **Queen Model (12 $\to$ 10 params):** Eliminates $p_2$ ($\text{rank}^2$, baseline 0, algebraic collinearity $r = 0.912$ EG).
6. **King Model (14 params retained):** Exact 7 step-distance parameters for MG and EG preserved.

---

## 4. Phase 10 through 13 Verification Results

### Phase 10 & 11: Development & Held-Out Failure Validation
- **Development Failure Cases (82 positions):** 100% ordering consistency, 0 regressions.
- **Held-Out Failure Cases (18 positions):** 100% ordering consistency, 0 regressions.

### Phase 12: Frozen External Move-Regret Corpus
- Evaluated against cached 5M-node Stockfish reference moves across the frozen 1,000-position corpus:
  - **Production Baseline Mean Regret:** 0.030108
  - **Candidate 2 Mean Regret:** 0.030108
  - **Regret Delta:** 0.000000 (Exact neutral, completely unimpaired)

### Phase 13: Deterministic Search Regression
- Correctness test suite (Perft depth 1-3 startpos, depth 1-2 kiwipete): PASS (exact match).
- Benchmark suite (6 positions, 4,168 nodes aggregate): PASS (identical PVs and scores).
- Tactical integrity & king safety cases: PASS.

---

## 5. Qualification Verdict
**QUALIFIES FOR PHASE 14 SELF MATCH.**
Candidate 2 restores full numerical rank (86/86), achieves a condition number of 4.38 (< 4.5), eliminates all 6 severe collinear pairs and both null dimensions, and guarantees 100.000% exact table identity with zero information loss.
