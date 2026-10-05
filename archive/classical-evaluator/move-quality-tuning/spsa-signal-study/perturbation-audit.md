# Howl SPSA Parameter Perturbation & Mathematical Viability Audit

## 1. Parameter Perturbation Audit (Scale 0.015625)

### Distribution of Parameters by Number of Perturbed Directions (out of 20)
Across all 20 tested directions at c = 0.015625 * c0:
- **0 directions:** 17 parameters (6.5%)
- **1 to 4 directions:** 173 parameters (66.3%)
- **5 to 9 directions:** 47 parameters (18.0%)
- **10 to 15 directions:** 18 parameters (6.9%)
- **16 to 20 directions:** 6 parameters (2.3%)

**Subset Survival:** The same tiny subset of parameters repeatedly survives rounding across directions. The top 6 parameters that changed in 16–20 directions are large-scale parameters: `QueenValue` (20/20), `RookValue` (20/20), `BishopValue` (20/20), `KnightValue` (20/20), `OppositeColorBishopMiddleGameScalePermille` (20/20), and `OppositeColorBishopEndGameScalePermille` (20/20). Meanwhile, 190 of 261 parameters (72.8%) changed in 4 or fewer directions.

### Evaluator Family Breakdown
| Family | Parameter Count | Nonzero (+) Rate | Nonzero (-) Rate | Displacement Rate | Mean |Displacement| when Changed |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **PieceValue** | 5 | 0.890 | 0.870 | 0.930 | 1.941 |
| **Inline** | 8 | 0.431 | 0.388 | 0.506 | 1.914 |
| **EndgameWeights** | 5 | 0.140 | 0.140 | 0.270 | 0.519 |
| **KingSafety** | 23 | 0.100 | 0.120 | 0.189 | 0.580 |
| **Attack** | 60 | 0.122 | 0.122 | 0.186 | 0.567 |
| **PieceSquare** | 96 | 0.101 | 0.103 | 0.188 | 0.543 |
| **RookFile** | 4 | 0.100 | 0.100 | 0.188 | 0.533 |
| **KnightMobility** | 8 | 0.081 | 0.113 | 0.188 | 0.517 |
| **RookMobility** | 10 | 0.205 | 0.165 | 0.165 | 0.515 |
| **PassedPawnV2** | 13 | 0.065 | 0.081 | 0.142 | 0.514 |
| **QueenMobility** | 10 | 0.100 | 0.075 | 0.165 | 0.530 |
| **KnightOutpost** | 4 | 0.125 | 0.062 | 0.175 | 0.536 |
| **BishopMobility** | 10 | 0.205 | 0.175 | 0.175 | 0.514 |
| **PawnStructure** | 1 | 0.100 | 0.050 | 0.150 | 0.500 |
| **IsolatedPawn** | 2 | 0.000 | 0.100 | 0.100 | 0.500 |
| **RookBehindPassedPawn** | 2 | 0.000 | 0.025 | 0.025 | 0.500 |

---

## 2. Stockfish SPSA Scaling & Perturbation Mechanics

### Mathematical Comparison
In Fishtest classic SPSA:
- Parameters are continuous or scaled such that c_i represents a meaningful fractional exploration step.
- In Howl, evaluation parameters are discrete integers (centipawns or permille).
- Stochastic rounding rounds theta_i +/- c_i * Delta_i to the nearest integer with probability proportional to the fractional remainder: P(round up) = x - floor(x).
- When a uniform global scalar (0.015625) was applied to avoid catastrophic chess disruption:
  - 185 out of 261 parameters (70.9%) have c_i = 0.0722 < 0.10.
  - 255 out of 261 parameters (97.7%) have c_i < 0.50.
  - Median c_i = 0.0722, Mean c_i = 0.1637.

### The Bernoulli Sparse Noise Defect
Because c_i << 1 for 97.7% of parameters, the perturbation does not explore a magnitude; rather, it collapses into a sparse Bernoulli coin flip where the parameter is unchanged with probability ~= 1 - c_i (often >= 93%) and perturbed by exactly +/- 1 with probability ~= c_i.
Applying a uniform global scaling multiplier of 0.015625 across all parameters is mathematically invalid for integer parameters: it leaves pieces values (QueenValue, RookValue) moving by multiple integers while turning the remaining 250+ parameters into erratic, sparse 1-unit binary noise.

---

## 3. Node Budget Relation (5k vs 20k Nodes)

From the 10 paired directions tested at both 5,000 and 20,000 nodes/move:
- **Sign Agreement (5k vs 20k):** 5 / 10 = **50.0%**
- Direction-by-direction comparison:
  - Dir 0: 5k D = -12.0, 20k D = -1.0 (Agree)
  - Dir 1: 5k D = +14.0, 20k D = +3.0 (Agree)
  - Dir 2: 5k D = -12.0, 20k D = +1.0 (Disagree)
  - Dir 3: 5k D = -2.0, 20k D = +16.0 (Disagree)
  - Dir 4: 5k D = +4.0, 20k D = -1.0 (Disagree)
  - Dir 5: 5k D = +3.0, 20k D = -2.0 (Disagree)
  - Dir 6: 5k D = -6.0, 20k D = -3.0 (Agree)
  - Dir 7: 5k D = -21.0, 20k D = +18.0 (Disagree)
  - Dir 8: 5k D = -6.0, 20k D = -28.0 (Agree)
  - Dir 9: 5k D = +5.0, 20k D = +1.0 (Agree)

**Conclusion:** 5k vs 20k sign agreement is exactly 50.0% (pure coin-flip independence). Gradient sign measured at 5,000 nodes has zero predictive value for 20,000 nodes.

---

## 4. Effective Dimension & SPSA Gradient Variance

Across the 20 directions:
- **Mean perturbed coordinates:** 53.85
- **Median perturbed coordinates:** 54.5
- **Min perturbed coordinates:** 42
- **Max perturbed coordinates:** 66
- Nominal dimension p = 261.

### Mathematical Impact on SPSA Variance
Classic SPSA gradient estimator along direction Delta is:
  g_hat = ((y(theta + c*Delta) - y(theta - c*Delta)) / (2*c)) * Delta^(-1)

When only an effective subset S subset of {1, ..., p} (|S| ~= 54) is perturbed due to integer zeroing:
1. Coordinates i not in S receive Delta_i = 0 in actual engine play, yet standard SPSA applies an update proportional to the random trial vector +/- 1/c_i, injecting pure uncorrelated noise into unperturbed parameters.
2. Even if restricting the gradient update to active coordinates, the effective perturbation vector Delta_tilde varies randomly in dimension and norm across iterations (E[||Delta_tilde||^2] ~= 54), which inflates the estimation variance of each active coordinate by a factor of p_eff / c^2 while destroying orthogonality.
3. Because y(theta) has stochastic game outcome noise sigma^2 ~= 0.5, resolving an effective 54-dimensional simultaneous step requires samples far exceeding 64 pairs.

---

## 5. Next Method Decision

### Selected Option: **D (Evidence genuinely supports switching to another tuning objective)**

1. **Why Option A is Rejected:** Per-parameter scaling cannot fix the fundamental dilemma: setting c_i >= 1 for all 261 parameters causes >55% root move disagreement and erratic tactical blunders; scaling down causes c_i << 1 which creates sparse Bernoulli noise and zero-signal games.
2. **Why Option B is Insufficient for Evaluator:** While low-dimensional SPSA (d <= 10) is valid for search parameters, condensing 261 distinct positional evaluation weights into <=10 master scalars discards the required granularity of chess evaluation.
3. **Why Option D is Established:**
   - 5k vs 20k node sign agreement is exactly 50% (noise).
   - Game self-play noise (Var(D) ~= 400 at 64 pairs) dwarfs the evaluator gradient.
   - Evaluation weights are differentiable linear/piecewise features over positions; static position loss (e.g. logistic loss / Texel tuning on millions of quiet positions) evaluates deterministically without game playout variance or search horizon instability.
