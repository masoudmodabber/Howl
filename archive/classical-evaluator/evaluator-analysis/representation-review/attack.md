# Attack Representation Audit

## 1. Parameter Architecture & Organization
- **Total Canonical Parameters:** 60 parameters organized as 6 attacking piece types $\times$ 5 victim piece types $\times$ 2 phases (MiddleGame and EndGame).
  - **Attackers:** Pawn, Knight, Bishop, Rook, Queen, King.
  - **Victims:** Pawn (1), Knight (2), Bishop (3), Rook (4), Queen (5). (King victim is handled by check/king-safety logic).
  - **Phase:** MiddleGame (`[0]`) and EndGame (`[2]`).
- **Runtime Execution:**
  - Evaluated in `PieceMoveCountFast` ([EvaluationLogic.cpp:2610-2975](file:///home/masoud/Code/Howl/EvaluationLogic.cpp#L2610-L2975)).
  - Crucially, intermediate piece-by-piece attack accumulations are completely overwritten at line 2980 by `threatScore(0)` and `threatScore(1)`.
  - `threatScore` iterates over `ctx.weakPieces` (pieces attacked by enemy or hanging) and for each attacker type finds $\max(\text{AttackTable}[\text{attacker}][\text{victim}])$.
  - The net score is scaled by `Option::PieceAttackScalePercent` (parameter in Inline family, default 100%).

---

## 2. Mathematical Conditioning & Identifiability
- **Nominal Parameter Count:** 60
- **Effective Numerical Rank:** 44 (Extreme rank deficiency: 16 null/dead dimensions).
- **Low Support Parameters (<5% nonzero activation or near-zero variance):** 57 of 60 parameters (95.0%).
  - 57 parameters fire in fewer than 50 of 1,000 corpus positions!
  - Examples: `KingAttackQueen_EndGame`, `KingAttackRook_MiddleGame`, `PawnAttackQueen_EndGame`, `QueenAttackPawn_EndGame`.
  - Many pairs (e.g. King attacking Queen) are practically impossible in legal positions or instantly refuted by tactical captures.
- **Condition Number:** `rank_deficient` (infinite condition number).
- **Diagnosis:** The $6 \times 5 \times 2$ Cartesian matrix is severely overparameterized. 95% of its cells are weakly observable or dead in real chess search positions.

---

## 3. Semantic & Conceptual Overlap with Other Families
1. **KingSafety Family:**
   - KingSafety evaluates attacker count, attacker weight (`KingAttackerMinorWeight`, `KingAttackerRookWeight`, `KingAttackerQueenWeight`), and convergence onto king-adjacent zones.
   - Attack independently scores minor and major attacks onto pieces defending the king or inside the shelter zone.
   - In positions with mating threats or king attacks, Attack bonuses and KingSafety bonuses fire in unison, creating uncontrolled exponential score escalation.
2. **Mobility & SEE (Static Exchange Evaluation):**
   - Ray and knight attacks onto enemy pieces already expand available mobility count squares (since mobility area includes non-pawn attacked enemy pieces).
   - Attack acts as an ad-hoc, unpruned static tactical threat heuristic that duplicates mobility popcounts and QSearch capture resolution.

---

## 4. Failure Analysis & Causal Evidence
- **Involved in High-Confidence Failures:** 22 of 100 cases (16 of 52 evaluation/mixed cases with $|\Delta| \ge 15\text{ cp}$).
- **Causal Ablation Reversals:** 5 cases reversed back to Stockfish move when scaling Attack down (0.0x / 0.5x).
- **Cluster Representation:** Concentrated in `King Exposure & Attack Overvaluation` (17 cases) and `Mobility / Activity Trapping` (20 cases).
- **Mechanism:** Attack bonuses reward "threatening" an enemy piece (e.g. Queen attacking Bishop or Rook attacking Knight) even when the threat is easily parried or tactically tactically refuted, causing Howl to miss quiet defensive/positional moves.

---

## 5. Structural Recommendation
- **Status:** **CONSOLIDATE INTERNAL PARAMETERS** (Primary candidate for immediate redesign)
- **Action:**
  - Replace the 60-parameter Cartesian grid with a compact 8-to-12 parameter threat matrix:
    - Minor-on-Major, Minor-on-Minor, Major-on-Minor, Pawn-on-Piece, and Hanging piece bonus.
  - Collapse EndGame attack terms that have zero statistical support into a unified tapered scalar.
  - Eliminate dead combinations (King attacking heavy pieces).
  - Estimated defensible degrees of freedom: 10-12 (down from 60, saving 48 unidentifiable parameters).
