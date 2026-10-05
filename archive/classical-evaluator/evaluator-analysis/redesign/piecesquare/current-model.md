# Current PieceSquare Model Decomposition

## 1. Overview and Architecture
The Howl evaluator's **PieceSquare** family comprises 96 canonical parameters defined in `Option.h` / `Option.cpp` and generated via `PieceSquareModel.h`.
The parameters are grouped by piece type and game phase (MiddleGame and EndGame):
- **Pawn:** 7 parameters $\times$ 2 phases = 14 parameters
- **Knight (Minor):** 11 parameters $\times$ 2 phases = 22 parameters
- **Bishop (Minor):** 11 parameters $\times$ 2 phases = 22 parameters
- **Rook (Major):** 6 parameters $\times$ 2 phases = 12 parameters
- **Queen (Major):** 6 parameters $\times$ 2 phases = 12 parameters
- **King:** 7 parameters $\times$ 2 phases = 14 parameters
$$\text{Total Parameters} = 2 \times (7 + 11 + 11 + 6 + 6 + 7) = 96$$

During initialization (`Option::Initialize()`), `PieceSquareModel::Generate*` functions expand these compact vectors into 64-square lookup tables (`PawnInValueWhiteMiddleGame`, `PawnInValueWhiteEndGame`, etc.). Black tables are generated symmetrically by vertically flipping ranks:
$$\text{sq}_{\text{black}} = (7 - (\text{sq} / 8)) \times 8 + (\text{sq} \bmod 8)$$

During evaluation, static piece placement scores are interpolated via the 24-point phase taper:
$$\text{Score} = \frac{\text{MG\_Table}[\text{sq}] \times \text{phase} + \text{EG\_Table}[\text{sq}] \times (24 - \text{phase})}{24}$$

---

## 2. Mathematical Formulas and Parameter Semantics

### 2.1 Pawn Model (7 parameters: $p_0 \dots p_6$)
$$\text{Table}[\text{sq}] = p_0 + p_1 \cdot \text{rank} + p_2 \cdot \text{rank}^2 + p_3 \cdot \text{edge} + p_4 \cdot (\text{edge} \land 1 \le \text{rank} \le 6) + p_5 \cdot (\text{rank} = 1) + p_6 \cdot (\text{rank} = 7)$$
- **$p_0$ (Base Offset):** Baseline constant offset.
- **$p_1$ (Linear Rank):** Bonus per rank of pawn advance.
- **$p_2$ (Quadratic Rank):** Curvature for deep pawn advance. *(Baseline = 0 in both MG and EG)*.
- **$p_3$ (Edge File):** Penalty/bonus for a- and h-file pawns ($f \in \{0, 7\}$).
- **$p_4$ (Edge Interior):** Additional modulation for edge pawns on ranks 2–7.
- **$p_5$ (Starting Rank):** Static adjustment for pawns on the initial 2nd rank ($\text{rank} = 1$).
- **$p_6$ (Promotion Rank):** Pawns on rank 8 ($\text{rank} = 7$). *(Unreachable; pawns promote immediately; baseline = 0)*.

### 2.2 Minor Piece Model (11 parameters: $p_0 \dots p_{10}$ for Knight and Bishop)
Let $\text{fileCentrality} = \min(f, 7 - f) \in [0, 3]$.
$$\text{Table}[\text{sq}] = p_0 + p_1 \cdot \text{rank} + p_2 \cdot \text{fileCentrality} + p_3 \cdot \text{fileCentrality}^2 + p_4 \cdot (\text{rank} = 0) + p_5 \cdot (\text{rank} = 6) + \sum_{r=2}^6 p_{6 + r - 2} \cdot \text{fileCentrality} \cdot [r = \text{rank}]$$
- **$p_0$ (Base Offset):** Piece baseline value.
- **$p_1$ (Linear Rank):** Global vertical advance tendency.
- **$p_2$ (Linear File Centrality):** Preference for central files (c, d, e, f vs a, b, g, h).
- **$p_3$ (Quadratic File Centrality):** Quadratic file curvature.
- **$p_4$ (Back Rank Penalty/Bonus):** Adjustment for 1st rank ($\text{rank} = 0$).
- **$p_5$ (7th Rank Penalty/Bonus):** Adjustment for 7th rank ($\text{rank} = 6$).
- **$p_6 \dots p_{10}$ (Rank-Specific File Centrality):** Independent centrality multipliers for ranks 3, 4, 5, 6, and 7.

### 2.3 Major Piece Model (6 parameters: $p_0 \dots p_5$ for Rook and Queen)
$$\text{Table}[\text{sq}] = p_0 + p_1 \cdot \text{rank} + p_2 \cdot \text{rank}^2 + p_3 \cdot \text{fileCentrality} + p_4 \cdot (\text{rank} = 0) + p_5 \cdot (\text{rank} = 6)$$
- **$p_0$ (Base Offset):** Baseline constant offset.
- **$p_1$ (Linear Rank):** General advancement slope.
- **$p_2$ (Quadratic Rank):** Nonlinear rank curvature. *(Baseline = 0 in both MG and EG)*.
- **$p_3$ (File Centrality):** Horizontal centralization slope.
- **$p_4$ (1st Rank Anchor):** Baseline back rank offset.
- **$p_5$ (7th Rank Infiltration):** Static bonus for penetration onto the 7th rank ($\text{rank} = 6$).

### 2.4 King Model (7 parameters: $p_0 \dots p_6$)
Let $\text{rankCentrality} = \min(r, 7 - r) \in [0, 3]$.
$$\text{Table}[\text{sq}] = p_0 + \sum_{k=1}^3 p_k \cdot [\text{fileCentrality} = k] + \sum_{k=1}^3 p_{3 + k} \cdot [\text{rankCentrality} = k]$$
- **$p_0$ (Corner / Edge Anchor):** Value for corner/edge squares.
- **$p_1, p_2, p_3$:** Independent step bonuses for file distances 1, 2, and 3 from the edge.
- **$p_4, p_5, p_6$:** Independent step bonuses for rank distances 1, 2, and 3 from the board edge.
