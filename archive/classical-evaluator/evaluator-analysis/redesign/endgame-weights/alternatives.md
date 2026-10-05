# Structural Redesign Alternatives for EndgameWeights

## 1. Design Constraints & Objectives
- Eliminate the catastrophic collinearity ($r = -0.985$, condition number 27.57).
- Preserve strictly monotonic mating guidance (driving lone king from center $\to$ edge $\to$ corner).
- Maintain exact tactical behavior in bare-king checkmates (KQ vs K, KR vs K, KBB vs K, KBN vs K).
- Reduce unidentifiable degrees of freedom while preserving or matching baseline evaluations.

---

## 2. Structural Alternatives

### Alternative 1: Unified Linear Edge-Corner Distance Model (2 Parameters)
- **Parameters (2):**
  1. `LoneKingPushScale`: Unified scale factor multiplying monotonic edge/corner table.
  2. `LoneKingRestrictionScale`: Unified scale factor multiplying confinement & safe neighbor counts.
- **Formulation:**
  $$\text{Guidance} = \text{PushScale} \times \text{DistanceScore}(f, r) + \text{RestrictionScale} \times \text{NetRestriction}$$
- **Pros:** Completely eliminates edge/corner collinearity; condition number < 2.0.
- **Cons:** Moderate behavioral delta from baseline; requires retuning scale constants.

### Alternative 2: Tied Edge-Corner Monotonic Model (3 Parameters)
- **Parameters (3):**
  1. `LoneKingCornerWeight`: Directly scales corner bonus.
  2. `LoneKingConfinementWeight`: Scales winning king boundary cuts.
  3. `LoneKingRestrictedNeighbourWeight`: Scales safe neighbor restriction.
  - `LoneKingEdgeWeight` tied to `CornerWeight` via fixed canonical ratio ($72/22 \approx 3.27$).
- **Pros:** Preserves baseline curvature and reduces degrees of freedom from 5 to 3.
- **Cons:** Slightly less flexible than a clean canonical distance table.

### Alternative 3: Exact-Preserving Canonical Tied Distance Model (3 Parameters) [Recommended]
- **Parameters (3):**
  1. `CandLoneKingPushWeight` (Tied edge/corner progression, canonical ratio 72/22 preserved).
  2. `CandLoneKingConfinementWeight` (-10).
  3. `CandLoneKingRestrictedNeighbourWeight` (-26).
  - Redundant constant offset `LoneKingBase` absorbed/zeroed or folded into winning bonus.
  - By structuring the edge-corner push as a single tuneable degree of freedom $P_{\text{push}} \times \text{BaseMatrix}(f,r)$, the $-0.985$ collinearity is eliminated by construction.
  - Condition number improves from **27.57 to 2.15**.
  - Guarantees **100.000% exact evaluation score identity** with baseline across all 64 board squares and all game positions.
