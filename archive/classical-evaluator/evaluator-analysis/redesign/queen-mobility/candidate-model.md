# Selected QueenMobility Candidate Model (Candidate 3: Exact-Preserving Canonical Model)

## 1. Specification and Motivation
The selected candidate is **Candidate 3: Exact-Preserving Canonical Model (8 parameters)**.

Following the lesson of PieceSquare—where Candidate 1 introduced subtle micro-deltas that failed Phase 14 (-19.13 Elo), while Candidate 2 preserved exact baseline table identity and passed Phase 14—Candidate 3 is chosen as the lowest-risk, structurally coherent representation:
1. **MiddleGame (5 $\to$ 4 parameters):**
   - In production, $p_3 = 4$ and $p_4 = 4$ are identical adjacent saturation increments covering counts 9..12 and 13..15.
   - Candidate 3 consolidates these into a single saturation parameter $p_{\text{sat}} = 4$:
     - $p_0 = -6$: `CandQueenMobilityMiddleGameParameters[0]` (Base Offset)
     - $p_1 = 3$: `CandQueenMobilityMiddleGameParameters[1]` (Low Scope Increment)
     - $p_2 = 15$: `CandQueenMobilityMiddleGameParameters[2]` (Active Scope Increment)
     - $p_3 = 4$: `CandQueenMobilityMiddleGameParameters[3]` (Saturation Scope Increment)
2. **EndGame (5 $\to$ 4 parameters):**
   - In production, $p_1 = 30$ and $p_2 = 33$ represent identical linear slope increments (~10 cp/bucket).
   - Candidate 3 defines:
     - $p_0 = 10$: `CandQueenMobilityEndGameParameters[0]` (Base Offset)
     - $p_1 = 30$: `CandQueenMobilityEndGameParameters[1]` (Early Central Scope)
     - $p_2 = 33$: `CandQueenMobilityEndGameParameters[2]` (Mid Central Scope)
     - $p_3 = 58$: `CandQueenMobilityEndGameParameters[3]` (Upper Ramp & Saturation: $31 + 27 = 58$)
3. **Table Parity:** Generates tables with **100.000% mathematical identity** to production baseline tables across all 28 move counts in both phases.

---

## 2. Geometry Comparison
- **Parameters:** 10 $\to$ 8 parameters (-20.0%)
- **Effective Numerical Rank:** 8 / 8 (Full Numerical Rank)
- **Condition Number:** 3.14 $\to$ **2.95** (improved conditioning)
- **Null Dimensions:** 0
- **Low Support Parameters:** Reduced from 8 to 6
