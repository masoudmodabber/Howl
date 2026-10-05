# Attack Representation Alternatives

## 1. Candidate 1: Tiered Threat Matrix with Tapered Scaling (Recommended)
- **Concept:** Replace the $6 \times 5 \times 2$ Cartesian grid with a compact matrix based on attacker piece tier and victim piece tier, plus an endgame threat taper scalar.
  - **Attacker Tiers (4):**
    1. Pawn
    2. Minor (Knight & Bishop merged)
    3. Rook
    4. Queen
    *(King attacker deleted as structurally unreachable)*
  - **Victim Tiers (3):**
    1. Pawn
    2. Minor (Knight & Bishop merged)
    3. Major (Rook & Queen)
  - **Tuning / Storage:**
    - Authored MiddleGame table: 10 parameters.
    - Single EndGame Taper Scaling Factor (or 2-phase weights for active terms): 1 parameter (or 10 paired EG parameters).
    - In the standard compact model: **10 canonical parameters**.
- **Parameter Breakdown:**
  1. `Threat_PawnOnMinor_MG` (Mapped baseline: 20)
  2. `Threat_PawnOnMajor_MG` (Mapped baseline: 84)
  3. `Threat_MinorOnPawn_MG` (Mapped baseline: 7)
  4. `Threat_MinorOnMinor_MG` (Mapped baseline: 24)
  5. `Threat_MinorOnMajor_MG` (Mapped baseline: 41)
  6. `Threat_RookOnPawn_MG` (Mapped baseline: -1)
  7. `Threat_RookOnMinor_MG` (Mapped baseline: 15)
  8. `Threat_RookOnQueen_MG` (Mapped baseline: 24)
  9. `Threat_QueenOnPawn_MG` (Mapped baseline: 3)
  10. `Threat_QueenOnPiece_MG` (Mapped baseline: 10)
  11. `Threat_EndgameScaleFactor` (Mapped baseline: 150%)
- **Retained Information:**
  - Full tactical differentiation between piece values.
  - Distinct pawn harassment vs major piece pinning.
  - Hanging piece multiplier preserved intact.
- **Removed Information:**
  - 10 structurally dead King attacker parameters.
  - Pawn attacking pawn (handled by pawn structure).
  - Knight vs Bishop asymmetric victim subtleties that had zero statistical separation.
  - 30 separate EndGame parameters with near-zero independent support.
- **KingSafety Relationship:** Cleans up minor piece overvaluation, decoupling static threats from king zone convergence.
- **Expected Effective Dimensions:** 10 / 11 (Full rank, condition number < 5.0).
- **Risk:** Very low. Preserves all active chess threat semantics while removing collinear deadwood.

---

## 2. Candidate 2: Threat Class + Hanging Scalar (Aggressive Consolidation)
- **Concept:** Group attacks purely by value exchange class:
  1. `FavorableThreat` (Attacker value < Victim value: Pawn on piece, Minor on major, Rook on Queen)
  2. `EqualThreat` (Attacker value == Victim value: Minor on Minor, Rook on Rook, Queen on Queen)
  3. `UnfavorableHarassment` (Attacker value > Victim value: Major on minor, Queen on pawn)
  4. `PawnTargetPressure` (Piece attacking weak pawn)
  5. `HangingPieceMultiplier`
- **Parameter Count:** 5 parameters.
- **Retained Information:** Coarse tactical trade direction.
- **Removed Information:** Fine-grained piece interactions (e.g. Pawn on Queen vs Minor on Queen).
- **Expected Effective Dimensions:** 4 / 5.
- **KingSafety Relationship:** Completely separates tactical exchange potential from positional king attack.
- **Risk:** Moderate-High. Losing distinction between Pawn-on-Queen (+86) and Pawn-on-Knight (+20) may cause Howl to mishandle major piece trapping.

---

## 3. Candidate 3: Pruned 16-Parameter Direct Matrix
- **Concept:** Keep the piece-by-piece matrix for Pawn, Minor, Rook, Queen against Pawn, Minor, Rook, Queen, but remove King attackers, eliminate equal piece self-attacks (N-on-N, B-on-B, R-on-R, Q-on-Q = 0), merge Knight and Bishop attackers into a unified Minor attacker row, and tie EG to MG via a fixed 1.5x endgame multiplier.
- **Parameter Count:** 14 parameters (3 Pawn targets + 4 Minor targets + 3 Rook targets + 4 Queen targets).
- **Retained Information:** Explicit attacker-to-victim mapping for all non-king pieces.
- **Removed Information:** King attackers, self-attacks, separate EG parameters.
- **Expected Effective Dimensions:** 12.
- **Risk:** Low, but slightly more degrees of freedom than Candidate 1 without distinct chess benefits.

---

## Conclusion and Selected Candidate
**Candidate 1 (Tiered Threat Matrix, 10-11 parameters)** is selected for implementation and validation. It captures 100% of the active conceptual chess degrees of freedom, removes all 16 null/dead parameters, and replaces 30 unobservable EndGame parameters with structured tapering.
