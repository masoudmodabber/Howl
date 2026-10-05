# Current Inline Representation Model

## 1. Parameters Inventory (8 Canonical Parameters)

| Name | Baseline Value | Scope | Activation Rate | Usage Formula | Chess Meaning |
| :--- | :---: | :---: | :---: | :--- | :--- |
| `BishopPairValue` | 132 | Both/Scalar | 0.0% (effective) | `max(0, BishopPairValue - 2 - totalPawns * 3)` | Baseline bishop pair bonus (saturates symmetrically; dead gradient) |
| `BishopOpenFilePawnScale` | 2 | Both/Scalar | 30.8% | `(8 - totalPawns) * BishopOpenFilePawnScale` | Per-bishop bonus scaled by open files/pawn absence |
| `TempoMiddleGame` | 16 | MG | 79.9% | Side-to-move bonus tapered by phase | Middle-game tempo advantage (+16 cp) |
| `TempoEndGame` | 41 | EG | 24.0% | Side-to-move bonus tapered by phase | End-game tempo advantage (+41 cp) |
| `OppositeColorBishopMiddleGameScalePermille` | 940 | MG | 0.2% | Phase-tapered global multiplier on evaluation | Permille scale (0.94) damping drawish OCBI positions in MG |
| `OppositeColorBishopEndGameScalePermille` | 794 | EG | 0.8% | Phase-tapered global multiplier on evaluation | Permille scale (0.794) damping drawish OCBI positions in EG |
| `EndgamePawnAdvancementRankMultiplier` | 2 | Both/Scalar | 26.7% | `rank * Multiplier` in passed pawn taper | Advanced rank scaling for passed pawns in late game |
| `PieceAttackScalePercent` | 135 | Both/Scalar | 0.8% | `(attackNet * Scale) / 100` | Percentage multiplier scaling tactical threat evaluations |

---

## 2. Mathematical Pathology & Audit

- **Condition Number:** **Infinite (`rank_deficient`)**.
- **Effective Numerical Rank:** **7 / 8** (1 completely dead dimension: `BishopPairValue`).
- **Low-Support Count:** **4 / 8** parameters fire in $<1\%$ of corpus positions (`OppositeColorBishopMiddleGameScalePermille`, `OppositeColorBishopEndGameScalePermille`, `PieceAttackScalePercent`, `BishopPairValue`).
- **Internal & Cross-Family Overlap:**
  - `BishopOpenFilePawnScale` ($r = 0.828$) duplicates `BishopMobility` (which directly measures open diagonals) and `PawnStructure` (total pawn count).
  - `OppositeColorBishop` permille scaling acts as a global damping factor on the entire evaluation score, overlapping with `PieceValue` and endgame draw detection.
  - `BishopPairValue` is algebraically null in the gradient subspace because both sides rarely possess unequal bishop pairs with identical pawn counts, and its linear offset is absorbed by piece value baselines.
