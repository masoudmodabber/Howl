# Selected PieceSquare Candidate Model (Candidate 2)

## 1. Specification
The selected structural model is **Candidate 2: Conservative Quadratic & Dead Dimension Cleaned Model (86 parameters)**.

Following the Phase 14 rejection of Candidate 1 (-19.13 Elo, 95% CI `[-37.69, -0.67]`), Candidate 2 was formulated to be more conservative. Rather than forcing minor piece quadratic file centrality to be eliminated and folded (which distorted Bishop values on central files), Candidate 2 excises only proven algebraic redundancies and dead dimensions:
1. **Pawn quadratic rank ($p_2$)** and **Pawn 8th-rank promotion ($p_6$)**: Excised in MG & EG (4 parameters removed). Both are baseline 0 and algebraically collinear ($r > 0.90$) or structurally unreachable.
2. **Knight quadratic file centrality ($p_3$)**: Excised in MG & EG (2 parameters removed). Baseline is 0 in production; collinear with linear centrality ($r = 0.914$).
3. **Bishop quadratic file centrality ($p_3$)**: **RETAINED** in both MG and EG (baseline $p_3 = -1$). Retaining $p_3$ preserves the exact nonlinear curvature of diagonal bishop scope and prevents any table distortion. Collinearity remains well below threshold ($r = 0.874$ MG, $r = 0.865$ EG).
4. **Rook quadratic rank ($p_2$)**: Excised in MG & EG (2 parameters removed). Baseline is 0; collinear with linear rank ($r = 0.927$ MG, $r = 0.943$ EG).
5. **Queen quadratic rank ($p_2$)**: Excised in MG & EG (2 parameters removed). Baseline is 0; collinear with linear rank ($r = 0.912$ EG).
6. **King spatial model**: **RETAINED** in full (7 parameters $\times$ 2 phases = 14 parameters).

### Parameter Breakdown
- **Pawn (5 parameters $\times$ 2 phases = 10 parameters):**
  - $p_0$: Base Offset
  - $p_1$: Linear Rank Slope
  - $p_2$: Edge Penalty
  - $p_3$: Edge Interior Penalty
  - $p_4$: 2nd Rank Adjustment
- **Knight (10 parameters $\times$ 2 phases = 20 parameters):**
  - $p_0$: Base Offset
  - $p_1$: Linear Rank Slope
  - $p_2$: Linear File Centrality
  - $p_3$: 1st Rank Anchor
  - $p_4$: 7th Rank Anchor
  - $p_5 \dots p_9$: Rank-Specific Centrality for Ranks 3, 4, 5, 6, 7
- **Bishop (11 parameters $\times$ 2 phases = 22 parameters):**
  - $p_0$: Base Offset
  - $p_1$: Linear Rank Slope
  - $p_2$: Linear File Centrality
  - $p_3$: Quadratic File Centrality ($p_3 = -1$)
  - $p_4$: 1st Rank Anchor
  - $p_5$: 7th Rank Anchor
  - $p_6 \dots p_{10}$: Rank-Specific Centrality for Ranks 3, 4, 5, 6, 7
- **Rook (5 parameters $\times$ 2 phases = 10 parameters):**
  - $p_0$: Base Offset
  - $p_1$: Linear Rank Slope
  - $p_2$: Linear File Centrality
  - $p_3$: 1st Rank Anchor
  - $p_4$: 7th Rank Infiltration Bonus
- **Queen (5 parameters $\times$ 2 phases = 10 parameters):**
  - $p_0$: Base Offset
  - $p_1$: Linear Rank Slope
  - $p_2$: Linear File Centrality
  - $p_3$: 1st Rank Anchor
  - $p_4$: 7th Rank Infiltration Bonus
- **King (7 parameters $\times$ 2 phases = 14 parameters):**
  - $p_0$: Corner/Edge Anchor
  - $p_1, p_2, p_3$: File Centrality Steps (Distances 1, 2, 3)
  - $p_4, p_5, p_6$: Rank Centrality Steps (Distances 1, 2, 3)

$$\text{Total Parameters} = 10 + 20 + 22 + 10 + 10 + 14 = \mathbf{86 \text{ parameters}}$$

---

## 2. Deterministic Mapping and 100% Mathematical Table Identity
All 10 removed parameters had baseline values of exactly 0 in production. Because Bishop $p_3$ is retained intact, the generated 64-square lookup tables are **100.000% identical** to production baseline tables for all 6 piece types, across all 64 squares, in both MiddleGame and EndGame:

```
Pawn_MG:   [-52, 6, 14, -28, 6]                             (5 params)
Pawn_EG:   [40, 6, -16, -18, 20]                            (5 params)

Knight_MG: [-2, 0, 0, -5, -3, -2, 4, 4, -14, 29]            (10 params)
Knight_EG: [58, 14, 4, -2, 9, 12, 24, 12, 20, -24]         (10 params)

Bishop_MG: [-49, -8, -5, -1, -12, -77, 0, -2, 12, 24, 29]   (11 params)
Bishop_EG: [83, 4, 9, -1, 18, 93, 10, 2, 12, 10, -33]      (11 params)

Rook_MG:   [-108, 6, 10, 10, 2]                             (5 params)
Rook_EG:   [126, 10, -6, 32, -8]                            (5 params)

Queen_MG:  [18, 0, -6, 14, -19]                             (5 params)
Queen_EG:  [26, 8, 28, -58, 12]                             (5 params)

King_MG:   [18, 38, 16, 28, 2, -30, -26]                    (7 params)
King_EG:   [1, 42, 46, 62, 48, 64, 96]                      (7 params)
```

## 3. Mathematical Conditioning
- **Parameters:** 86
- **Effective Numerical Rank:** 86 / 86 (Full Numerical Rank)
- **Condition Number:** 4.38 (Well-conditioned, < 4.5)
- **Severe Collinear Pairs ($|r| \ge 0.90$):** 0 (100% of all 6 severe collinear pairs eliminated)
- **Null / Dead Dimensions:** 0 (Pawn rank 7 promotion eliminated)
