# Current EndgameWeights Representation Model

## 1. Parameters Inventory (5 Canonical Parameters)

| Name | Baseline Value | Units | Scope | Usage Formula | Chess Meaning |
| :--- | :---: | :---: | :---: | :--- | :--- |
| `LoneKingBase` | 36 | cp | Lone-King | `+ base` | Base score offset awarded when winning against a lone king |
| `LoneKingEdgeWeight` | -72 | cp/step | Lone-King | `- edgeWeight * edgeSteps` | Step bonus (+72) for driving lone king towards outer edges |
| `LoneKingCornerWeight` | 22 | cp/step | Lone-King | `+ cornerWeight * cornerSteps` | Step bonus (+22) for driving lone king towards corners |
| `LoneKingConfinementWeight` | -10 | cp/sq | Lone-King | `- confinementWeight * confinementSupport` | Bonus (+10) for winning king restricting escape boundary squares |
| `LoneKingRestrictedNeighbourWeight` | -26 | cp/sq | Lone-King | `- restrictedNeighbourWeight * (8 - safeNeighbours)` | Bonus (+26) for each king neighbor square attacked/controlled |

---

## 2. Mathematical Formulation

```cpp
const int edgeDistance = std::min({file, 7 - file, rank, 7 - rank});
const int edgeSteps = 3 - edgeDistance; // 0..3
const int nearestCornerDistance = std::min(file, 7 - file) + std::min(rank, 7 - rank);
const int cornerSteps = 6 - nearestCornerDistance; // 0..6

const int guidance = base - edgeWeight * edgeSteps + cornerWeight * cornerSteps
                   - confinementWeight * confinementSupport
                   - restrictedNeighbourWeight * (8 - safeNeighbours);
return whiteWinning ? guidance : -guidance;
```

---

## 3. Mathematical Pathology & Collinearity Analysis

- **Condition Number:** **27.57** (the highest in the entire Howl evaluation function).
- **Internal Collinearity:**
  - `LoneKingEdgeWeight` vs `LoneKingCornerWeight`: $r = -0.985$ ($|r| \ge 0.97$).
  - `LoneKingCornerWeight` vs `LoneKingRestrictedNeighbourWeight`: $r = -0.938$.
  - `LoneKingEdgeWeight` vs `LoneKingRestrictedNeighbourWeight`: $r = 0.935$.
  - `LoneKingBase` vs `LoneKingRestrictedNeighbourWeight`: $r = -0.930$.
- **Mechanism of Pathology:**
  1. Both `edgeSteps` (0..3) and `cornerSteps` (0..6) measure geometric distance from the center (3,3) to the boundary. In fact:
     $$\text{cornerSteps} = \text{edgeSteps} + (3 - \max(\min(f, 7-f), \min(r, 7-r)))$$
     The two terms are 85.3% collinear across all board squares and >98% collinear along game trajectories.
  2. Because `edgeWeight` enters with a minus sign ($-72$) and `cornerWeight` enters with a plus sign ($+22$), optimizer steps on one are virtually indistinguishable from opposing steps on the other, creating a narrow, ill-conditioned parabolic valley.
  3. `restrictedNeighbours` ($8 - \text{safeNeighbours}$) is physically constrained by the edge: on the rim, a king has at most 5 board neighbors; in the corner, only 3. Thus, pushing the king to the edge automatically inflates the restricted neighbour term, duplicating the edge/corner bonus.
