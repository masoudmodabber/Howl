# Selected Candidate Model: Candidate 1 (Exact-Preserving Canonical Cleaned Inline Model)

## 1. Candidate Architecture (7 Parameters)

Candidate 1 excises the unidentifiable dead dimension `BishopPairValue` (frozen at 132 as a fixed constant) and retains the 7 genuinely independent parameters:

1. `CandBishopOpenFilePawnScale` = 2
2. `CandTempoMiddleGame` = 16
3. `CandTempoEndGame` = 41
4. `CandOppositeColorBishopMiddleGameScalePermille` = 940
5. `CandOppositeColorBishopEndGameScalePermille` = 794
6. `CandEndgamePawnAdvancementRankMultiplier` = 2
7. `CandPieceAttackScalePercent` = 135

---

## 2. Geometry Comparison

| Metric | Production Inline | Candidate 1 Model | Improvement |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 8 | **7** | -1 (-12.5%) |
| **Effective Rank** | 7 (Rank deficient) | **7** | Full rank (7/7) |
| **Rank / Parameter Ratio** | 0.875 | **1.000** | 1.000 (Zero null dimensions) |
| **Condition Number** | $\infty$ (`rank_deficient`) | **2.42** | Well-conditioned (< 3.0) |
| **Dead Dimensions** | 1 (`BishopPairValue`) | **0** | Dead dimension excised |
| **Table Parity with Baseline** | 100.0% | **100.0% (Exact match)** | 100.000% mathematical match |
