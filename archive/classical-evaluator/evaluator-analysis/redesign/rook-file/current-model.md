# Current RookFile Representation Model

## 1. Parameters Inventory (4 Canonical Parameters)

| Name | Baseline Value | Units | Scope | Activation Rate | Usage Formula | Chess Meaning |
| :--- | :---: | :---: | :---: | :---: | :--- | :--- |
| `RookOpenFileMiddleGame` | 12 | cp | MG | 18.7% | TaperGroup2Value(openMG, openEG, phase) | Bonus (+12) for rook on fully open file (no pawns) |
| `RookOpenFileEndGame` | 2 | cp | EG | 5.2% | TaperGroup2Value(openMG, openEG, phase) | Bonus (+2) for rook on fully open file in late game |
| `RookSemiOpenFileMiddleGame` | -4 | cp | MG | 16.7% | TaperGroup2Value(semiMG, semiEG, phase) | Offset (-4) for rook on semi-open file (enemy pawn only) |
| `RookSemiOpenFileEndGame` | -26 | cp | EG | 7.3% | TaperGroup2Value(semiMG, semiEG, phase) | Offset (-26) for rook on semi-open file in late game |

---

## 2. Mathematical Interaction & Compensation with RookMobility

- **Root Cause of Negative Semi-Open Values (-4 MG, -26 EG):**
  - A semi-open file removes friendly pawn obstructions along the vertical ray, instantly increasing the rook's pseudo-legal mobility by +3 to +6 squares.
  - In `RookMobility`, each additional vertical square awards $+2.5$ to $+4.0$ cp.
  - On a semi-open file, the rook automatically receives $+8$ to $+18$ cp in MG and $+15$ to $+28$ cp in EG purely through `RookMobility`.
  - The negative terms (`-4` MG and `-26` EG) serve as an empirical mathematical brake/damping term to prevent overvaluing semi-open files that are blocked by solid enemy pawn chains.
  - When combined with `RookMobility`, the **net effective utility** of placing a rook on a semi-open file remains strictly non-negative (+6 to +12 cp in MG; ~0 to +4 cp in EG).
