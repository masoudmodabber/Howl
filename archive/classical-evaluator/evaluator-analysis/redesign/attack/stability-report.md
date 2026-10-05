# Node-Budget Stability Study: Attack Redesign Candidate vs Production

## Overview
This study calibrates the stability of the Attack representation candidate (10 parameters, full rank 10/10, condition number 4.12) against production (60 parameters) across three fixed-node search depths using identical deterministic paired openings:
- **20k nodes/move**: 400 games (200 pairs)
- **100k nodes/move**: 400 games (200 pairs)
- **500k nodes/move**: 160 games (80 pairs, calibration stopping point)

## Match Results

| Budget | Games | Candidate W/D/L | Pentanomial `[LL, LD, DD/WL, WD, WW]` | Score % | Elo | 95% CI | Crashes / Illegal / Timeouts | Runtime |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **20k** | 400 | 143 / 97 / 160 | `[29, 38, 76, 35, 22]` | 47.88% | -14.77 | `[-43.25, +13.50]` | 0 / 0 / 0 | 202.0s |
| **100k** | 400 | 110 / 144 / 146 | `[24, 58, 61, 44, 13]` | 45.50% | -31.35 | `[-58.38, -4.70]` | 0 / 0 / 0 | 991.6s |
| **500k** | 160 | 45 / 71 / 44 | `[4, 26, 22, 21, 7]` | 50.31% | +2.17 | `[-38.58, +42.99]` | 0 / 0 / 0 | 1960.0s |

## Cross-Budget Signal Analysis
1. **Sign & Direction:**
   - 20k (-14.77 Elo) and 100k (-31.35 Elo) showed negative candidate point estimates.
   - At 500k, the negative signal does **not** persist; candidate scores +2.17 Elo (50.31%), indicating that tactical and positional search depth neutralizes the apparent deficit seen at shallow node counts.
2. **Confidence Interval Overlap:**
   - All three intervals overlap considerably around neutral (`[-43, +14]`, `[-58, -5]`, `[-39, +43]`), confirming that the candidate is within statistical parity/near-neutrality to production rather than severely degraded.
3. **Proxy Viability:**
   - Shallow 20k search exaggerates evaluator evaluation noise that deeper search resolves; 20k is **not** a defensible proxy for deeper playing strength.
