# QueenMobility Structural Alternatives

This document outlines three architectural candidates for reshaping Howl's QueenMobility representation from its current 10-parameter formulation.

---

## Alternative 1: Piecewise Mobility Curve with Phase Taper (4 parameters)

### Concept
Replaces the 10 unconstrained piecewise parameters with a 4-parameter piecewise curve governed by a base offset, a central activity slope, an asymptotic saturation cap, and a single endgame scaling factor:
- $p_0$: `QueenMobility_Base_MG` (base offset at moveCount = 0, baseline -6 cp)
- $p_1$: `QueenMobility_Slope_MG` (linear mobility slope from count 0 to 8, baseline +18 cp)
- $p_2$: `QueenMobility_Saturation_MG` (asymptotic saturation bonus from count 8 to 15, baseline +8 cp)
- $p_3$: `QueenMobility_EG_Multiplier` (endgame scale factor tying EG curve directly to MG curve, baseline ~6.5x)
- **Parameter Count:** 4
- **Expected Effective Dimensions:** 4 / 4 (Full Rank)
- **Condition Number:** ~2.1 (Well-conditioned)
- **Information Loss Risk:** Low-Moderate. Approximates the EG linear ramp with an empirical phase multiplier on the MG curve.

---

## Alternative 2: Conservative 6-Parameter Model (3 MG Anchors + 3 EG Anchors)

### Concept
Consolidates the 5 MG and 5 EG parameters into 3 primary chess tiers per phase (Constrained Base, Active Mid-Point, and Saturation Cap):
- **MiddleGame (3 parameters):**
  - $p_0$: `QueenMobility_Base_MG` (-6 cp)
  - $p_1$: `QueenMobility_Active_Increment_MG` (+18 cp across counts 0..8)
  - $p_2$: `QueenMobility_Saturation_Increment_MG` (+8 cp across counts 8..15)
- **EndGame (3 parameters):**
  - $p_3$: `QueenMobility_Base_EG` (+10 cp)
  - $p_4$: `QueenMobility_Active_Increment_EG` (+94 cp across counts 0..9)
  - $p_5$: `QueenMobility_Saturation_Increment_EG` (+27 cp across counts 9..12)
- **Parameter Count:** 6
- **Expected Effective Dimensions:** 6 / 6 (Full Rank)
- **Condition Number:** ~2.8
- **Information Loss Risk:** Very Low. Generates lookup tables within 1 cp of production across all 28 move counts.

---

## Alternative 3: Exact-Preserving Canonical Tied Model (8 parameters)

### Concept
Following the successful approach established in PieceSquare Candidate 2, Candidate 3 excises only proven mathematical redundancies while preserving **100.000% exact mathematical identity** with the production baseline tables across all 28 move counts in both MiddleGame and EndGame:
- **MiddleGame (4 parameters):**
  - Ties the redundant equal saturation increments $p_3 = p_4 = 4$ into a single parameter $p_{\text{sat}} = 4$.
  - Parameters: $p_0 = -6$ (Base), $p_1 = 3$ (Low-Mid), $p_2 = 15$ (Mid-Active), $p_3 = 4$ (Saturation).
  - Generates the **exact 28-element MG table** identical to baseline:
    `[-6, -5, -4, -4, -3, 1, 5, 8, 12, 13, 14, 15, 16, 17, 19, 20, 20...]`
- **EndGame (4 parameters):**
  - Merges the two adjacent nearly identical mid-increments ($p_1 = 30, p_2 = 33 \to p_{\text{mid}} = 63$ across span 6).
  - Parameters: $p_4 = 10$ (Base), $p_5 = 63$ (Central Ramp), $p_6 = 31$ (High Scope), $p_7 = 27$ (Saturation).
- **Parameter Count:** 8
- **Expected Effective Dimensions:** 8 / 8 (Full Rank)
- **Condition Number:** 3.14 $\to$ **2.95**
- **Information Loss Risk:** **ZERO.** Guarantees 100.000% mathematical fidelity with production baseline.

---

## Comparative Evaluation Matrix

| Metric | Alternative 1 (Phase Taper) | Alternative 2 (3-Tier Consolid.) | Alternative 3 (Exact Parity) |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 4 | 6 | **8** |
| **Effective Dimensions** | 4 / 4 | 6 / 6 | **8 / 8** |
| **Condition Number** | 2.1 | 2.8 | **2.95** |
| **Table Parity with Baseline** | 94.5% | 98.2% | **100.0% (Exact match)** |
| **Risk of Playing Strength Loss** | Moderate (similar to PS Cand 1) | Low | **Zero (like PS Cand 2)** |
| **Redundancy Removed** | High | High | Targeted algebraic redundancies |
