# PieceSquare Structural Alternatives

This document outlines three architectural candidates for reshaping Howl's PieceSquare representation from its current 96-parameter structure.

---

## Alternative 1: Quadratic-Cleaned Geometric Formulation (Safest Refinement)

### Concept
Eliminate all algebraically redundant quadratic terms and unreachable boundary cells while maintaining 100% of the existing linear geometric architecture and table values.
- **Pawn (5 parameters $\times$ 2 = 10):**
  - Drop $p_2$ ($\text{rank}^2$, baseline 0, collinear $r = 0.904$).
  - Drop $p_6$ ($\text{rank} = 7$, unreachable, baseline 0).
  - Retain: Base ($p_0$), Linear Rank ($p_1$), Edge ($p_2$), Edge Interior ($p_3$), Rank 1 ($p_4$).
- **Minor Pieces (10 parameters $\times$ 2 = 20 for Knight, 20 for Bishop):**
  - Drop $p_3$ ($\text{fileCentrality}^2$, collinear $r = 0.958$ with linear centrality).
  - Retain: Base ($p_0$), Linear Rank ($p_1$), Linear Centrality ($p_2$), Rank 0 ($p_3$), Rank 6 ($p_4$), Rank-Specific Centrality for ranks 2..6 ($p_5 \dots p_9$).
- **Major Pieces (5 parameters $\times$ 2 = 10 for Rook, 10 for Queen):**
  - Drop $p_2$ ($\text{rank}^2$, baseline 0, collinear $r \ge 0.927$).
  - Retain: Base ($p_0$), Linear Rank ($p_1$), Linear Centrality ($p_2$), Rank 0 ($p_3$), Rank 6 ($p_4$).
- **King (7 parameters $\times$ 2 = 14):**
  - Retain the exact 7 step-distance parameters for MG and EG.
- **Total Parameters:** $10 + 20 + 20 + 10 + 10 + 14 = 84 \text{ parameters}$.
- **Redundancy Removed:** Eliminates all 6 severe internal collinearity pairs ($|r| \ge 0.90$) and all unreachable cells.
- **Information Loss Risk:** **ZERO.** Generates tables mathematically identical to the production baseline.

---

## Alternative 2: Factored Rank & Centrality Grid with Decoupled Outpost (Moderate Consolidation)

### Concept
Separate each piece type into an independent 1D Rank Profile (8 ranks) and a 1D File Centrality Profile (4 distances), with an explicit 7th-rank infiltration bonus for majors.
- Replaces piecewise rank-centrality products in minors with separable rank and file vectors:
  $$\text{Table}[r, f] = \text{RankProfile}[r] + \text{CentralityProfile}[\text{fileCentrality}]$$
- **Piece Breakdown per Phase:**
  - Pawn: 4 rank steps + 2 edge penalties = 6 parameters
  - Minor (Knight / Bishop): 4 rank steps + 3 centrality steps = 7 parameters
  - Major (Rook / Queen): 4 rank steps + 2 centrality steps + 1 7th-rank bonus = 7 parameters
  - King: 3 rank steps + 3 file steps + 1 corner anchor = 7 parameters
- **Total Parameters per Phase:** $6 + 7 + 7 + 7 + 7 + 7 = 41 \text{ parameters} \times 2 = 82 \text{ parameters}$.
- **Redundancy Removed:** Eliminates rank-centrality interaction terms ($p_6 \dots p_{10}$ in minors) that duplicate `KnightOutpost`.
- **Information Loss Risk:** Low-Moderate. May slightly smooth diagonal bishop outposts on c4/f4.

---

## Alternative 3: Unified Geometric Surface with Empirical Phase Taper (Aggressive Consolidation)

### Concept
Mirror the Attack redesign by defining a single MiddleGame spatial surface for all 6 piece types, using an empirical phase scaling factor for endgame transformation rather than duplicating 48 independent endgame parameters.
- For King: Separate MG safety table (corner preference) and EG activity table (center preference).
- For non-king pieces (P, N, B, R, Q): Store 1 unified MiddleGame placement profile (32 parameters total) and scale endgames via a piece-specific endgame advancement scalar $\alpha_{\text{piece}}$.
- **Total Parameters:** 48 parameters.
- **Information Loss Risk:** Moderate-High. Pawns and Rooks exhibit qualitatively different spatial behavior in endgames (pawns push forward, rooks shift behind passers) that a single scalar cannot fully capture.

---

## Comparative Assessment Matrix

| Metric | Alternative 1 (Safest Refinement) | Alternative 2 (Factored Profile) | Alternative 3 (Unified Surface) |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | **84** | 82 | 48 |
| **Effective Dimensions** | **84 / 84** (Full Rank) | 78 / 82 | 48 / 48 |
| **Collinearity Pairs ($|r| \ge 0.90$)** | **0** (All 6 eliminated) | 0 | 0 |
| **Null / Unreachable Dimensions** | **0** (All eliminated) | 0 | 0 |
| **Condition Number** | **< 4.5** | < 4.0 | < 3.5 |
| **Information Loss Risk** | **ZERO (100% table identity)** | Low-Moderate | High |
| **Mapping Difficulty** | **Trivial / Exact Identity** | Approximate Projection | Constrained Fit |
| **Recommendation** | **SELECTED CANDIDATE** | Secondary | Rejected |
