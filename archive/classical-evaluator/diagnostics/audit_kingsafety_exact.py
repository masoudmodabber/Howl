import csv

with open("evaluator-analysis/geometry/parameter-statistics.tsv") as f:
    stats = {r["name"]: r for r in csv.DictReader(f, delimiter="\t") if r["family"] == "KingSafety"}

print(f"Total KingSafety parameters: {len(stats)}")

# Categorization of all 23 parameters
params_info = [
    ("KingAttackerPawnWeight", 0, "attacker", "A: structurally unreachable in practice (pawns rarely attack king zone directly without pawn check/tactics; baseline 0)"),
    ("KingAttackerMinorWeight", 9, "attacker", "G: meaningful and distinct (minor piece in king zone)"),
    ("KingAttackerRookWeight", 8, "attacker", "G: meaningful and distinct (rook in king zone)"),
    ("KingAttackerQueenWeight", 8, "attacker", "G: meaningful and distinct (queen in king zone)"),
    ("KingDefenderPawnWeight", 12, "defender", "F: independently meaningful but low support (friendly pawn defending king zone; 1.6% active)"),
    ("KingDefenderMinorWeight", 108, "defender", "G: meaningful and distinct (friendly minor defending king zone; 11% active)"),
    ("KingDefenderRookWeight", 6, "defender", "F: independently meaningful but low support (friendly rook defending king zone; 4% active)"),
    ("KingDefenderQueenWeight", 135, "defender", "F: independently meaningful but low support (friendly queen defending king zone; 7.3% active)"),
    ("KingShelterSecondRankDanger", 1, "shelter", "G: meaningful and distinct (shelter pawn on 2nd rank vs 1st rank; 14.5% active)"),
    ("KingShelterAdvancedPawnDanger", 20, "shelter", "G: meaningful and distinct (shelter pawn pushed 2+ ranks / pawn storm; 11.8% active)"),
    ("KingShelterMissingPawnDanger", 2, "shelter", "G: meaningful and distinct (missing shield pawn; 19.4% active)"),
    ("KingShelterOpenFileDanger", 0, "shelter", "C: redundant with KingOpenLineDanger and baseline zero"),
    ("KingUndefendedZoneDanger", 4, "escape_zone", "G: meaningful and distinct (undefended ring squares; 20.6% active)"),
    ("KingAdditionalZoneAttackerDanger", 55, "escape_zone", "F: low support (3.3% active; extreme danger scale +55)"),
    ("KingSemiOpenLineDanger", 36, "lines", "F: low support (4.4% active; semi-open file pressure)"),
    ("KingOpenLineDanger", 26, "lines", "F: low support (5.6% active; open file pressure)"),
    ("KingDiagonalLineDanger", 9, "lines", "F: low support (0.8% active; bishop/queen diagonal ray to king)"),
    ("KingControlledEscapeDanger", 22, "escape_zone", "G: meaningful and distinct (enemy controlled escape squares; 22.5% active)"),
    ("KingBlockedEscapeDanger", 0, "escape_zone", "C: intentional baseline zero (friendly pieces blocking escapes; 42.9% active)"),
    ("KingTrappedEscapeDanger", 0, "escape_zone", "C: intentional baseline zero (<3 safe escapes; 27.2% active)"),
    ("KingHeavyBatteryDanger", 0, "battery", "B: reachable but extremely rare (0.1% active) and baseline zero"),
    ("KingPinnedShelterPawnWeight", 0, "pins", "B: baseline zero (pinned shelter pawn; 7.1% active)"),
    ("KingInfiltratedQueenWeight", 19, "infiltrated", "F: low support (3.2% active; enemy queen on 1st/2nd rank)")
]

with open("evaluator-analysis/redesign/king-safety/low-support-analysis.tsv", "w") as out:
    out.write("parameter\tcurrent_value\tsubsystem\tclassification\tnonzero_positions\tnonzero_fraction\tstd\tnotes\n")
    for name, cur, sub, cls in params_info:
        s = stats.get(name, {})
        nonz = s.get("nonzero_positions", "0")
        frac = s.get("nonzero_fraction", "0.0")
        sd = s.get("standard_deviation", "0.0")
        out.write(f"{name}\t{cur}\t{sub}\t{cls[:1]}\t{nonz}\t{frac}\t{sd}\t{cls}\n")

print("Generated low-support-analysis.tsv")
