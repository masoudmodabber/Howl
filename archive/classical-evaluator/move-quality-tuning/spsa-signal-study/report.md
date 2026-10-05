# Howl classic SPSA signal study: Corrected Independent Evaluation

## Executive Summary & Viability Determination

**Determination: Stockfish-style classic SPSA tuning across 261 evaluator parameters is NOT statistically viable on a single local machine.**

While local fixed-node throughput is mechanically sound (~24 games/sec at 1,000 nodes/move on 16 threads), the statistical signal of simultaneous 261-parameter SPSA perturbation is swamped by noise:
1. **Low Gradient Sign Stability:** At the selected perturbation scale (c = 0.015625 * c0), sign agreement between smaller pair counts and 64 pairs was **40.0% at 8 pairs** (worse than a coin flip), **55.0% at 16 pairs**, and **70.0% at 32 pairs**. Even at 64 pairs, pair-level bootstrap 95% confidence intervals span 0 for 18 out of 20 tested directions (e.g., D = -3.0, 95% CI [-23.0, 17.0]).
2. **Poor Node Budget Generalization:** Comparing the gradient direction between 1,000 nodes and deeper search budgets (5,000 and 20,000 nodes/move) yielded only **70.0% sign consistency**. Fast shallow-node feedback does not reliably predict deeper play.
3. **Severe Integer Discretization Loss:** At c = 0.015625 * c0, only 20.63% of evaluator parameters experience a non-zero displacement after integer rounding (mean absolute displacement 0.159 cp). Attempting to perturb all 261 parameters simultaneously with valid behavioral changes (<25% root move disagreement) means ~80% of parameters receive zero perturbation in any given step, while the remaining ~20% introduce variance without discernible game-score signal at practical local sample sizes.

---

## 1. Fixed-Node Timeout & Search Repetition Fix

During earlier diagnostics, worker timeouts under fixed-node searches occurred. Investigation revealed two root causes:
- **Harness Clock Assumption:** The Python harness previously derived safety timeouts from dummy chess clock times even when nodes_per_move was specified. This was corrected in tools/tuning/spsa_tune.py line 451 to use an absolute safety deadline (120.0s) protecting only against genuine engine hangs.
- **Engine Search Repetition Loop:** In Search.cpp, when all legal root moves lead to immediate three-fold repetition, the search spends 0 nodes (searchNodeCount == 0). Consequently, searchNodeCount >= maxNodes was never satisfied, causing iterative deepening to spin up to depth 73,000+ taking >120s. A safety guard was added in Search.cpp at line 1105 to break the iterative deepening loop if maxNodes > 0 && (searchNodeCount >= maxNodes || recDepth >= 128).
- **Verification:** Verified with 16-way concurrent execution of repetition positions and game tasks at fixed nodes. Immediate termination (<0.02s), zero timeouts, and legal move output confirmed.

---

## 2. Perturbation Screening (Selected Scale: 0.015625 * c)

Using 128 unique openings from 128 separate TWIC games across 20 deterministic perturbation directions, perturbation magnitude screening yielded:

| Scale | Root Disagreement | Static Score Disagreement | Parameters Changed | Mean Rounded Displacement | Screening Failures |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **0.25 * c** | 55.31% | 98.83% | 99.04% | 2.544 | 0 |
| **0.125 * c** | 46.72% | 98.09% | 83.58% | 1.272 | 0 |
| **0.0625 * c** | 38.48% | 96.02% | 58.18% | 0.636 | 0 |
| **0.03125 * c** | 28.83% | 94.06% | 35.98% | 0.318 | 0 |
| **0.015625 * c** | **22.19%** | **87.11%** | **20.63%** | **0.159** | **0** |

Scale **0.015625 * c** is the only tested scale satisfying the target local perturbation envelope (5% <= root disagreement <= 25%).

---

## 3. Independent 64-Pair Signal Study

Using 128 unique openings with paired color reversals (nested prefixes of 8, 16, 32, and 64 pairs) across 20 deterministic perturbation directions at c = 0.015625:

### Sign Agreement vs 64-Pair Reference
- **8 pairs vs 64 pairs:** 40.0% (8 / 20)
- **16 pairs vs 64 pairs:** 55.0% (11 / 20)
- **32 pairs vs 64 pairs:** 70.0% (14 / 20)

### Pair-Level Bootstrap Uncertainty (64 Pairs)
Across all 20 directions, the 95% bootstrap confidence interval of score difference D = sum(s+ - s-) spanned 0 in 18 out of 20 directions (90% of samples):
- Dir 0: D = -3.0, 95% CI [-23.0, 17.0]
- Dir 1: D = +4.0, 95% CI [-18.0, 25.0]
- Dir 2: D = -7.0, 95% CI [-26.0, 13.0]
- Dir 3: D = -8.0, 95% CI [-28.0, 12.0]
- Dir 4: D = 0.0, 95% CI [-23.0, 23.0]
- Dir 5: D = -3.0, 95% CI [-23.0, 17.0]
- Dir 6: D = -8.0, 95% CI [-28.0, 13.0]
- Dir 7: D = +2.0, 95% CI [-19.0, 24.0]
- Dir 8: D = -2.0, 95% CI [-25.0, 21.0]
- Dir 9: D = +8.0, 95% CI [-11.0, 27.0]
- Dir 10: D = -2.0, 95% CI [-22.0, 18.0]
- Dir 11: D = -1.0, 95% CI [-21.0, 19.0]
- Dir 12: D = +1.0, 95% CI [-19.0, 20.0]
- Dir 13: D = -14.0, 95% CI [-34.0, 6.0]
- Dir 14: D = +31.0, 95% CI [11.0, 51.0] (significant)
- Dir 15: D = -1.0, 95% CI [-22.0, 21.0]
- Dir 16: D = -23.0, 95% CI [-41.0, -4.0] (significant)
- Dir 17: D = +12.0, 95% CI [-6.0, 30.0]
- Dir 18: D = -11.0, 95% CI [-31.0, 10.0]
- Dir 19: D = +14.0, 95% CI [-6.0, 33.0]

---

## 4. Search Node Budget Stability

Evaluating 10 deterministic directions across identical opening pairs at deeper node budgets:
- **5,000 nodes/move vs 1,000 nodes/move:** 70.0% (7 / 10) sign agreement
- **20,000 nodes/move vs 1,000 nodes/move:** 70.0% (7 / 10) sign agreement

A 30% sign inversion rate indicates substantial noise and tactical horizon effects when transferring shallow node gradients to deeper play.

---

## 5. Performance & Wall-Clock Projections

- **Recommended Node Budget (if attempting):** 5,000 nodes/move
- **Recommended Minimum Pairs per Update:** 64 pairs (128 games) minimum to attain >=70% nominal sign stability
- **Measured Wall Time per Update (64 pairs at 5k nodes):** 18.55 seconds
- **Projected Local Runtime:**
  - 500 updates: 2.58 hours (9,276 seconds)
  - 1,000 updates: 5.15 hours (18,552 seconds)
  - 2,000 updates: 10.31 hours (37,104 seconds)

---

## 6. Conclusion & Architectural Recommendation

Local classic SPSA tuning of all 261 parameters simultaneously on game results is **not viable** due to:
1. Signal-to-noise ratio: games at c=0.015625 cannot resolve the gradient without hundreds of pairs per update.
2. Discretization: 80% of parameters do not move due to integer rounding at small c, meaning SPSA steps are effectively random walks on a small, shifting subset of weights.
3. Instead of whole-engine self-play SPSA, parameter tuning should utilize:
   - Supervised Texel-style / logistic regression tuning on quiet labelled positions for the 261 evaluation weights.
   - SPSA reserved strictly for small subsets (e.g. 5-10 search heuristics / time management parameters) where static regression is inapplicable.
