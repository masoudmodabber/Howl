# Phase 2 Representation Review: PieceSquare, Attack, and QueenMobility

## Executive Summary
This report presents the in-depth representation audit of the three evaluator families that exhibited both high chess failure involvement and severe mathematical/conditioning concerns in Howl: **PieceSquare**, **Attack**, and **QueenMobility**.

Combined, these three families currently account for **166 canonical parameters** (63.6% of Howl's entire 261-parameter evaluator). However, rigorous mathematical decomposition, activation analysis, and empirical failure tracing demonstrate that they actually span only approximately **61 to 72 defensible independent chess dimensions**.

---

## 1. Family Audits & Diagnoses

### A. PieceSquare (96 Current Parameters $\to$ Est. 48-56 Effective Dimensions)
- **Status:** **RESHAPE CURRENT REPRESENTATION**
- **Architecture:** Compact geometric formula ([PieceSquareModel.h](file:///home/masoud/Code/Howl/PieceSquareModel.h)) decomposing boards by rank, rank$^2$, file centrality, and rank edge offsets for 6 piece types $\times$ 2 phases.
- **Mathematical Health:**
  - Effective numerical rank: 94 (rank deficient).
  - 34 low-support parameters (firing $<5\%$ of corpus positions).
  - 6 severe internal collinearity pairs with $|r| \ge 0.90$. Specifically, linear rank and quadratic rank parameters ($P[1]$ vs $P[2]$) are mathematically collinear across Pawns ($r = 0.904$), Rooks ($r = 0.927\text{ MG}, 0.943\text{ EG}$), and Queens ($r = 0.912$).
- **Cross-Family Overlap:** Heavily duplicates `KnightMobility`, `BishopMobility`, `KnightOutpost`, and `RookFile`. Placing minors and rooks on central files automatically scores both PST centrality and peak mobility ray buckets.
- **Failure Impact:** Involved in 34 of 100 high-confidence failures, with 26 cases showing large component deltas ($|\Delta| \ge 15\text{ cp}$) and 11 verified causal reversals when scaled down.

### B. Attack (60 Current Parameters $\to$ Est. 10-12 Effective Dimensions)
- **Status:** **CONSOLIDATE INTERNAL PARAMETERS** (Primary candidate for initial redesign)
- **Architecture:** $6 \times 5 \times 2$ Cartesian grid (6 attackers $\times$ 5 victims $\times$ 2 phases) evaluated in `PieceMoveCountFast` threat scoring.
- **Mathematical Health:**
  - Effective numerical rank: 44 (16 completely dead/unidentifiable dimensions).
  - **57 of 60 parameters (95.0%) have low support** ($<5\%$ activation frequency in 1,000 game positions).
  - Condition number: Infinite (`rank_deficient`).
- **Cross-Family Overlap:** Overlaps directly with `KingSafety`. Attack scores threats against king-zone defenders, while `KingSafety` simultaneously scores attacker weight sums and line pressures against the same king zone.
- **Failure Impact:** Involved in 22 of 100 failures (16 with $|\Delta| \ge 15\text{ cp}$), producing 5 direct causal reversals when ablated.

### C. QueenMobility (10 Current Parameters $\to$ Est. 3-4 Effective Dimensions)
- **Status:** **RESHAPE CURRENT REPRESENTATION**
- **Architecture:** Piecewise 5-anchor linear interpolation ([MobilityV2.h](file:///home/masoud/Code/Howl/MobilityV2.h)) mapping to 28 discrete mobility buckets for MG and EG.
- **Mathematical Health:**
  - Effective numerical rank: 10 (algebraically full rank).
  - **8 of 10 parameters (80.0%) are low-support**. In quiet positions, the queen rarely possesses high mobility without immediate capture refutations. All 5 EG parameters are statistically starved ($<7\%$ activation).
  - Represents a smooth monotonic saturation curve using unnecessarily independent piecewise anchors.
- **Cross-Family Overlap:** Duplicates Major PieceSquare centrality and Queen attack threats.
- **Failure Impact:** Present in 39 failure positions as part of overall mobility deltas, though individual scaling does not independently reverse moves.

---

## 2. Cross-Family Representation Matrix

| Group | Primary Family | Overlapping Families | Represented Concept | Correlation | Defensible Dimensions |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Minor Placement vs Activity** | PieceSquare | KnightMobility, KnightOutpost | Minor centralization and outpost support | $0.65 - 0.78$ | 1 dim (Activity) |
| **Major Control vs Open Lines** | PieceSquare | RookFile, QueenMobility | Rook/Queen ray control and file penetration | $0.58 - 0.73$ | 1 dim (Open Line Activity) |
| **King Threat vs Shelter Safety** | Attack | KingSafety | Converging attacker pressure on shelter | $0.70 - 0.81$ | 1 dim (Attacker Convergence) |
| **PST Internal Polynomials** | PieceSquare | PieceSquare | Spatial rank curvatures | $0.90 - 0.94$ | 1 dim per piece/phase |
| **Queen Mobility Curve** | QueenMobility | PieceSquare, Attack | Queen scope and non-tactical freedom | $0.62 - 0.71$ | 1 dim (Smooth Saturation) |

---

## 3. Structural Synthesis & Decision Matrix

| Family | Current Params | Defensible Dimensions | Core Problem | Proposed Structural Action | Recommendation |
| :--- | :---: | :---: | :--- | :--- | :--- |
| **Attack** | 60 | 10 to 12 | 16 dead dimensions; 95% low-support parameters; duplicates KingSafety | Consolidate $6 \times 5 \times 2$ grid into an 8-12 parameter threat matrix (piece classes + hanging) | **CONSOLIDATE INTERNAL PARAMETERS** |
| **PieceSquare**| 96 | 48 to 56 | Linear/quadratic rank collinearity ($r > 0.90$); double-counts with mobility/outpost | Eliminate quadratic rank parameters; decouple file centrality from explicit outpost bonuses | **RESHAPE CURRENT REPRESENTATION** |
| **QueenMobility**| 10 | 3 to 4 | 80% low-support; fits smooth curve with independent anchors; EG buckets starved | Replace piecewise anchors with smooth 3-parameter function + unified phase taper | **RESHAPE CURRENT REPRESENTATION** |

- **Current Combined Degrees of Freedom:** 166 parameters.
- **Defensible Combined Degrees of Freedom:** 61 to 72 parameters (saving 94 to 105 redundant/unidentifiable parameters).
- **First Recommended Target for Redesign:** **`Attack`**. Attack has the most extreme pathology (16 null dimensions, 95% dead parameters) and high failure entanglement with KingSafety, making its consolidation cleanly bounded and highest return on representation health with virtually zero information loss risk.
