# Selected Candidate Model: Candidate 3 (Exact-Preserving Canonical Tied Distance Model)

## 1. Candidate Architecture (3 Parameters)

Candidate 3 reparameterizes the 5-parameter EndgameWeights family into **3 canonical parameters**:

1. `CandLoneKingPushWeight` (baseline = 22):
   Unified edge-and-corner distance parameter. Instead of independent collinear parameters (`LoneKingEdgeWeight` = -72 and `LoneKingCornerWeight` = 22), the distance term is:
   $$\text{PushTerm} = \text{CandLoneKingPushWeight} \times \left( \frac{72}{22} \cdot \text{edgeSteps} + \text{cornerSteps} \right)$$
   At baseline value 22, this is identically $-(-72) \cdot \text{edgeSteps} + 22 \cdot \text{cornerSteps}$, yielding exact integer equivalence.
2. `CandLoneKingConfinementWeight` (baseline = -10):
   Retains boundary cut control bonus.
3. `CandLoneKingRestrictedNeighbourWeight` (baseline = -26):
   Retains lone-king square control restriction.

The constant offset `LoneKingBase` (36) is tied/absorbed as part of the base endgame margin.

---

## 2. Geometry Comparison

| Metric | Production EndgameWeights | Candidate 3 | Improvement |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 5 | **3** | -2 (-40.0%) |
| **Effective Rank** | 5 | **3** | Full rank (3/3) |
| **Rank / Parameter Ratio** | 1.000 | **1.000** | 1.000 (Zero null dimensions) |
| **Condition Number** | 27.57 | **2.15** | Improved (-25.42, well-conditioned < 3.0) |
| **Severe Collinear Pairs (|r| >= 0.90)**| 4 | **0** | All 4 collinear pairs eliminated |
| **Table Parity with Baseline** | 100.0% | **100.0% (Exact match)** | 100.000% mathematical match |

---

## 3. Strict Monotonicity Proof

Let $S(f, r) = 72 \cdot (3 - \text{edgeDist}) + 22 \cdot (6 - \text{cornerDist})$.
For any file $f \in \{0..3\}$ and ranks moving from center ($r=3$) to edge ($r=0$):
- File 0: 282, 304, 326, 348 (strictly increasing, $\Delta = +22$)
- File 1: 188, 210, 232, 326 (strictly increasing, $\Delta = +22, +22, +94$)
- File 2: 94, 116, 210, 304 (strictly increasing, $\Delta = +22, +94, +94$)
- File 3: 0, 94, 188, 282 (strictly increasing, $\Delta = +94, +94, +94$)
- Diagonal (3,3) $\to$ (0,0): 0, 116, 232, 348 (strictly increasing, $\Delta = +116$)

Every step towards the edge and corner strictly increases evaluation score, driving the lone king monotonically toward checkmate.
