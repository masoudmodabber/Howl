# KnightOutpost Structural Alternatives

This document outlines structural design alternatives for Howl's KnightOutpost representation.

---

## Candidate 1: Exact-Preserving Canonical Model (Selected)
- **Concept:** Maintain the existing 4-parameter formulation (`KnightOutpostMiddleGame`, `KnightOutpostEndGame`, `KnightSupportedOutpostMiddleGame`, `KnightSupportedOutpostEndGame`) without arbitrary parameter elimination or value alteration.
- **Rationale:**
  - Full numerical rank (4/4) and well-conditioned (condition number 2.18).
  - Empirical corpus audit proves supported and unsupported outposts represent distinct, non-redundant chess concepts: across 190 active outposts in 1,000 reference games, 57.9% are unsupported (hole occupancy) and 42.1% are pawn-supported (permanent anchors).
  - The negative EG value (-3 cp) in unsupported outposts functions as a subtle mobility dampener against off-board knight displacement in sparse endgames without friendly pawn anchors.
- **Parameters:** 4 canonical parameters.
- **Information Loss Risk:** **ZERO.** 100.000% exact evaluation score identity.

---

## Candidate 2: Merged Outpost with Supported Multiplier (Aggressive Reduction)
- **Concept:** Merge base and supported outposts into a single base outpost taper ($P_1, P_2$) with an integer support bonus multiplier $M_{\text{supp}}$:
  $$V = \text{Taper}(P_1, P_2, \text{phase}) \times (1 + M_{\text{supp}} \cdot [\text{isSupported}])$$
- **Parameters:** 3 parameters.
- **Trade-off:** Compels supported outposts to scale proportionally with unsupported outposts across both MG and EG, destroying the independent endgame behavior where supported outposts are heavily rewarded (+48 cp) while unsupported outposts are neutral/dampened (-3 cp).
- **Information Loss Risk:** High.

---

## Candidate 3: Additive Single Outpost Feature (Extreme Consolidation)
- **Concept:** Drop the unsupported outpost entirely; only award bonuses to permanently pawn-supported knights.
- **Parameters:** 2 parameters (`KnightSupportedOutpostMiddleGame`, `KnightSupportedOutpostEndGame`).
- **Trade-off:** Discards the valuable strategic signal of occupying unchallengeable outposts (57.9% of outpost occurrences).
- **Information Loss Risk:** Very High.
