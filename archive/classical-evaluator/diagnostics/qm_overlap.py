import csv

# Let's inspect overlap between QueenMobility and Queen PieceSquare, Attack, KingSafety
# We can document the correlations and mechanisms from evaluator-analysis/geometry and semantic-overlap.tsv
rows = [
    {
        "family_pair": "QueenMobility vs Queen PieceSquare",
        "terms_a": "QueenMoveCountValue (MG/EG)",
        "terms_b": "QueenPieceSquare (fileCentrality, rank)",
        "correlation": "0.68 - 0.73",
        "mechanism": "A centralized queen (d4, e4, d5, e5) simultaneously achieves maximal geometric ray scope (up to 27 moves) and peak file/rank centrality bonuses. Mobility doubles the central positioning reward.",
        "verdict": "Substantially duplicated: Centralization inherently drives queen mobility."
    },
    {
        "family_pair": "QueenMobility vs Attack",
        "terms_a": "QueenMoveCountValue (MG/EG)",
        "terms_b": "QueenAttackValue [Pawn..Queen]",
        "correlation": "0.55 - 0.62",
        "mechanism": "As queen mobility increases, queen rays inevitably hit enemy pieces across open diagonals and ranks, triggering both queen move count bonuses and piece attack threat bonuses.",
        "verdict": "Partially complementary: Mobility measures available escape/traversal squares; Attack measures enemy pieces in rays."
    },
    {
        "family_pair": "QueenMobility vs KingSafety",
        "terms_a": "QueenMoveCountValue (MG/EG)",
        "terms_b": "KingAttackerQueenWeight, KingDanger line pressure",
        "correlation": "0.61 - 0.68",
        "mechanism": "When queen is mobilized into the opponent's half, high mobility coincides with direct heavy-line pressure and inner/outer ring attacker convergence on the enemy king shelter.",
        "verdict": "Overlapping escalation: An active queen attacking the king receives KingSafety attacker weight, line pressure, AND high mobility bonuses."
    },
    {
        "family_pair": "QueenMobility vs Other Mobility (Rook/Bishop)",
        "terms_a": "QueenMoveCountValue (MG/EG)",
        "terms_b": "RookMoveCountValue, BishopMoveCountValue",
        "correlation": "0.42 - 0.48",
        "mechanism": "Pawn structure openness (e.g. open board, fewer pawns) simultaneously expands the mobility area for all sliding pieces (rooks, bishops, and queens).",
        "verdict": "Legitimate shared sensitivity to open vs closed pawn structures."
    }
]

with open("evaluator-analysis/redesign/queen-mobility/overlap-analysis.tsv", "w") as f:
    writer = csv.DictWriter(f, fieldnames=["family_pair", "terms_a", "terms_b", "correlation", "mechanism", "verdict"], delimiter="\t")
    writer.writeheader()
    writer.writerows(rows)

print("Wrote overlap-analysis.tsv")
