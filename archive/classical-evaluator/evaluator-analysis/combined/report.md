# Combined Evaluator Representation & Failure Analysis Report

## Executive Synthesis

- **Evaluator canonical parameters audited:** 261
- **Evaluator families audited:** 16
- **High-confidence failure positions analyzed:** 100
- **Search vs Evaluation classification:** 31 likely search, 36 likely evaluation, 16 mixed, 17 unresolved
- **Failure clusters discovered:** 6 distinct interpretable clusters (64 dev / 36 held-out)
- **Families with clear mathematical conditioning/redundancy concerns:** 7
- **Families where chess failures overlap with representation concerns:** 3

## Family Classification Matrix

| Family | Params | Rank | Cond No. | Low Supp | Max |r| | Failures | Ablation Reversals | Final Classification |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Attack | 60 | 44 | rank_deficient | 57 | 0.707 | 22 | 5 | **mathematically unhealthy and high observed chess impact** |
| BishopMobility | 10 | 10 | 3.19262786932 | 4 | 0.605 | 39 | 0 | **healthy and useful** |
| EndgameWeights | 5 | 5 | 27.5652632571 | 5 | 0.985 | 0 | 0 | **mathematically unhealthy but low observed chess impact** |
| Inline | 8 | 7 | rank_deficient | 4 | 0.828 | 0 | 0 | **mathematically unhealthy but low observed chess impact** |
| IsolatedPawn | 2 | 2 | 1.58737363929 | 0 | 0.432 | 0 | 0 | **healthy but low impact** |
| KingSafety | 23 | 23 | 6.19679052727 | 11 | 0.737 | 26 | 5 | **healthy and useful** |
| KnightMobility | 8 | 8 | 3.8113807177 | 1 | 0.772 | 39 | 2 | **healthy and useful** |
| KnightOutpost | 4 | 4 | 2.17948257609 | 4 | 0.514 | 0 | 0 | **mathematically unhealthy but low observed chess impact** |
| PassedPawnV2 | 13 | 13 | 6.62718864068 | 5 | 0.834 | 8 | 0 | **healthy but low impact** |
| PawnStructure | 1 | 1 | 1 | 0 | 0.000 | 7 | 0 | **healthy but low impact** |
| PieceSquare | 96 | 94 | rank_deficient | 34 | 0.943 | 34 | 11 | **mathematically unhealthy and high observed chess impact** |
| PieceValue | 5 | 5 | 2.7307297496 | 1 | 0.642 | 12 | 0 | **healthy and useful** |
| QueenMobility | 10 | 10 | 3.13634122521 | 8 | 0.727 | 39 | 0 | **mathematically unhealthy and high observed chess impact** |
| RookBehindPassedPawn | 2 | 2 | 1.3855577734 | 2 | 0.315 | 0 | 0 | **mathematically unhealthy but low observed chess impact** |
| RookFile | 4 | 4 | 1.24983849825 | 2 | 0.183 | 3 | 0 | **healthy but low impact** |
| RookMobility | 10 | 10 | 2.11320783481 | 4 | 0.460 | 39 | 0 | **healthy and useful** |

## Key Architectural Insights for Future Evaluator Redesign

1. **Attack & KingSafety Overlap:** Both terms represent attacker proximity and king zone convergence. Attack is severely rank-deficient (rank 44/60, 57 low-support terms), while KingSafety is directly implicated in 17 high-confidence blunder positions where 16 causal reversals occurred upon scaling down.
2. **PieceSquare Table Collinearity:** High rank deficiency (94/96) and extreme internal collinearity (e.g. Rook EG table elements r = 0.943). Distorts piece placement independently of mobility.
3. **EndgameWeights Collinearity:** Condition number 27.57 with 4 pairs exceeding |r| >= 0.90 (LoneKingEdgeWeight vs LoneKingCornerWeight r = -0.985). High mathematical corruption, though lower middle-game chess failure impact.
4. **Mobility vs Outpost Duplication:** Pseudo-legal move counts inflate trapped minor pieces, overlapping directly with PST and KnightOutpost features.

