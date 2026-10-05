# PieceSquare Preserved Information Specification

## 1. Principles of Preservation
Unlike the Attack redesign—which was primarily about eliminating dead dimensions and redundant threat grids—PieceSquare represents the core spatial intuition of the chess engine. Blind compression would destroy essential positional guideposts that search cannot feasibly compensate for.

We identify the positional features that have strong empirical support, zero or low collinearity, and clear chess justification.

---

## 2. Genuinely Independent and Essential Positional Concepts

### 2.1 Piece Centralization (Minors & Queens)
- **Concept:** Placing Knights, Bishops, and Queens closer to the center ($f \in \{2, 3, 4, 5\}$, $\text{fileCentrality} \ge 1$).
- **Chess Role:** Essential for opening development and middlegame board control.
- **Form:** Linear file centrality: $\text{bonus} = p \cdot \text{fileCentrality}$.
- **Evidence:** Active across $>60\%$ of positions. Must be preserved, but quadratic file centrality ($p_3 \cdot \text{fileCentrality}^2$) is redundant ($r = 0.958$) and should be dropped.

### 2.2 Territorial Pawn Advance & Starting Rank Penalty
- **Concept:** Pawns gain value as they advance up the board, securing space and creating mating/queening threats.
- **Chess Role:**
  - Initial 2nd-rank adjustment ($p_5$): Discourages leaving pawns passively unmoved when advantageous.
  - Linear rank advancement ($p_1$): Continuous space incentive.
  - Edge pawn penalty ($p_3, p_4$): A- and h-pawns are less valuable for central space than c-, d-, e-, f-pawns.
- **Evidence:** Essential for positional space evaluation. Must be preserved with linear rank terms and edge adjustments.

### 2.3 Rook 7th-Rank Infiltration & Back-Rank Anchoring
- **Concept:**
  - Major piece 7th-rank infiltration ($p_5$): The classic tactical motif of doubling or placing rooks on the enemy 7th rank ($\text{rank} = 6$).
  - 1st-rank baseline anchor ($p_4$): Calibrates unmoved vs active rooks along the back rank.
- **Chess Role:** Crucial for major piece invasion in both middlegames and endgames.

### 2.4 King Spatial Dualism (Shelter vs Endgame Centralization)
- **Concept:**
  - **MiddleGame:** King must stay on the edge/corners after castling (g1/b1/c1). Moving to central ranks (e1, e2, d2, e3) is heavily penalized.
  - **EndGame:** King must march toward the center (e4, d4, e5, d5) to escort passed pawns and blockade enemy kings.
- **Chess Role:** Fundamental phase-dependent chess behavior.
- **Form:**
  - MG King table rewards edge files and penalizes central ranks.
  - EG King table gives large positive bonuses for rank and file centrality (+96 at center).
- **Evidence:** Vital for both king safety in middlegames and king activation in endgames. Must be strictly preserved.

---

## 3. Redundancies and Distortions That Must Be Removed

1. **Quadratic Rank Collinearities ($p_2 = 0$):**
   - Pawns, Rooks, and Queens already have $p_2 = 0$ in production baseline. Re-exposing quadratic rank parameters in tuning creates mathematical collinearity ($|r| \in [0.90, 0.94]$) with linear rank terms. These must be permanently eliminated.
2. **Unreachable Promotion Cells ($p_6 = 0$ for Pawns):**
   - Pawns on rank 7 ($\text{rank} = 7$, 8th rank) promote immediately in chess. This cell never activates in real search and should be excised.
3. **Quadratic File Centrality in Minors:**
   - On a 4-value domain ($0, 1, 2, 3$), $x$ and $x^2$ have correlation $0.958$. Having both creates severe ill-conditioning without unique positional information.
4. **Independent Per-Rank Centrality Slopes in Minors ($p_6 \dots p_{10}$):**
   - Ranks 2 through 6 each have an independent centrality multiplier. In practice, ranks 5 and 6 duplicate `KnightOutpost`. A unified or constrained rank-centrality interaction provides sufficient expressiveness without overfitting.
