# Evaluator Parameter Geometry & Identifiability Audit

## Executive Summary
- **Total canonical parameters audited:** 261
- **Total evaluator families audited:** 16
- **Rank-deficient families:** 3 (Attack, Inline, PieceSquare)
- **Total low-support parameters (<5% active or near-zero variance):** 142 / 261 (54.4%)
- **High internal collinearity pairs (|r| >= 0.90):** 10
- **Semantic overlap groups identified:** 6

## Family Identifiability Summary

| Family | Params | Rank | Condition No. | Low Support | Max |r| | Classification |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Attack | 60 | 44 | rank_deficient | 57 | 0.707 | poorly identified |
| BishopMobility | 10 | 10 | 3.19 | 4 | 0.605 | partially identified |
| EndgameWeights | 5 | 5 | 27.57 | 5 | 0.985 | poorly identified |
| Inline | 8 | 7 | rank_deficient | 4 | 0.828 | poorly identified |
| IsolatedPawn | 2 | 2 | 1.59 | 0 | 0.432 | well identified |
| KingSafety | 23 | 23 | 6.20 | 11 | 0.737 | partially identified |
| KnightMobility | 8 | 8 | 3.81 | 1 | 0.772 | partially identified |
| KnightOutpost | 4 | 4 | 2.18 | 4 | 0.514 | poorly identified |
| PassedPawnV2 | 13 | 13 | 6.63 | 5 | 0.834 | partially identified |
| PawnStructure | 1 | 1 | 1.00 | 0 | 0.000 | well identified |
| PieceSquare | 96 | 94 | rank_deficient | 34 | 0.943 | partially identified |
| PieceValue | 5 | 5 | 2.73 | 1 | 0.642 | partially identified |
| QueenMobility | 10 | 10 | 3.14 | 8 | 0.727 | poorly identified |
| RookBehindPassedPawn | 2 | 2 | 1.39 | 2 | 0.315 | poorly identified |
| RookFile | 4 | 4 | 1.25 | 2 | 0.183 | poorly identified |
| RookMobility | 10 | 10 | 2.11 | 4 | 0.460 | partially identified |

## Severe Collinearity Highlights

- **EndgameWeights**: `LoneKingBase` vs `LoneKingRestrictedNeighbourWeight` (r = -0.9296, >=0.90)
- **EndgameWeights**: `LoneKingEdgeWeight` vs `LoneKingCornerWeight` (r = -0.9848, >=0.97)
- **EndgameWeights**: `LoneKingEdgeWeight` vs `LoneKingRestrictedNeighbourWeight` (r = 0.9354, >=0.90)
- **EndgameWeights**: `LoneKingCornerWeight` vs `LoneKingRestrictedNeighbourWeight` (r = -0.9375, >=0.90)
- **PieceSquare**: `PawnPieceSquareMiddleGame_1` vs `PawnPieceSquareMiddleGame_2` (r = 0.9044, >=0.90)
- **PieceSquare**: `KnightPieceSquareMiddleGame_2` vs `KnightPieceSquareMiddleGame_3` (r = 0.9141, >=0.90)
- **PieceSquare**: `RookPieceSquareMiddleGame_1` vs `RookPieceSquareMiddleGame_2` (r = 0.9272, >=0.90)
- **PieceSquare**: `PawnPieceSquareEndGame_1` vs `PawnPieceSquareEndGame_2` (r = 0.9094, >=0.90)
- **PieceSquare**: `RookPieceSquareEndGame_1` vs `RookPieceSquareEndGame_2` (r = 0.9427, >=0.90)
- **PieceSquare**: `QueenPieceSquareEndGame_1` vs `QueenPieceSquareEndGame_2` (r = 0.9115, >=0.90)

## Semantic Overlap Groups

### King Attack vs King Safety vs Pawn Shield
- **Families:** `Attack` & `KingSafety`
- **Terms:** `KnightAttackValue, BishopAttackValue, RookAttackValue, QueenAttackValue` vs `KingAttacker[Pawn/Minor/Rook/Queen]Weight, KingDefenderWeights, KingShelterDanger`
- **Source Reference:** `EvaluationLogic.cpp:2150-2220, 2600-2980; KingSetup.cpp`
- **Mechanism:** King zone pressure and attacker count are counted in both Attack (PieceMoveCountFast) and KingSafety (EvaluateKingDanger). Both evaluate attacking pieces converging on the opponent king.

### PST King Placement vs King Safety Shelter
- **Families:** `PieceSquare` & `KingSafety`
- **Terms:** `KingInValueWhiteMiddleGame, WhiteKingPlaceSafetyMiddleGame` vs `KingShelterSecondRankDanger, KingShelterMissingPawnDanger, KingShelterOpenFileDanger`
- **Source Reference:** `EvaluationLogic.cpp:2162-2206`
- **Mechanism:** King PST penalizes or rewards king squares based on castling destinations (g1/c1 vs e1/d1), while KingSafety independently scores pawn shelter defects, open files, and missing shield pawns on those exact same destination files.

### Mobility vs PieceSquare vs Outpost
- **Families:** `KnightMobility / BishopMobility / RookMobility` & `PieceSquare / KnightOutpost`
- **Terms:** `KnightMoveCountValue, BishopMoveCountValue, RookMoveCountValue` vs `KnightPieceSquare, BishopPieceSquare, KnightOutpost[MiddleGame/EndGame]`
- **Source Reference:** `EvaluationLogic.cpp:2650-2710, 2828-2890; Option.cpp`
- **Mechanism:** Centralizing a minor piece simultaneously triggers higher PST values, higher available pseudo-legal move counts (Mobility), and KnightOutpost bonuses if placed on advanced protected squares.

### Rook Open File vs Rook Mobility vs Rook PST
- **Families:** `RookFile` & `RookMobility / PieceSquare`
- **Terms:** `RookOpenFileMiddleGame, RookSemiOpenFileMiddleGame` vs `RookMoveCountValue, RookPieceSquareMiddleGame`
- **Source Reference:** `EvaluationLogic.cpp:2154-2156, 2730-2750, 2900-2908`
- **Mechanism:** An open or semi-open file provides open vertical rays, which directly elevates RookMoveCountValue popcounts, yields explicit RookFile bonuses, and overlaps with 7th/8th rank PST placements.

### Passed Pawn Advancement vs Endgame Scaling
- **Families:** `PassedPawnV2` & `EndgameWeights / Inline`
- **Terms:** `PassedPawnMiddleGameIncrement, PassedPawnEndGameIncrement` vs `EndgamePawnAdvancementRankMultiplier, OppositeColorBishopScale`
- **Source Reference:** `EvaluationLogic.cpp:2230-2246, 580-606`
- **Mechanism:** Passed pawn rank increments are scaled nonlinearly in PassedPawnV2, while endgame evaluation independently applies advancement multipliers and taper curves.

### Lone King Mate Confinement vs Edge/Corner
- **Families:** `EndgameWeights` & `EndgameWeights`
- **Terms:** `LoneKingEdgeWeight, LoneKingCornerWeight` vs `LoneKingRestrictedNeighbourWeight, LoneKingBase`
- **Source Reference:** `EvaluationLogic.cpp:2386-2392; family-correlations.tsv`
- **Mechanism:** Internal family collinearity (|r| > 0.98): LoneKingEdgeWeight and LoneKingCornerWeight have r = -0.985, and both collinear with RestrictedNeighbourWeight (r = 0.935). All represent geometric proximity of the lone king to board boundaries.

