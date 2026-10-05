# Structural Alternatives for RookFile Redesign

## 1. Design Constraints & Objectives
- Eliminate unnecessary degrees of freedom without disrupting the empirical compensation between `RookFile` and `RookMobility`.
- Do NOT arbitrarily flip negative semi-open weights to positive, as this would severely inflate blocked rook activity.
- Preserve full numerical rank and excellent conditioning ($< 2.0$).
- Guarantee 100.000% exact evaluation score identity with baseline.

---

## 2. Alternatives Considered

### Alternative 1: Exact-Preserving Canonical Cleaned Model (4 Parameters) [Recommended]
- **Parameters (4):**
  1. `CandRookOpenFileMiddleGame` = 12
  2. `CandRookOpenFileEndGame` = 2
  3. `CandRookSemiOpenFileMiddleGame` = -4
  4. `CandRookSemiOpenFileEndGame` = -26
- **Rationale:** The 4-parameter formulation is already full rank (4/4) with an exceptionally healthy condition number (1.25). The negative semi-open values are theoretically justified as necessary activity damping terms against `RookMobility`. Retaining the clean representation cleanly preserves 100.000% exact table identity without information loss or search destabilization.
- **Conditioning:** Full rank (4/4), condition number **1.25**.

### Alternative 2: Tied Semi-Open Dampening Model (3 Parameters)
- **Parameters (3):**
  1. `CandRookOpenFileMiddleGame` = 12
  2. `CandRookOpenFileEndGame` = 2
  3. `CandRookSemiOpenDamping` (Tied MG/EG ratio)
- **Pros:** Reduces 1 parameter.
- **Cons:** Constrains phase progression of semi-open files where endgames require distinct damping from middlegames.

### Alternative 3: Explicit Net-Mobility Decoupled Model (2 Parameters)
- **Parameters (2):**
  1. `CandRookOpenFileBonus` = 12
  2. `CandRookSemiOpenFileBonus` = 6
  - Subtracts raw vertical ray mobility directly in MoveLogic.
- **Pros:** Eliminates negative parameter values visually.
- **Cons:** High architectural risk; requires refactoring `MoveLogic.cpp` and breaking evaluator encapsulation.
