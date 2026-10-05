# KingSafety Non-Linear Danger Model Decomposition

## 1. Algorithmic Pipeline in `EvaluateKingDanger`

The KingSafety evaluation pipeline processes four stages of transformation:

```
Stage 1: Linear Term Accumulation
  attackerParticipation, defenderParticipation, safeChecks, escapeDanger,
  lineDanger, shelterDanger, pawnStorm, undefendedZoneDanger,
  defensiveRestriction, infiltratedQueenDanger
         │
         ▼
Stage 2: Raw Danger Synthesis
  rawDanger = (attackerParticipation * 2) + safeChecks + escapeDanger +
              lineDanger + shelterDanger + pawnStorm + balanceDanger +
              undefendedZoneDanger + defensiveRestriction + infiltratedQueenDanger
         │
         ▼
Stage 3: Attacking Material Scaling (Phase & Piece Counts)
  attackingMaterialScale = min(100, base + 45*Q + 12*R + 5*M) * phaseScale
  rawDanger = rawDanger * attackingMaterialScale / 100
         │
         ▼
Stage 4: Escalation & Gating
  if (!credibleAttack) -> rawDanger = 0
  if (attackerCount >= 2):
      escalatedDanger = rawDanger + (rawDanger^2) / (180 + defenderParticipation * 4)
  if (attackerCount == 1):
      escalatedDanger = gated fractions of rawDanger (1/2 to 7/8)
  finalDanger = min(450, escalatedDanger)
```

---

## 2. Mathematical Breakdown of Non-Linearities

### A. The Quadratic Escalation Term: $\frac{\text{rawDanger}^2}{180 + 4 \cdot \text{defenders}}$
- When `attackerCount >= 2` and `credibleAttack` is true, the danger term squares the raw danger sum.
- **Pathology:** If `rawDanger` reaches 80 cp, the squared term adds $\frac{6400}{180} \approx 35\text{ cp}$. If `rawDanger` reaches 150 cp, the squared term adds $\frac{22500}{180} \approx 125\text{ cp}$!
- This super-linear explosive growth is the direct engine mechanism causing the 17-case "King Exposure & Attack Overvaluation" cluster. Small heuristic additions (e.g. +36 for semi-open line, +22 for escape square) become doubled and tripled through the quadratic escalation, overwhelming positional and material judgment.

### B. Attacker / Defender Balance Ratio
- `balanceDanger = max(0, attackerParticipation - defenderParticipation) + max(0, attackerCount - defenderCount) * 4`
- However, `attackerParticipation` is already added as `attackerParticipation * 2` into `rawDanger`, AND `defenderParticipation` is used in the denominator of the quadratic escalation (`180 + defenderParticipation * 4`).
- Thus, defender count/weight is used in three non-linearly interacting locations simultaneously.

### C. Gating Condition: `credibleAttack`
- `credibleAttack = (nonPawnRingAttackers >= 2 || safeCheckCount > 0 || directRingLine)`
- If false, `rawDanger` is instantly collapsed to 0.
- This creates sharp discontinuous cliffs: adding a single harmless pawn or distant piece that qualifies as a safe check or ring attacker causes the evaluation to jump from 0 to over 100 centipawns in a single ply.
