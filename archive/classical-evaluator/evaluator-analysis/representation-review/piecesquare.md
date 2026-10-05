# PieceSquare Representation Audit

## 1. Parameter Architecture & Organization
- **Total Canonical Parameters:** 96 parameters across 6 piece types $\times$ 2 phases (MiddleGame and EndGame).
  - **Pawn:** 7 MG + 7 EG = 14 parameters (`PawnPieceSquareMiddleGameParameters`, `PawnPieceSquareEndGameParameters`).
    - Formulation: $P[0] + P[1]\cdot\text{rank} + P[2]\cdot\text{rank}^2 + P[3]\cdot\text{edge} + P[4]\cdot(\text{edge}\land 1\le\text{rank}\le 6) + P[5]\cdot(\text{rank}==1) + P[6]\cdot(\text{rank}==7)$.
  - **Knight & Bishop (Minors):** 11 MG + 11 EG each = 44 parameters.
    - Formulation: $P[0] + P[1]\cdot\text{rank} + P[2]\cdot\text{fileCent} + P[3]\cdot\text{fileCent}^2 + P[4]\cdot(\text{rank}==0) + P[5]\cdot(\text{rank}==6) + \sum_{r=2}^6 P[6+r-2]\cdot\text{fileCent}$.
  - **Rook & Queen (Majors):** 6 MG + 6 EG each = 24 parameters.
    - Formulation: $P[0] + P[1]\cdot\text{rank} + P[2]\cdot\text{rank}^2 + P[3]\cdot\text{fileCent} + P[4]\cdot(\text{rank}==0) + P[5]\cdot(\text{rank}==6)$.
  - **King:** 7 MG + 7 EG = 14 parameters.
    - Formulation: file centrality indicator steps (files 1, 2, 3) + rank centrality indicator steps (ranks 1, 2, 3).
- **Encoding & Symmetries:** Horizontal file symmetry is already baked into the compact formulation for minors, majors, and kings via `fileCentrality = std::min(col, 7 - col)`.
- **Generation:** [PieceSquareModel.h](file:///home/masoud/Code/Howl/PieceSquareModel.h) dynamically populates the $6 \times 64$-square tables (`PawnInValueWhiteMiddleGame`, etc.), and black tables are mirrored copies.

---

## 2. Mathematical Conditioning & Identifiability
- **Nominal Parameter Count:** 96
- **Effective Numerical Rank:** 94 (Rank deficient; singular value spectrum drops below relative tolerance $10^{-6}$).
- **Low Support Parameters (<5% nonzero activation or near-zero variance):** 34 of 96 parameters (35.4%).
- **Severe Collinearities ($|r| \ge 0.90$):**
  - `PawnPieceSquareMiddleGame_1` vs `PawnPieceSquareMiddleGame_2` ($r = 0.9044$)
  - `KnightPieceSquareMiddleGame_2` vs `KnightPieceSquareMiddleGame_3` ($r = 0.9141$)
  - `RookPieceSquareMiddleGame_1` vs `RookPieceSquareMiddleGame_2` ($r = 0.9272$)
  - `PawnPieceSquareEndGame_1` vs `PawnPieceSquareEndGame_2` ($r = 0.9094$)
  - `RookPieceSquareEndGame_1` vs `RookPieceSquareEndGame_2` ($r = 0.9427$)
  - `QueenPieceSquareEndGame_1` vs `QueenPieceSquareEndGame_2` ($r = 0.9115$)
- **Core Diagnosis:** The quadratic rank and linear rank parameters ($P[1]$ and $P[2]$) are strongly collinear across all pieces (correlation $>0.90$). The model fits redundant polynomial curvatures over narrow boards.

---

## 3. Semantic & Conceptual Overlap with Other Families
1. **Mobility (Knight/Bishop/Rook/Queen):**
   - Centralized squares inherently yield higher available ray attacks and pseudo-legal move counts. In Howl, placing a knight or bishop in the center awards both the PST centrality parameter and the top mobility table bucket.
2. **KnightOutpost:**
   - [EvaluationLogic.cpp:2656](file:///home/masoud/Code/Howl/EvaluationLogic.cpp#L2656) adds `KnightOutpostValue` on ranks 4-6 if protected by a pawn. Minor PST already awards specific rank bonuses ($P[6+r-2]\cdot\text{fileCent}$ for ranks 2 to 6).
3. **RookFile:**
   - Rooks on central files receive PST file centrality, open-file bonuses (`RookOpenFileMiddleGame`), and open-file mobility ray popcounts simultaneously.
4. **KingSafety & Pawn Shield:**
   - King PST penalizes central squares in MG ($P[1..6]$) while `EvaluateKingDanger` independently evaluates pawn shield degradation, open files to the king, and virtual mobility of attackers.

---

## 4. Failure Analysis & Causal Evidence
- **Involved in High-Confidence Failures:** 34 of 100 cases (26 of 52 evaluation/mixed cases with $|\Delta| \ge 15\text{ cp}$).
- **Causal Ablation Reversals:** 11 cases directly reversed from Howl blunder back to Stockfish reference move when scaling down PieceSquare (0.0x / 0.5x).
- **Cluster Representation:** Appears heavily in `PST Piece Placement Collinearity` (19 cases) and `Mobility / Activity Trapping` (20 cases).
- **Mechanism:** Static square bonuses frequently overrule piece coordination, leading Howl to leave pieces statically "well-placed" on central squares even when tactically dominated or trapped.

---

## 5. Structural Recommendation
- **Status:** **RESHAPE CURRENT REPRESENTATION**
- **Action:**
  - Reduce independent polynomial degree: replace correlated $P[1]\text{rank} + P[2]\text{rank}^2$ with single monotonic progression or linear slope.
  - Decouple central file bonuses from explicit outpost bonuses to eliminate double-counting with Mobility and KnightOutpost.
  - Preserves spatial baseline (est. ~48-56 effective dimensions) while eliminating rank deficiency and high collinearity pairs.
