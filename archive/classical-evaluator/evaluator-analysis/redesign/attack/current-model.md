# Current Attack Model Decomposition

## 1. Overview and Architecture
The Howl evaluator's **Attack** family is formally defined in `Option.cpp` and `tuner/TunerEvaluationState.h` as a $6 \times 5 \times 2 = 60$ parameter Cartesian grid:
- **6 Attacking Piece Types:** Pawn (1), Knight (2), Bishop (3), Rook (4), Queen (5), King (6).
- **5 Victim Piece Types:** Pawn (1), Knight (2), Bishop (3), Rook (4), Queen (5). (Slots 9..13 mirror 1..5 for opponent color).
- **2 Phases:** MiddleGame (`[0]`) and EndGame (`[2]`).

Evaluation logic is executed inside `PieceMoveCountFast` ([EvaluationLogic.cpp:2950-2984](file:///home/masoud/Code/Howl/EvaluationLogic.cpp#L2950-L2984)):
1. A local lambda `threatScore(attackingSide)` iterates over the opponent's `ctx.weakPieces[victimSide]` (undefended pieces or pieces attacked by enemy pawns/double attacks).
2. For each weak victim, it queries all friendly pieces currently attacking that victim.
3. If the attacker is a pawn, it verifies the pawn is not undefended against enemy pawn counter-attacks.
4. It computes the threat bonus:
   $$\text{value} = \text{TaperedTable}(\text{AttackerAttackValue}, \text{victimType})$$
   and selects the maximum threat value over all attackers: $\max(0, \text{value})$.
5. If the victim is hanging (`ctx.hangingPieces & (1ULL << victim)`), an additional 50% bonus is added: $\text{total} += \text{best} + \text{best} / 2$.
6. `whiteAttackValue = threatScore(0)` and `blackAttackValue = threatScore(1)`.
7. The net difference is scaled:
   $$\text{scaledAttackNet} = \frac{(\text{whiteAttackValue} - \text{blackAttackValue}) \times \text{Option::PieceAttackScalePercent}}{100}$$

*Note:* Earlier in `PieceMoveCountFast` (lines 2610–2945), various piece-move loops add values into `whiteAttackValue` and `blackAttackValue`. However, at lines 2980–2981, these variables are completely overwritten by `threatScore(0)` and `threatScore(1)`.

---

## 2. Parameter Subgroup Breakdown

### A. Pawn Attackers (`PawnAttack*`, 10 parameters)
- **Features Activated:** Pawns attacking enemy Knights, Bishops, Rooks, and Queens (Victim types 2..5).
- **MG / EG Behavior:**
  - `PawnAttackKnight`: MG 20, EG 20
  - `PawnAttackBishop`: MG 20, EG 20
  - `PawnAttackRook`: MG 76, EG 99
  - `PawnAttackQueen`: MG 86, EG 118
  - `PawnAttackPawn`: MG 0, EG 0 (Never evaluated: pawn attacks on pawns are handled by pawn structure; initialized to 0).
- **Concept:** Tactical pawn fork / piece harassment. High value against major pieces, moderate against minor pieces.
- **Interactions:** Weak overlap with mobility (pawn attacks restrict piece mobility squares). Completely independent of KingSafety except when a pawn attacks a piece defending the king.

### B. Knight Attackers (`KnightAttack*`, 10 parameters)
- **Features Activated:** Knights attacking Pawns, Knights, Bishops, Rooks, Queens.
- **MG / EG Behavior:**
  - `KnightAttackPawn`: MG 7, EG 10
  - `KnightAttackKnight`: MG 0, EG 0
  - `KnightAttackBishop`: MG 24, EG 39
  - `KnightAttackRook`: MG 41, EG 49
  - `KnightAttackQueen`: MG 41, EG 49
- **Concept:** Knight outpost harassment and fork potential.
- **Interactions:** Strong co-activation with KnightMobility and KnightOutpost. Moderate overlap with KingSafety when the knight jumps to the king zone.

### C. Bishop Attackers (`BishopAttack*`, 10 parameters)
- **Features Activated:** Bishops attacking Pawns, Knights, Bishops, Rooks, Queens along diagonals.
- **MG / EG Behavior:**
  - `BishopAttackPawn`: MG 7, EG 10
  - `BishopAttackKnight`: MG 24, EG 39
  - `BishopAttackBishop`: MG 0, EG 0
  - `BishopAttackRook`: MG 41, EG 49
  - `BishopAttackQueen`: MG 41, EG 49
- **Concept:** Long diagonal tactical pin, skewer, or harassment against enemy pieces.
- **Interactions:** Identical numerical weights to KnightAttack in almost all cells (`[7, 0, 24, 41, 41]` in MG and `[10, 0, 39, 49, 49]` in EG).

### D. Rook Attackers (`RookAttack*`, 10 parameters)
- **Features Activated:** Rooks attacking Pawns, Knights, Bishops, Rooks, Queens along ranks and files.
- **MG / EG Behavior:**
  - `RookAttackPawn`: MG -1, EG 29
  - `RookAttackKnight`: MG 15, EG 49
  - `RookAttackBishop`: MG 15, EG 49
  - `RookAttackRook`: MG 0, EG 0
  - `RookAttackQueen`: MG 24, EG 49
- **Concept:** 7th rank pressure, open file rook battery, pinning enemy pieces.
- **Interactions:** Heavy co-activation with RookFile (open/semi-open files) and RookMobility. Overlaps with KingSafety when rooks align against king zone defenders.

### E. Queen Attackers (`QueenAttack*`, 10 parameters)
- **Features Activated:** Queens attacking enemy pieces.
- **MG / EG Behavior:**
  - `QueenAttackPawn`: MG 3, EG 6
  - `QueenAttackKnight`: MG 8, EG 15
  - `QueenAttackBishop`: MG 8, EG 15
  - `QueenAttackRook`: MG 16, EG 30
  - `QueenAttackQueen`: MG 0, EG 0
- **Concept:** Queen infiltration and multi-target threats.
- **Interactions:** Heavy overlap with QueenMobility. High risk of overvaluing "queen checks" or pseudo-threats that are easily repelled.

### F. King Attackers (`KingAttack*`, 10 parameters)
- **Features Activated:** Kings attacking enemy pieces.
- **MG / EG Behavior:** All 10 parameters are identically 0 in `Option.cpp` (`{0, 0, 0, 0, 0, 0}`).
- **Status:** **Structurally unreachable and dead.**

---

## 3. Conceptual Nature of the 10-12 Effective Dimensions
Although nominally 60 parameters, the effective rank is only 44 (with 16 dead dimensions), and the actual distinct chess degrees of freedom are only 10–12:
1. **Pawn-on-Minor Threat:** Pawn attacking Knight/Bishop (~20 cp).
2. **Pawn-on-Major Threat:** Pawn attacking Rook/Queen (~80-100 cp).
3. **Minor-on-Pawn Harassment:** Knight/Bishop attacking weak pawn (~7-10 cp).
4. **Minor-on-Minor Asymmetry:** Knight attacking Bishop or vice-versa (~24-39 cp). Equal piece attacks are 0.
5. **Minor-on-Major Tactical Threat:** Minor piece attacking Rook/Queen (~41-49 cp).
6. **Rook-on-Pawn Pressure:** Rook attacking pawn (MG -1, EG 29).
7. **Rook-on-Minor Harassment:** Rook attacking Knight/Bishop (~15 cp MG, ~49 cp EG).
8. **Rook-on-Queen Skewer:** Rook attacking Queen (~24 cp MG, ~49 cp EG).
9. **Queen Multi-Target Harassment:** Queen attacking minor/rook (~8-16 cp).
10. **Hanging Target Bonus:** Explicit $+50\%$ multiplier on weak pieces.
11. **Phase Tapering:** Transition from MG to EG (EG threats are statistically rare and largely collinear with MG ratios).
