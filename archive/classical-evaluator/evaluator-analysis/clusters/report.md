# Interpretable Failure Clustering & Held-Out Splits

## Overview
- **Total clusters identified:** 6
- **Total positions partitioned:** 100
- **Development partition (65%):** 64
- **Held-out validation partition (35%):** 36

## Failure Clusters

| Cluster Name | Cases | Dev | Held-Out | Primary Mechanism |
| :--- | :--- | :--- | :--- | :--- |
| Search Selective Pruning Overhang | 31 | 20 | 11 | Evaluator favored reference move or neutral, but search pruned or failed to see refutation |
| Mobility / Activity Trapping | 20 | 13 | 7 | Pseudo-legal move counts inflated tactical squares into phantom advantages |
| Positional Pawn Structure Distortion | 11 | 7 | 4 | Pawn structure chain / isolated pawn terms created false positional imbalances |
| Endgame Scaling & Minor Piece Conversion | 2 | 1 | 1 | Sparse piece endgame where incorrect scaling or boundary geometry misvalued position |
| King Exposure & Attack Overvaluation | 17 | 11 | 6 | Evaluator overvalued phantom king attack or penalized sound king placement |
| PST Piece Placement Collinearity | 19 | 12 | 7 | Static square bonuses overrode piece coordination and positional harmony |
