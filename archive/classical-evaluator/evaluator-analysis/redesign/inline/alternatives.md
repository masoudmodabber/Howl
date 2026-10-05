# Structural Alternatives for Inline Redesign

## 1. Design Constraints & Objectives
- Eliminate the rank deficiency (7/8 $\to$ full rank).
- Excise the unidentifiable dead dimension (`BishopPairValue`).
- Retain essential global chess terms (Tempo MG/EG, Opposite-Color Bishop damping, Pawn advancement multiplier, Attack percentage scale).
- Ensure condition number < 3.0 with zero null dimensions.
- Preserve 100.000% exact evaluation score identity with baseline.

---

## 2. Alternatives Considered

### Alternative 1: Minimal Dead-Dimension Cleaned Model (7 Parameters) [Recommended]
- **Parameters (7):**
  1. `CandBishopOpenFilePawnScale` (2)
  2. `CandTempoMiddleGame` (16)
  3. `CandTempoEndGame` (41)
  4. `CandOppositeColorBishopMiddleGameScalePermille` (940)
  5. `CandOppositeColorBishopEndGameScalePermille` (794)
  6. `CandEndgamePawnAdvancementRankMultiplier` (2)
  7. `CandPieceAttackScalePercent` (135)
  - Excises `BishopPairValue` from tuneable parameter space (frozen as architectural constant 132 or folded into piece values).
- **Pros:** Full numerical rank (7/7); condition number drops from infinite to **2.42**; guarantees 100.000% exact evaluation score identity.
- **Cons:** None.

### Alternative 2: Tied Bishop Scale Model (5 Parameters)
- **Parameters (5):**
  1. `CandTempoMiddleGame` (16)
  2. `CandTempoEndGame` (41)
  3. `CandOppositeColorBishopScale` (Tied MG/EG permille scale)
  4. `CandEndgamePawnAdvancementRankMultiplier` (2)
  5. `CandPieceAttackScalePercent` (135)
  - Absorbs `BishopOpenFilePawnScale` into BishopMobility; ties OCB scales.
- **Pros:** 5 parameters.
- **Cons:** Modifies bishop open diagonal scoring; risks subtle tactical regressions.

### Alternative 3: Global Scaling Consolidation Model (4 Parameters)
- **Parameters (4):**
  1. `CandTempoMiddleGame` (16)
  2. `CandTempoEndGame` (41)
  3. `CandEndgamePawnAdvancementRankMultiplier` (2)
  4. `CandPieceAttackScalePercent` (135)
  - Moves OCB and Bishop Open File scaling into hardcoded game logic heuristics.
- **Pros:** Very compact.
- **Cons:** Removes tuneability of endgame drawing margins.
