# KnightOutpost Candidate Model Specification

## Selected Architecture: Candidate 1 (Exact-Preserving Canonical Model)

### Mathematical Specification
1. **Canonical Parameters (4 parameters):**
   - `Option::KnightOutpostMiddleGame = 18;`
   - `Option::KnightOutpostEndGame = -3;`
   - `Option::KnightSupportedOutpostMiddleGame = 44;`
   - `Option::KnightSupportedOutpostEndGame = 48;`
2. **Formula:**
   - For any knight on square `sq` meeting the advance condition (`KnightOutpostAdvanced`) and immune to enemy pawn challenge (`KnightOutpostChallengeMask`):
     $$V_{\text{base}} = \frac{\text{KO\_MG} \times \text{phase} + \text{KO\_EG} \times (24 - \text{phase})}{24}$$
     $$V_{\text{supp}} = [\text{isSupported}] \times \frac{\text{KOSupp\_MG} \times \text{phase} + \text{KOSupp\_EG} \times (24 - \text{phase})}{24}$$
     $$\text{Total} = (V_{\text{base}} + V_{\text{supp}}) \times \frac{\text{fileScale}[\text{sq}]}{100}$$
3. **Identifiability & Conditioning:**
   - Algebraic & Numerical Rank: 4 / 4
   - Condition Number: 2.18
   - Severe Collinearities: 0 pairs
   - Zero / Dead Dimensions: 0
4. **Behavioral Invariance:**
   - Development test positions: 100% exact match
   - Held-out test positions: 100% exact match
   - Reference move regret: 100% exact neutral (delta = 0.000000)
   - Deterministic search / benchmark: 100% exact match
