# KnightOutpost Current Model Decomposition

## 1. Overview and Architecture
The Howl evaluator's **KnightOutpost** family comprises 4 canonical parameters defined in `Option.h` / `Option.cpp` and evaluated in `EvaluationLogic.cpp`:
- `KnightOutpostMiddleGame` = 18 cp
- `KnightOutpostEndGame` = -3 cp
- `KnightSupportedOutpostMiddleGame` = 44 cp
- `KnightSupportedOutpostEndGame` = 48 cp

## 2. Geometric Mask & Logic Formulation
During board evaluation (`EvaluationLogic.cpp:323-374`), knight positions are audited against static bitboards initialized in `InitializeKnightOutpost()`:
1. **Advance Condition (`KnightOutpostAdvanced`):**
   - White: Ranks 4 to 6 (`r >= 3 && r <= 5`, 0-indexed ranks 3..5).
   - Black: Ranks 3 to 5 (`r >= 2 && r <= 4`, 0-indexed ranks 2..4).
2. **Challenge Immunity (`KnightOutpostChallengeMask`):**
   - Checked via `(PassedPawnSetup::[White/Black]PassedMask[sq] & ~ownFile) & enemyPawns == 0`.
   - Ensures no enemy pawn on adjacent files can ever advance forward to challenge or dislodge the knight.
3. **Pawn Support (`KnightOutpostSupportMask`):**
   - Evaluated via `AttackPlaces::[Black/White]PawnAttackPlaces[sq] & friendlyPawns != 0`.
   - Checks if a friendly pawn actively protects the outpost square.
4. **File Weighting (`KnightOutpostFileScale`):**
   - Scaled by file centrality: `fileScale = {25, 60, 90, 100, 100, 90, 60, 25}` for files a through h.
5. **Phase Taper and Support Accumulation:**
   - Base outpost value: $V_{\text{base}} = \text{Taper}(\text{KO\_MG}, \text{KO\_EG}, \text{phase})$
   - If supported by a friendly pawn: $V_{\text{total}} = V_{\text{base}} + \text{Taper}(\text{KOSupp\_MG}, \text{KOSupp\_EG}, \text{phase})$
   - Final evaluation contribution: $\text{Score} = V_{\text{total}} \times \text{KnightOutpostFileScale}[\text{sq}] / 100$

## 3. Mathematical Identifiability & Health
- **Parameter Count:** 4 parameters.
- **Effective Numerical Rank:** 4 / 4 (full algebraic and numerical rank).
- **Condition Number:** 2.18 (well-conditioned; well below 5.0 threshold).
- **Severe Collinearities ($|r| \ge 0.90$):** 0 pairs (max internal $|r| = 0.748$ between MG base and MG supported).
- **Corpus Activation Frequency:** 8.5% in initial SPSA corpus, 16.5% across 1,000-game positions (165 active positions).
