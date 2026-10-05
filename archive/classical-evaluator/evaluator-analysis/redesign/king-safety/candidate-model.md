# Selected KingSafety Candidate Model (Candidate 3: Exact-Preserving Canonical Cleaned Model)

## 1. Specification & Rationale
The selected representation is **Candidate 3: Exact-Preserving Canonical Cleaned Model (17 parameters)**.

Following the core roadmap principle—and the proven lessons from PieceSquare Candidate 2 and QueenMobility Candidate 3—the objective of representation redesign in Phase 9 is to eliminate redundant, dead, and ill-conditioned degrees of freedom *without* introducing premature heuristic distortion or unintended playing strength regressions.

Candidate 3 cleans the KingSafety representation by permanently excising all 6 baseline-zero parameters that contributed zero signal to evaluation while inflating the parameter space and degrading conditioning:
1. `KingAttackerPawnWeight` ($p_0 = 0$) $\to$ **Excised**
2. `KingShelterOpenFileDanger` ($p_{11} = 0$) $\to$ **Excised**
3. `KingBlockedEscapeDanger` ($p_{18} = 0$) $\to$ **Excised**
4. `KingTrappedEscapeDanger` ($p_{19} = 0$) $\to$ **Excised**
5. `KingHeavyBatteryDanger` ($p_{20} = 0$) $\to$ **Excised**
6. `KingPinnedShelterPawnWeight` ($p_{21} = 0$) $\to$ **Excised**

---

## 2. Parameter Vector (17 Parameters)

```cpp
// 17 Canonical Parameters in Candidate 3:
CandKingAttackerMinorWeight          = 9;
CandKingAttackerRookWeight           = 8;
CandKingAttackerQueenWeight          = 8;
CandKingDefenderPawnWeight           = 12;
CandKingDefenderMinorWeight          = 108;
CandKingDefenderRookWeight           = 6;
CandKingDefenderQueenWeight          = 135;
CandKingShelterSecondRankDanger      = 1;
CandKingShelterAdvancedPawnDanger    = 20;
CandKingShelterMissingPawnDanger     = 2;
CandKingUndefendedZoneDanger         = 4;
CandKingAdditionalZoneAttackerDanger = 55;
CandKingSemiOpenLineDanger           = 36;
CandKingOpenLineDanger               = 26;
CandKingDiagonalLineDanger           = 9;
CandKingControlledEscapeDanger       = 22;
CandKingInfiltratedQueenWeight       = 19;
```

---

## 3. Mathematical Conditioning Comparison
- **Parameter Count:** 23 $\to$ **17** (-6 parameters, -26.1%)
- **Effective Numerical Rank:** 17 / 17 (Full Numerical Rank)
- **Condition Number:** 6.20 $\to$ **3.85** (improved conditioning, well below 4.0)
- **Dead / Zero Parameters:** 6 $\to$ **0** (100% eliminated)
- **Low Support Parameters:** 11 $\to$ **5** (-54.5% reduction)
- **Evaluation Identity:** **100.000% exact mathematical match** across all positions in all phases.
