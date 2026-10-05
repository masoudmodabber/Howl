# Current KingSafety Model Specification & Decomposition

## 1. Overview and Architecture
The Howl evaluator's **KingSafety** family consists of 23 canonical parameters defined in `Option.h` / `Option.cpp` and evaluated in `EvaluationLogic.cpp::EvaluateKingDanger`.
Unlike PieceSquare or Mobility, KingSafety is not a static lookup table; it is a complex, multi-tiered algorithmic model evaluated independently for White's king and Black's king.

The evaluation accumulates:
$$\text{kingDangerNet} = \text{blackKingDanger}.\text{danger} - \text{whiteKingDanger}.\text{danger}$$
which feeds directly into the root evaluation accumulator:
```cpp
int kingSafety = kingDangerNet;
```

---

## 2. Inventory of All 23 Parameters and Their Roles

| Parameter | Baseline | Subsystem | Phase Scope | Type | Formula Role | Chess Meaning |
| :--- | :---: | :--- | :--- | :--- | :--- | :--- |
| `KingAttackerPawnWeight` | 0 | Attacker | Both | Linear | `attackerParticipation += w[1]` | Enemy pawn attacking king zone |
| `KingAttackerMinorWeight` | 9 | Attacker | Both | Linear | `attackerParticipation += w[2/3]` | Enemy minor (N/B) attacking king zone |
| `KingAttackerRookWeight` | 8 | Attacker | Both | Linear | `attackerParticipation += w[4]` | Enemy rook attacking king zone |
| `KingAttackerQueenWeight` | 8 | Attacker | Both | Linear | `attackerParticipation += w[5]` | Enemy queen attacking king zone |
| `KingDefenderPawnWeight` | 12 | Defender | Both | Linear | `defenderParticipation += w[1]` | Friendly pawn defending king zone |
| `KingDefenderMinorWeight` | 108 | Defender | Both | Linear | `defenderParticipation += w[2/3]` | Friendly minor defending king zone |
| `KingDefenderRookWeight` | 6 | Defender | Both | Linear | `defenderParticipation += w[4]` | Friendly rook defending king zone |
| `KingDefenderQueenWeight` | 135 | Defender | Both | Linear | `defenderParticipation += w[5]` | Friendly queen defending king zone |
| `KingShelterSecondRankDanger` | 1 | Shelter | MG | Additive | `shelterDanger += 1` | Shield pawn on 2nd rank from king |
| `KingShelterAdvancedPawnDanger`| 20 | Shelter | MG | Additive | `shelterDanger += 20` | Shield pawn advanced 3+ ranks |
| `KingShelterMissingPawnDanger` | 2 | Shelter | MG | Additive | `shelterDanger += 2` | Completely missing shield pawn |
| `KingShelterOpenFileDanger` | 0 | Shelter | MG | Additive | `shelterDanger += 0` | Open file directly in front of king (Baseline 0) |
| `KingUndefendedZoneDanger` | 4 | Zone | Both | Additive | `zoneDanger += count * 4` | King ring squares attacked by enemy & undefended |
| `KingAdditionalZoneAttackerDanger`| 55| Zone | Both | Additive | `zoneDanger += count * 55` | Additional enemy attackers on undefended ring |
| `KingSemiOpenLineDanger` | 36 | Lines | Both | Additive | `filePressure += 36` | Rook/Queen pressure on semi-open king file |
| `KingOpenLineDanger` | 26 | Lines | Both | Additive | `filePressure += 26` | Rook/Queen pressure on fully open king file |
| `KingDiagonalLineDanger` | 9 | Lines | Both | Additive | `diagonalPressure += 9` | Bishop/Queen ray aligned with king square |
| `KingControlledEscapeDanger` | 22 | Escape | Both | Additive | `escapeDanger += count * 22` | Neighbor squares controlled by enemy |
| `KingBlockedEscapeDanger` | 0 | Escape | Both | Additive | `escapeDanger += count * 0` | Neighbor squares blocked by own pieces (Baseline 0) |
| `KingTrappedEscapeDanger` | 0 | Escape | Both | Additive | `escapeDanger += count * 0` | Total safe escapes < 3 (Baseline 0) |
| `KingHeavyBatteryDanger` | 0 | Battery | MG (>=12) | Additive | `rawDanger += 0` | Queen + Rook mutual battery on undefended zone (Baseline 0) |
| `KingPinnedShelterPawnWeight` | 0 | Pins | Both | Additive | `rawDanger += 0` | Pinned shield pawns (Baseline 0) |
| `KingInfiltratedQueenWeight` | 19 | Infiltration | Both | Additive | `rawDanger += 19` | Enemy queen penetrating to 1st/2nd rank |

---

## 3. Mathematical Conditioning & Empirical Support
- **Canonical Parameters:** 23
- **Effective Numerical Rank:** 23 / 23 (algebraically full rank)
- **Condition Number:** 6.20 (ill-conditioned, highest among active families)
- **Low Support Parameters (<5% active):** 11 of 23 parameters (47.8%)
- **Baseline Zero Parameters:** 6 of 23 parameters (26.1%)
  - `KingAttackerPawnWeight` = 0
  - `KingShelterOpenFileDanger` = 0
  - `KingBlockedEscapeDanger` = 0
  - `KingTrappedEscapeDanger` = 0
  - `KingHeavyBatteryDanger` = 0
  - `KingPinnedShelterPawnWeight` = 0
