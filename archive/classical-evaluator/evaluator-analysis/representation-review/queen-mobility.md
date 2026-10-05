# QueenMobility Representation Audit

## 1. Parameter Architecture & Organization
- **Total Canonical Parameters:** 10 parameters across MiddleGame and EndGame.
  - 5 MiddleGame parameters: `QueenMobilityMiddleGameBase`, `QueenMobilityMiddleGameIncrement_1..4`.
  - 5 EndGame parameters: `QueenMobilityEndGameBase`, `QueenMobilityEndGameIncrement_1..4`.
- **Generation & Decoding ([MobilityV2.h](file:///home/masoud/Code/Howl/MobilityV2.h)):**
  - Uses anchor decoding: `anchors[i] = anchors[i-1] + max(0, increment[i])`.
  - Interpolates linearly across fixed bucket nodes:
    - MG buckets: `{0, 4, 8, 12, 15}`, with all buckets from 16 to 27 clamped flat to `anchors[4]`.
    - EG buckets: `{0, 3, 6, 9, 12}`, with all buckets from 13 to 27 clamped flat to `anchors[4]`.
  - Total array output: 28 mobility counts (`output[0..27]`).

---

## 2. Mathematical Conditioning & Identifiability
- **Nominal Parameter Count:** 10
- **Effective Numerical Rank:** 10 (Full algebraic rank relative to $10^{-6}$, condition number 3.14).
- **Low Support Parameters (<5% nonzero activation or near-zero variance):** 8 of 10 parameters (80.0%).
  - Only `QueenMobilityMiddleGameIncrement_2` (27.3% active) and `QueenMobilityMiddleGameIncrement_3` (18.7% active) have meaningful corpus variance!
  - `QueenMobilityMiddleGameBase`: active in only 5.8% of positions (mean -0.024, std 0.24).
  - `QueenMobilityMiddleGameIncrement_4`: active in only 2.6% of positions (std 0.16).
  - All 5 EndGame parameters (`QueenMobilityEndGame*`) are active in only 5.7% to 6.9% of positions!
- **Diagnosis:** Queens rarely reach high mobility in quiet positions without immediate tactical fireworks. The middle and high buckets in MG and nearly the entire EG curve are statistically starved in normal play.

---

## 3. Semantic & Conceptual Overlap with Other Families
1. **Queen PieceSquare (Major PST):**
   - Major PST gives $P[0] + P[1]\text{rank} + P[2]\text{rank}^2 + P[3]\text{fileCent}$. A queen in the center automatically has both high file/rank centrality AND the highest possible mobility count.
2. **Attack & KingSafety:**
   - When a queen has high mobility, its ray attacks converge into the opponent's territory, generating `QueenAttack*` threat bonuses and `KingAttackerQueenWeight` / line pressure in KingSafety.
   - High queen mobility is essentially synonymous with queen active infiltration.

---

## 4. Failure Analysis & Causal Evidence
- **Involved in High-Confidence Failures:** 39 of 100 cases (35 of 52 evaluation/mixed cases with $|\Delta| \ge 15\text{ cp}$).
- **Causal Ablation Reversals:** 0 direct reversals when scaling QueenMobility alone.
- **Cluster Representation:** Present across all 5 tactical and positional clusters (`Mobility / Activity Trapping`, `PST Piece Placement Collinearity`, etc.).
- **Mechanism:** In failures where the queen is lured into tactical traps (e.g., snatching pawns or checking on loose squares), high pseudo-legal mobility masks the severe positional vulnerability of the queen. Because QueenMobility is tied into the general mobility score, it contributes to false activity assessments.

---

## 5. Structural Recommendation
- **Status:** **RESHAPE CURRENT REPRESENTATION**
- **Action:**
  - Merge the 5-anchor piecewise interpolation into a smooth 2-parameter or 3-parameter polynomial/sigmoid curve (Base, Linear Slope, and Asymptotic Saturation).
  - Eliminate the 5 independent EG increments: tie EG queen mobility directly to MG mobility via a single phase taper multiplier.
  - Defensible degrees of freedom: 3-4 parameters (down from 10, saving 6 low-support parameters).
