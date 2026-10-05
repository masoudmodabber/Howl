# QueenMobility Preserved Information Specification

## 1. Principles of Preservation
Queen mobility represents a sliding piece activity signal, but in contrast to Knights, Bishops, and Rooks, Queens have massive ray reach (up to 27 squares) and high material value (900 cp). Consequently:
1. **Severe Trapping & Confinement Must Be Penalized:** A queen with 0 to 3 legal/pseudo-legal moves outside enemy pawn control is either blockaded behind friendly pieces or trapped. Penalizing low mobility is an essential safety barrier against trapped queen blunders.
2. **Normal Middlegame Scope Transition Must Be Preserved:** Moving from constrained scope (4 moves) to active development (8 moves) provides the primary positional incentive for sound queen mobilization.
3. **Open-Board Endgame Activity Must Be Maintained:** In endgames, active queens dominate open boards and escort passed pawns. The high endgame scale (+10 to +131 cp) reflects this decisive dynamic.

---

## 2. Redundancies and Artifacts to Eliminate
1. **Equal Saturation Increments ($p_3 = 4, p_4 = 4$):**
   - In MG, $p_3$ covers buckets 9..12 (+4 cp) and $p_4$ covers 13..15 (+4 cp). Both are modest asymptotic transitions leading into the flat cap at 16..27.
2. **Low-Support Endgame Increments ($p_1 \dots p_4$ in EG):**
   - In standard play, queen endgames occur in only ~6% of positions. The 4 separate EG increments ($30, 33, 31, 27$) are virtually collinear ($r > 0.98$) with a single linear slope of ~10 cp/bucket. Retaining 5 independent parameters for a linear progression over starved data creates parameter bloat without chess benefit.
3. **Cross-Family Overlap with Queen PieceSquare and KingSafety:**
   - A centralized queen already receives file centrality bonuses from Queen PieceSquare. When QueenMobility also grants steep bonuses, the evaluator double-rewards central queen outings before minor piece development is complete.
