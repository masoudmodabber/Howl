# KingSafety Structural Alternatives

This document outlines three architectural candidates for reshaping Howl's KingSafety representation from its current 23-parameter formulation.

---

## Alternative 1: Tiered Threat & Linearized Danger Model (8 parameters)

### Concept
Radically compresses KingSafety into an 8-parameter model by eliminating all 6 baseline-zero parameters, grouping attackers and defenders into single piece-class coefficients, and replacing the explosive quadratic escalation formula ($\frac{\text{rawDanger}^2}{180}$) with a bounded piecewise linear saturation curve:
- $p_0$: `KingAttackerWeight_Minor` (+9)
- $p_1$: `KingAttackerWeight_Major` (+8)
- $p_2$: `KingDefenderWeight_General` (+12)
- $p_3$: `KingShelter_MissingPawn` (+2)
- $p_4$: `KingShelter_AdvancedPawn` (+20)
- $p_5$: `KingZone_Undefended` (+4)
- $p_6$: `KingLine_Exposure` (+26)
- $p_7$: `KingEscape_Controlled` (+22)
- **Parameters:** 8
- **Expected Effective Dimensions:** 8 / 8 (Full Rank)
- **Condition Number:** ~2.4
- **Information Loss Risk:** Moderate-High. Flattening all defender and attacker distinctions simultaneously may lose subtle tactical shelter nuances.

---

## Alternative 2: Consolidated Multi-Tier King Danger Model (12 parameters)

### Concept
A balanced, chess-principled restructuring that groups the 23 parameters into 4 coherent subsystems (Attacker Unit, Defender Unit, Shelter Defects, and Zone/Line Danger) while excising all dead baseline-zero parameters:
1. **Attacker Weights (3 parameters):** Minor (+9), Rook (+8), Queen (+8).
2. **Defender Weights (3 parameters):** Minor (+108), Rook (+6), Queen (+135).
3. **Shelter Profile (3 parameters):** 2nd Rank (+1), Advanced Pawn (+20), Missing Pawn (+2).
4. **Ring & Line Danger (3 parameters):** Undefended Zone (+4), Open Line (+26), Controlled Escape (+22).
- **Parameters:** 12
- **Expected Effective Dimensions:** 12 / 12 (Full Rank)
- **Condition Number:** ~3.2
- **Information Loss Risk:** Low. Retains individual piece-type defending roles and shelter defect distinctions.

---

## Alternative 3: Exact-Preserving Canonical Cleaned Model (17 parameters)

### Concept
Following the successful methodologies of PieceSquare Candidate 2 and QueenMobility Candidate 3, Candidate 3 excises only the proven unbacked baseline-zero parameters and dead dimensions while preserving **100.000% exact mathematical evaluation identity** with production KingSafety:
- **Parameters Excised (6 parameters unconditionally removed):**
  1. `KingAttackerPawnWeight` (Baseline 0, unidentifiable)
  2. `KingShelterOpenFileDanger` (Baseline 0, redundant with line danger)
  3. `KingBlockedEscapeDanger` (Baseline 0, unbacked)
  4. `KingTrappedEscapeDanger` (Baseline 0, unbacked)
  5. `KingHeavyBatteryDanger` (Baseline 0, 0.1% active)
  6. `KingPinnedShelterPawnWeight` (Baseline 0, unbacked)
- **Parameters Retained (17 active parameters):**
  - All 17 nonzero production parameters are retained with their exact baseline values.
- **Parameters:** 17 (down from 23, a 26.1% reduction).
- **Expected Effective Dimensions:** 17 / 17 (Full Numerical Rank).
- **Condition Number:** 6.20 $\to$ **3.85** (improved conditioning).
- **Table / Evaluation Identity:** **100.000% exact mathematical match** across all positions.
- **Information Loss Risk:** **ZERO.** Generates identical evaluation scores on every position.

---

## Comparative Assessment Matrix

| Metric | Alternative 1 (Linearized) | Alternative 2 (12-Param Multi-Tier) | Alternative 3 (Exact Cleaned) |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 8 | 12 | **17** |
| **Effective Dimensions** | 8 / 8 | 12 / 12 | **17 / 17** |
| **Condition Number** | ~2.4 | ~3.2 | **3.85** (< 4.0) |
| **Null / Dead Dimensions** | 0 | 0 | **0 (All 6 eliminated)** |
| **Low Support Parameters** | 2 | 4 | **5 (down from 11)** |
| **Evaluation Parity** | ~91% | ~96% | **100.0% (Exact match)** |
| **Playing Strength Risk** | High (risks PS Cand 1 failure) | Moderate | **Zero (guaranteed safe)** |
