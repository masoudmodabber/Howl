# Current QueenMobility Model Decomposition

## 1. Overview and Architecture
In Howl, the **QueenMobility** family comprises 10 canonical parameters:
- **5 MiddleGame parameters:** `QueenMobilityMiddleGameParameters[0..4]`
  - `QueenMobilityMiddleGameBase` ($p_0 = -6$)
  - `QueenMobilityMiddleGameIncrement_1` ($p_1 = 3$)
  - `QueenMobilityMiddleGameIncrement_2` ($p_2 = 15$)
  - `QueenMobilityMiddleGameIncrement_3` ($p_3 = 4$)
  - `QueenMobilityMiddleGameIncrement_4` ($p_4 = 4$)
- **5 EndGame parameters:** `QueenMobilityEndGameParameters[0..4]`
  - `QueenMobilityEndGameBase` ($p_0 = 10$)
  - `QueenMobilityEndGameIncrement_1` ($p_1 = 30$)
  - `QueenMobilityEndGameIncrement_2` ($p_2 = 33$)
  - `QueenMobilityEndGameIncrement_3` ($p_3 = 31$)
  - `QueenMobilityEndGameIncrement_4` ($p_4 = 27$)

During engine initialization (`Option::Initialize()`), these vectors are expanded via `MobilityV2::GenerateQueenMiddleGame` and `MobilityV2::GenerateQueenEndGame` into 28-element lookup tables (`QueenMoveCountValueMiddleGame[0..27]` and `QueenMoveCountValueEndGame[0..27]`).

During search and evaluation (`EvaluationLogic.cpp`), the queen's pseudo-legal mobility count is looked up in `Option::QueenMoveCountValue` via the 24-point phase taper:
```cpp
taperedGroup1Table(Option::QueenMoveCountValue, moveCount);
```
which evaluates to:
$$\text{Score} = \frac{\text{MG\_Table}[\text{moveCount}] \times \text{phase} + \text{EG\_Table}[\text{moveCount}] \times (24 - \text{phase})}{24}$$
and is accumulated directly into the board `activity` accumulator.

---

## 2. Table Generation & Bucket Mechanics ([MobilityV2.h](file:///home/masoud/Code/Howl/MobilityV2.h))

### 2.1 Anchor Decoding
Both phases decode parameters sequentially with a monotonic non-negativity constraint on increments:
$$\text{anchor}[0] = p_0$$
$$\text{anchor}[i] = \text{anchor}[i - 1] + \max(0, p_i) \quad \text{for } i \in \{1, 2, 3, 4\}$$

- **MiddleGame Anchors:**
  - $a_0 = -6$ (Bucket 0)
  - $a_1 = -6 + 3 = -3$ (Bucket 4)
  - $a_2 = -3 + 15 = 12$ (Bucket 8)
  - $a_3 = 12 + 4 = 16$ (Bucket 12)
  - $a_4 = 16 + 4 = 20$ (Bucket 15)
- **EndGame Anchors:**
  - $a_0 = 10$ (Bucket 0)
  - $a_1 = 10 + 30 = 40$ (Bucket 3)
  - $a_2 = 40 + 33 = 73$ (Bucket 6)
  - $a_3 = 73 + 31 = 104$ (Bucket 9)
  - $a_4 = 104 + 27 = 131$ (Bucket 12)

### 2.2 Linear Interpolation Across Fixed Bucket Nodes
- **MiddleGame Buckets:** Anchors are placed at counts `{0, 4, 8, 12, 15}`.
  Linear integer interpolation with symmetric rounding (`RoundDivide`) is applied across each span:
  - Span [0..4] (span 4, delta +3): values $[-6, -5, -4, -4, -3]$
  - Span [4..8] (span 4, delta +15): values $[-3, 1, 5, 8, 12]$
  - Span [8..12] (span 4, delta +4): values $[12, 13, 14, 15, 16]$
  - Span [12..15] (span 3, delta +4): values $[16, 17, 19, 20]$
  - Clamped Span [16..27]: all clamped to $a_4 = 20$.
  Full MG Table (0..27):
  `[-6, -5, -4, -4, -3, 1, 5, 8, 12, 13, 14, 15, 16, 17, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20]`

- **EndGame Buckets:** Anchors are placed at counts `{0, 3, 6, 9, 12}`.
  - Span [0..3] (span 3, delta +30): values $[10, 20, 30, 40]$
  - Span [3..6] (span 3, delta +33): values $[40, 51, 62, 73]$
  - Span [6..9] (span 3, delta +31): values $[73, 83, 94, 104]$
  - Span [9..12] (span 3, delta +27): values $[104, 113, 122, 131]$
  - Clamped Span [13..27]: all clamped to $a_4 = 131$.
  Full EG Table (0..27):
  `[10, 20, 30, 40, 51, 62, 73, 83, 94, 104, 113, 122, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131]`

---

## 3. How Mobility Is Counted in Evaluation
In `EvaluationLogic.cpp`:
1. `ctx.mobilityArea[side]` excludes:
   - Own pieces: `~own`
   - Squares attacked by opponent pawns: `~pawnAttacks[1 - side]`
   - Squares subject to uncontested opponent double-attacks: `~(doubleAttacks[1 - side] & ~doubleAttacks[side])`
2. For each queen on board:
   ```cpp
   const uint64_t attacks = ctx.attacks[piecePosition];
   moveCount = __builtin_popcountll(attacks & ctx.mobilityArea[side]);
   activity += taperedGroup1Table(Option::QueenMoveCountValue, moveCount);
   ```
3. Chess Meaning of Every Bucket Tier:
   - **Tiers 0–3 (Trapped / Severely Constrained):** Queen has 0 to 3 legal/pseudo-legal escape squares outside enemy pawn control. In MG, penalized (-6 to -4 cp); in EG, receives small base (+10 to +40 cp).
   - **Tiers 4–8 (Normal Middlegame Scope):** Queen has standard central or second-rank scope (4 to 8 moves). Represents the steep active transition from passive (-3 cp) to active (+12 cp).
   - **Tiers 9–15 (Highly Active Infiltration):** Queen commands broad rays across open files/diagonals. MG bonus saturates smoothly from +13 to +20 cp.
   - **Tiers 16–27 (Unbounded Open Board Scope):** Rare in quiet play; clamped flat at +20 cp (MG) and +131 cp (EG).
