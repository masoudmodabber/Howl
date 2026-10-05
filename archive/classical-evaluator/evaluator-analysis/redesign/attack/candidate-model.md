# Candidate Attack Model Specification

## 1. Mathematical and Algorithmic Model
Candidate 1 defines a **Tiered Threat Matrix** evaluated conditionally inside `threatScore`:
- Let $A$ be the attacking piece type $\in \{1=\text{Pawn}, 2=\text{Knight}, 3=\text{Bishop}, 4=\text{Rook}, 5=\text{Queen}\}$. (King is omitted).
- Let $V$ be the victim piece type $\in \{1=\text{Pawn}, 2=\text{Knight}, 3=\text{Bishop}, 4=\text{Rook}, 5=\text{Queen}\}$.
- Piece Tier Groupings:
  - $\text{Tier}(V) = \begin{cases} \text{Pawn} & \text{if } V = 1 \\ \text{Minor} & \text{if } V \in \{2, 3\} \\ \text{Rook} & \text{if } V = 4 \\ \text{Queen} & \text{if } V = 5 \end{cases}$

### MiddleGame Parameter Vector $\theta_{\text{threat}}$ (10 canonical parameters):
1. $\theta_0$: `PawnOnMinor_MG` = 20
2. $\theta_1$: `PawnOnMajor_MG` = 84 (Weighted avg of Rook: 76, Queen: 86)
3. $\theta_2$: `MinorOnPawn_MG` = 7 (Weighted avg of Knight: 7, Bishop: 7)
4. $\theta_3$: `MinorOnMinor_MG` = 24 (Weighted avg of N-on-B: 24, B-on-N: 24)
5. $\theta_4$: `MinorOnMajor_MG` = 41 (Weighted avg of Minor-on-Rook: 41, Minor-on-Queen: 41)
6. $\theta_5$: `RookOnPawn_MG` = -1
7. $\theta_6$: `RookOnMinor_MG` = 15 (Weighted avg of R-on-N: 15, R-on-B: 15)
8. $\theta_7$: `RookOnQueen_MG` = 24
9. $\theta_8$: `QueenOnPawn_MG` = 3
10. $\theta_9$: `QueenOnPiece_MG` = 10 (Weighted avg of Q-on-N: 8, Q-on-B: 8, Q-on-R: 16)

### Phase Scaling:
- Candidate derives EndGame values deterministically using the historical empirical phase multiplier:
  $$\text{Value}(A, V, \text{phase}) = \frac{\text{Value}_{\text{MG}}(A, V) \times \text{phase} + \text{Value}_{\text{EG}}(A, V) \times (24 - \text{phase})}{24}$$
  where $\text{Value}_{\text{EG}} = \text{round}(1.4 \times \text{Value}_{\text{MG}})$, consistent with the empirical EG boost in the baseline.

---

## 2. Deterministic Mapping from Production Parameters
Every value in the candidate model is computed as the support-weighted average of the corresponding production parameters:
- `PawnOnMinor`: $\frac{15 \times 20 + 14 \times 20}{29} = 20$
- `PawnOnMajor`: $\frac{1 \times 76 + 5 \times 86}{6} = 84.3 \rightarrow 84$
- `MinorOnPawn`: $\frac{161 \times 7 + 169 \times 7}{330} = 7$
- `MinorOnMinor`: $\frac{11 \times 24 + 30 \times 24}{41} = 24$
- `MinorOnMajor`: $\frac{12 \times 41 + 5 \times 41 + 7 \times 41 + 8 \times 41}{32} = 41$
- `RookOnPawn`: $-1$
- `RookOnMinor`: $\frac{12 \times 15 + 15 \times 15}{27} = 15$
- `RookOnQueen`: $24$
- `QueenOnPawn`: $3$
- `QueenOnPiece`: $\frac{55 \times 8 + 34 \times 8 + 50 \times 16}{139} = 10.8 \rightarrow 10$

No arbitrary new values or tuned constants are introduced.
