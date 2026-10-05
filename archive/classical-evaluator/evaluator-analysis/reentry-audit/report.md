# Post-Redesign Reentry Audit Report

## 1. Executive Summary
Following the completion of all Phase 8 through Phase 14 representation cleanups across the six modified evaluator families (**Attack**, **PieceSquare**, **QueenMobility**, **KingSafety**, **EndgameWeights**, and **Inline**), this audit evaluates every degree of freedom removed, tied, absorbed, or frozen.

The purpose is to separate **permanently dead or redundant artifacts** of the old representations from **genuine chess concepts** that were temporarily removed or frozen only because the old formulation made them unidentifiable, collinear, or dead.

- **Current Canonical Parameter Count:** 190
- **Total Removed / Frozen Degrees of Freedom Audited:** 71 parameters
- **Classification Summary:**
  - **Category 1 (Permanently Dead / Unreachable):** 14 dimensions (e.g., King attacks in threat table, pawn rank 8 promotion, trapped king extremes).
  - **Category 2 (Purely Redundant & Merged):** 49 dimensions (e.g., raw 30 endgame attack cells, collinear quadratic PST terms, KingSafety duplicate lines).
  - **Category 3 (Structural Relationships Remaining Fixed):** 6 dimensions (e.g., EndgameWeights monotonic 72/22 edge/corner ratio, QueenMobility saturation tying).
  - **Category 4 (Genuine Chess Concepts Eligible for Clean Reentry):** **2 candidates**.

---

## 2. In-Depth Audit of Specific Key Dimensions

### A. `BishopPairValue` (Frozen at 132 in Inline)
- **Old Pathology:** In the old `Inline` representation, `BishopPairValue` was embedded in a non-linear expression `max(0, BishopPairValue - 2 - totalPawns * 3)` that was evaluated symmetrically for both sides. In positions with equal bishop pairs or equal pawn counts, the derivative with respect to `BishopPairValue` was identically zero, causing it to appear as an algebraically dead dimension (effective rank 7/8, condition number $\infty$).
- **Chess Reality:** The advantage of having the two bishops against bishop+knight or two knights is one of the most fundamental principles in positional chess, directly affecting long-range diagonal dominance, open-game conversion, and endgame technique.
- **Classification:** **Category 4: Genuine Chess Concept**.
- **Reentry Recommendation:** Restore `BishopPairValue` as a clean, differential, non-saturating bonus when one side possesses $\ge 2$ bishops and the opponent possesses $< 2$.
  - Parameterization: Single scalar parameter `BishopPairBonus` (baseline 132, range 80–200).
  - Identifiability: Guaranteed full rank and non-zero derivative across all unbalanced minor-piece imbalances (~34% of positions).

### B. `AttackEndgameMultiplier` (Frozen at 1.4x / 140% in Attack)
- **Old Pathology:** In production Howl, the Attack table allocated 30 independent parameters to EndGame piece-on-piece threat cells. These 30 cells were severely data-starved ($<0.5\%$ activation frequency) and rendered the family rank-deficient (rank 44/60). In Candidate 1, they were collapsed into a single phase-tapered representation where $V_{\text{EG}} = \text{round}(1.4 \times V_{\text{MG}})$.
- **Chess Reality:** Tactical threats become relatively more decisive as material comes off the board (a rook penetrating to threaten an isolated pawn or an unprotected minor in an endgame is worth far more than the same threat amidst heavy middlegame pieces). The degree to which threats escalate in endgames is a legitimate, global tuning degree of freedom.
- **Classification:** **Category 4: Genuine Chess Concept**.
- **Reentry Recommendation:** Expose the empirical phase multiplier as an identifiable single scalar parameter:
  - Parameterization: `AttackEndgameMultiplierPercent` (baseline 140%, range 50%–250%).
  - Identifiability: Orthogonal to the 10 MG threat tiers; active in every endgame position containing an attacked piece.

### C. `EndgameWeights` Edge / Corner Tied Ratio (72 / 22 Ratio)
- **Old Pathology:** `LoneKingEdgeWeight` and `LoneKingCornerWeight` had a near-perfect negative collinearity of $r = -0.985$, driving the condition number to 27.57.
- **Analysis:** Tying the edge and corner terms in the ratio $\frac{72}{22}$ mathematically guarantees strict monotonicity ($\Delta \ge +22$ cp per step, +116 cp along diagonals) towards the mating corner for bare-king endgames (KQvK, KRvK). Un-tying them would re-introduce severe collinearity and create the risk of non-monotonic anti-mating local minima.
- **Classification:** **Category 3: Structural Relationship That Should Remain Fixed**.

### D. `QueenMobility` Tied Saturation Relationships
- **Analysis:** In MiddleGame, increments $p_3$ and $p_4$ (move counts 9..12 and 13..15) were tied into a single saturation parameter $p_{\text{sat}} = 4$. Queens rarely achieve $\ge 13$ quiet legal moves without initiating tactics. In EndGame, the extreme terminal increment is fixed to ensure monotonic saturation.
- **Classification:** **Category 3: Structural Relationship That Should Remain Fixed**.

### E. `KingSafety` Baseline Zero Parameters (6 Parameters)
- **Analysis:** `KingAttackerPawnWeight`, `KingShelterOpenFileDanger`, `KingBlockedEscapeDanger`, `KingTrappedEscapeDanger`, `KingHeavyBatteryDanger`, and `KingPinnedShelterPawnWeight` were zero in production and either completely subsumed by existing terms (open line danger, missing pawn danger, pin evaluation) or dominated by search.
- **Classification:** **Category 1 & 2: Permanently Dead or Purely Redundant**.

### F. `PieceSquare` Quadratic and Edge Redundancies (10 Parameters)
- **Analysis:** Quadratic rank parameters for pawns, rooks, and queens ($r > 0.90$ collinear with linear rank), quadratic file centrality for knights ($r = 0.914$ collinear with linear centrality), and unreachable rank 8 pawn promotion.
- **Classification:** **Category 1 & 2: Permanently Dead or Purely Redundant**.

---

## 3. Parameter Count Synthesis & Phase 15 Readiness

| Stage / Representation State | Canonical Parameter Count | Identifiability / Condition Status |
| :--- | :---: | :--- |
| Baseline Before Roadmap | 261 | Rank-deficient, cond $\infty$, 142 low-support terms |
| Post Phase 8 Redesigns (Current Frozen) | **190** | 100% full rank across all families, all cond $<4.5$ |
| Candidate Reentry 1: `BishopPairValue` | +1 | Fully identified differential scalar |
| Candidate Reentry 2: `AttackEndgameMultiplier` | +1 | Fully identified phase-scaling scalar |
| **Expected Final Tunable Count for Phase 15** | **192** | **100% full rank, zero dead dimensions** |

### Readiness Verdict
The Howl evaluator is **structurally prepared for Phase 15 tuning**. Restoring the 2 clean Category 4 candidates will provide complete chess expressiveness for minor piece imbalance and tactical endgame scaling without compromising the clean, orthogonal geometry established during the redesigns.
