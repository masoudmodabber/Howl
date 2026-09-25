#include "AttackPlaces.h"
#include "BoardInitializer.h"
#include "BoardMaker.h"
#include "EvaluationLogic.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "MobilityV2.h"
#include "PassedPawnV2.h"
#include "PieceSquareModel.h"
#include "Option.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace
{
void InitializeEngine()
{
    Option::Initialize();
    AttackPlaces::Initialize();
    BoardInitializer::Initialize();
    PieceMoves::Initialize();
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();
}

void CleanupEngine()
{
    AttackPlaces::Cleanup();
    BoardInitializer::Cleanup();
    PieceMoves::Cleanup();
    MoveLogic::Cleanup();
    KingSetup::Cleanup();
    PassedPawnSetup::Cleanup();
}

struct CanonicalParameter
{
    std::string name;
    std::string family;
    int* targetPtr;
};

std::vector<CanonicalParameter> BuildCanonical192Registry()
{
    std::vector<CanonicalParameter> reg;
    reg.reserve(192);

    // 1. PieceValue (5)
    reg.push_back({"PawnValue", "PieceValue", &Option::PawnValue});
    reg.push_back({"KnightValue", "PieceValue", &Option::KnightValue});
    reg.push_back({"BishopValue", "PieceValue", &Option::BishopValue});
    reg.push_back({"RookValue", "PieceValue", &Option::RookValue});
    reg.push_back({"QueenValue", "PieceValue", &Option::QueenValue});

    // 2. PawnStructure (1)
    reg.push_back({"DoubledPawnValue", "PawnStructure", &Option::DoubledPawnValue});

    // 3. PassedPawnV2 (13: 6 MG ranks, 6 EG ranks, 1 MG file amplitude)
    reg.push_back({"PassedPawnMiddleGameBase", "PassedPawnV2", &Option::PassedPawnMiddleGameParameters[0]});
    for (int i = 1; i < 6; ++i)
        reg.push_back({"PassedPawnMiddleGameIncrement_" + std::to_string(i), "PassedPawnV2", &Option::PassedPawnMiddleGameParameters[i]});
    reg.push_back({"PassedPawnEndGameBase", "PassedPawnV2", &Option::PassedPawnEndGameParameters[0]});
    for (int i = 1; i < 6; ++i)
        reg.push_back({"PassedPawnEndGameIncrement_" + std::to_string(i), "PassedPawnV2", &Option::PassedPawnEndGameParameters[i]});
    reg.push_back({"PassedPawnMiddleGameFileAmplitude", "PassedPawnV2", &Option::PassedPawnMiddleGameFileAmplitude});

    // 4. PieceSquare (86: Candidate 2 conservative model)
    for (int i = 0; i < PieceSquareModel::CandPawnParameterCount; ++i)
        reg.push_back({"PawnPieceSquareMiddleGame_" + std::to_string(i), "PieceSquare", &Option::CandPawnPieceSquareMiddleGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandKnightParameterCount; ++i)
        reg.push_back({"KnightPieceSquareMiddleGame_" + std::to_string(i), "PieceSquare", &Option::CandKnightPieceSquareMiddleGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandBishopParameterCount; ++i)
        reg.push_back({"BishopPieceSquareMiddleGame_" + std::to_string(i), "PieceSquare", &Option::CandBishopPieceSquareMiddleGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandMajorParameterCount; ++i)
        reg.push_back({"RookPieceSquareMiddleGame_" + std::to_string(i), "PieceSquare", &Option::CandRookPieceSquareMiddleGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandMajorParameterCount; ++i)
        reg.push_back({"QueenPieceSquareMiddleGame_" + std::to_string(i), "PieceSquare", &Option::CandQueenPieceSquareMiddleGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandKingParameterCount; ++i)
        reg.push_back({"KingPieceSquareMiddleGame_" + std::to_string(i), "PieceSquare", &Option::KingPieceSquareMiddleGameParameters[i]});

    for (int i = 0; i < PieceSquareModel::CandPawnParameterCount; ++i)
        reg.push_back({"PawnPieceSquareEndGame_" + std::to_string(i), "PieceSquare", &Option::CandPawnPieceSquareEndGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandKnightParameterCount; ++i)
        reg.push_back({"KnightPieceSquareEndGame_" + std::to_string(i), "PieceSquare", &Option::CandKnightPieceSquareEndGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandBishopParameterCount; ++i)
        reg.push_back({"BishopPieceSquareEndGame_" + std::to_string(i), "PieceSquare", &Option::CandBishopPieceSquareEndGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandMajorParameterCount; ++i)
        reg.push_back({"RookPieceSquareEndGame_" + std::to_string(i), "PieceSquare", &Option::CandRookPieceSquareEndGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandMajorParameterCount; ++i)
        reg.push_back({"QueenPieceSquareEndGame_" + std::to_string(i), "PieceSquare", &Option::CandQueenPieceSquareEndGameParameters[i]});
    for (int i = 0; i < PieceSquareModel::CandKingParameterCount; ++i)
        reg.push_back({"KingPieceSquareEndGame_" + std::to_string(i), "PieceSquare", &Option::KingPieceSquareEndGameParameters[i]});

    // 5. KnightMobility (9)
    reg.push_back({"KnightMobilityMiddleGameBase", "KnightMobility", &Option::KnightMobilityMiddleGameParameters[0]});
    for (int i = 1; i < 4; ++i)
        reg.push_back({"KnightMobilityMiddleGameIncrement_" + std::to_string(i), "KnightMobility", &Option::KnightMobilityMiddleGameParameters[i]});
    reg.push_back({"KnightMobilityEndGameBase", "KnightMobility", &Option::KnightMobilityEndGameParameters[0]});
    for (int i = 1; i < 4; ++i)
        reg.push_back({"KnightMobilityEndGameIncrement_" + std::to_string(i), "KnightMobility", &Option::KnightMobilityEndGameParameters[i]});

    // 6. BishopMobility (14)
    reg.push_back({"BishopMobilityMiddleGameBase", "BishopMobility", &Option::BishopMobilityMiddleGameParameters[0]});
    for (int i = 1; i < 5; ++i)
        reg.push_back({"BishopMobilityMiddleGameIncrement_" + std::to_string(i), "BishopMobility", &Option::BishopMobilityMiddleGameParameters[i]});
    reg.push_back({"BishopMobilityEndGameBase", "BishopMobility", &Option::BishopMobilityEndGameParameters[0]});
    for (int i = 1; i < 5; ++i)
        reg.push_back({"BishopMobilityEndGameIncrement_" + std::to_string(i), "BishopMobility", &Option::BishopMobilityEndGameParameters[i]});

    // 7. RookMobility (15)
    reg.push_back({"RookMobilityMiddleGameBase", "RookMobility", &Option::RookMobilityMiddleGameParameters[0]});
    for (int i = 1; i < 5; ++i)
        reg.push_back({"RookMobilityMiddleGameIncrement_" + std::to_string(i), "RookMobility", &Option::RookMobilityMiddleGameParameters[i]});
    reg.push_back({"RookMobilityEndGameBase", "RookMobility", &Option::RookMobilityEndGameParameters[0]});
    for (int i = 1; i < 5; ++i)
        reg.push_back({"RookMobilityEndGameIncrement_" + std::to_string(i), "RookMobility", &Option::RookMobilityEndGameParameters[i]});

    // 8. QueenMobility (8: Candidate 3 exact-preserving model)
    reg.push_back({"QueenMobilityMiddleGameBase", "QueenMobility", &Option::CandQueenMobilityMiddleGameParameters[0]});
    for (int i = 1; i < 4; ++i)
        reg.push_back({"QueenMobilityMiddleGameIncrement_" + std::to_string(i), "QueenMobility", &Option::CandQueenMobilityMiddleGameParameters[i]});
    reg.push_back({"QueenMobilityEndGameBase", "QueenMobility", &Option::CandQueenMobilityEndGameParameters[0]});
    for (int i = 1; i < 4; ++i)
        reg.push_back({"QueenMobilityEndGameIncrement_" + std::to_string(i), "QueenMobility", &Option::CandQueenMobilityEndGameParameters[i]});

    // 9. Attack (11: 10 Threat tiers + AttackEndgameMultiplierPercent)
    static int Threat_PawnOnMinor_MG = 20;
    static int Threat_PawnOnMajor_MG = 84;
    static int Threat_MinorOnPawn_MG = 7;
    static int Threat_MinorOnMinor_MG = 24;
    static int Threat_MinorOnMajor_MG = 41;
    static int Threat_RookOnPawn_MG = -1;
    static int Threat_RookOnMinor_MG = 15;
    static int Threat_RookOnQueen_MG = 24;
    static int Threat_QueenOnPawn_MG = 3;
    static int Threat_QueenOnPiece_MG = 10;
    reg.push_back({"Threat_PawnOnMinor_MG", "Attack", &Threat_PawnOnMinor_MG});
    reg.push_back({"Threat_PawnOnMajor_MG", "Attack", &Threat_PawnOnMajor_MG});
    reg.push_back({"Threat_MinorOnPawn_MG", "Attack", &Threat_MinorOnPawn_MG});
    reg.push_back({"Threat_MinorOnMinor_MG", "Attack", &Threat_MinorOnMinor_MG});
    reg.push_back({"Threat_MinorOnMajor_MG", "Attack", &Threat_MinorOnMajor_MG});
    reg.push_back({"Threat_RookOnPawn_MG", "Attack", &Threat_RookOnPawn_MG});
    reg.push_back({"Threat_RookOnMinor_MG", "Attack", &Threat_RookOnMinor_MG});
    reg.push_back({"Threat_RookOnQueen_MG", "Attack", &Threat_RookOnQueen_MG});
    reg.push_back({"Threat_QueenOnPawn_MG", "Attack", &Threat_QueenOnPawn_MG});
    reg.push_back({"Threat_QueenOnPiece_MG", "Attack", &Threat_QueenOnPiece_MG});
    reg.push_back({"AttackEndgameMultiplierPercent", "Attack", &Option::AttackEndgameMultiplierPercent});

    // 10. Inline (8: 7 Candidate 1 parameters + BishopPairValue)
    reg.push_back({"BishopPairValue", "Inline", &Option::BishopPairValue});
    reg.push_back({"CandBishopOpenFilePawnScale", "Inline", &Option::CandBishopOpenFilePawnScale});
    reg.push_back({"CandTempoMiddleGame", "Inline", &Option::CandTempoMiddleGame});
    reg.push_back({"CandTempoEndGame", "Inline", &Option::CandTempoEndGame});
    reg.push_back({"CandOppositeColorBishopMiddleGameScalePermille", "Inline", &Option::CandOppositeColorBishopMiddleGameScalePermille});
    reg.push_back({"CandOppositeColorBishopEndGameScalePermille", "Inline", &Option::CandOppositeColorBishopEndGameScalePermille});
    reg.push_back({"CandEndgamePawnAdvancementRankMultiplier", "Inline", &Option::CandEndgamePawnAdvancementRankMultiplier});
    reg.push_back({"CandPieceAttackScalePercent", "Inline", &Option::CandPieceAttackScalePercent});

    // 11. RookFile (4: Candidate 1)
    reg.push_back({"CandRookOpenFileMiddleGame", "RookFile", &Option::CandRookOpenFileMiddleGame});
    reg.push_back({"CandRookOpenFileEndGame", "RookFile", &Option::CandRookOpenFileEndGame});
    reg.push_back({"CandRookSemiOpenFileMiddleGame", "RookFile", &Option::CandRookSemiOpenFileMiddleGame});
    reg.push_back({"CandRookSemiOpenFileEndGame", "RookFile", &Option::CandRookSemiOpenFileEndGame});

    // 12. KnightOutpost (4)
    reg.push_back({"KnightOutpostMiddleGame", "KnightOutpost", &Option::KnightOutpostMiddleGame});
    reg.push_back({"KnightOutpostEndGame", "KnightOutpost", &Option::KnightOutpostEndGame});
    reg.push_back({"KnightSupportedOutpostMiddleGame", "KnightSupportedOutpostMiddleGame", &Option::KnightSupportedOutpostMiddleGame});
    reg.push_back({"KnightSupportedOutpostEndGame", "KnightSupportedOutpostEndGame", &Option::KnightSupportedOutpostEndGame});

    // 13. IsolatedPawn (2)
    reg.push_back({"IsolatedPawnMiddleGame", "IsolatedPawn", &Option::IsolatedPawnMiddleGame});
    reg.push_back({"IsolatedPawnEndGame", "IsolatedPawn", &Option::IsolatedPawnEndGame});

    // 14. RookBehindPassedPawn (2)
    reg.push_back({"RookBehindPassedPawnMiddleGame", "RookBehindPassedPawn", &Option::RookBehindPassedPawnMiddleGame});
    reg.push_back({"RookBehindPassedPawnEndGame", "RookBehindPassedPawn", &Option::RookBehindPassedPawnEndGame});

    // 15. EndgameWeights (3: Candidate 3)
    reg.push_back({"CandLoneKingPushWeight", "EndgameWeights", &Option::CandLoneKingPushWeight});
    reg.push_back({"CandLoneKingConfinementWeight", "EndgameWeights", &Option::CandLoneKingConfinementWeight});
    reg.push_back({"CandLoneKingRestrictedNeighbourWeight", "EndgameWeights", &Option::CandLoneKingRestrictedNeighbourWeight});

    // 16. KingSafety (17: Candidate 3)
    reg.push_back({"CandKingAttackerMinorWeight", "KingSafety", &Option::CandKingAttackerMinorWeight});
    reg.push_back({"CandKingAttackerRookWeight", "KingSafety", &Option::CandKingAttackerRookWeight});
    reg.push_back({"CandKingAttackerQueenWeight", "KingSafety", &Option::CandKingAttackerQueenWeight});
    reg.push_back({"CandKingDefenderPawnWeight", "KingSafety", &Option::CandKingDefenderPawnWeight});
    reg.push_back({"CandKingDefenderMinorWeight", "KingSafety", &Option::CandKingDefenderMinorWeight});
    reg.push_back({"CandKingDefenderRookWeight", "KingSafety", &Option::CandKingDefenderRookWeight});
    reg.push_back({"CandKingDefenderQueenWeight", "KingSafety", &Option::CandKingDefenderQueenWeight});
    reg.push_back({"CandKingShelterSecondRankDanger", "KingSafety", &Option::CandKingShelterSecondRankDanger});
    reg.push_back({"CandKingShelterAdvancedPawnDanger", "KingSafety", &Option::CandKingShelterAdvancedPawnDanger});
    reg.push_back({"CandKingShelterMissingPawnDanger", "KingSafety", &Option::CandKingShelterMissingPawnDanger});
    reg.push_back({"CandKingUndefendedZoneDanger", "KingSafety", &Option::CandKingUndefendedZoneDanger});
    reg.push_back({"CandKingAdditionalZoneAttackerDanger", "KingSafety", &Option::CandKingAdditionalZoneAttackerDanger});
    reg.push_back({"CandKingSemiOpenLineDanger", "KingSafety", &Option::CandKingSemiOpenLineDanger});
    reg.push_back({"CandKingOpenLineDanger", "KingSafety", &Option::CandKingOpenLineDanger});
    reg.push_back({"CandKingDiagonalLineDanger", "KingSafety", &Option::CandKingDiagonalLineDanger});
    reg.push_back({"CandKingControlledEscapeDanger", "KingSafety", &Option::CandKingControlledEscapeDanger});
    reg.push_back({"CandKingInfiltratedQueenWeight", "KingSafety", &Option::CandKingInfiltratedQueenWeight});

    return reg;
}

// Regenerate derived runtime tables when a parameter changes
void RegenerateTablesForParam(const std::string& family, const std::string& name)
{
    if (family == "PassedPawnV2")
    {
        PassedPawnV2::Generate(Option::PassedPawnMiddleGameParameters,
                               Option::PassedPawnMiddleGameFileAmplitude,
                               Option::PassedPawnEndGameParameters,
                               Option::WhitePassedPawnValueMiddleGam,
                               Option::WhitePassedPawnValueEndGame);
        for (int sq = 0; sq < 64; ++sq)
        {
            int mSq = (7 - (sq / 8)) * 8 + (sq % 8);
            Option::BlackPassedPawnValueMiddleGam[sq] = Option::WhitePassedPawnValueMiddleGam[mSq];
            Option::BlackPassedPawnValueEndGam[sq] = Option::WhitePassedPawnValueEndGame[mSq];
        }
    }
    else if (family == "PieceSquare")
    {
        auto mirrorSquare = [](int sq) { return (7 - (sq / 8)) * 8 + (sq % 8); };
        if (name.find("Pawn") != std::string::npos)
        {
            PieceSquareModel::GenerateCandPawn(Option::CandPawnPieceSquareMiddleGameParameters, Option::PawnInValueWhiteMiddleGame);
            PieceSquareModel::GenerateCandPawn(Option::CandPawnPieceSquareEndGameParameters, Option::PawnInValueWhiteEndGame);
            for (int sq = 0; sq < 64; ++sq)
            {
                Option::PawnInValueWhite[0][sq] = Option::PawnInValueWhiteMiddleGame[sq];
                Option::PawnInValueWhite[2][sq] = Option::PawnInValueWhiteEndGame[sq];
                Option::PawnInValueBlack[0][sq] = Option::PawnInValueWhiteMiddleGame[mirrorSquare(sq)];
                Option::PawnInValueBlack[2][sq] = Option::PawnInValueWhiteEndGame[mirrorSquare(sq)];
            }
        }
        else if (name.find("Knight") != std::string::npos)
        {
            PieceSquareModel::GenerateCandMinor(Option::CandKnightPieceSquareMiddleGameParameters, Option::KnightInValueWhiteMiddleGame);
            PieceSquareModel::GenerateCandMinor(Option::CandKnightPieceSquareEndGameParameters, Option::KnightInValueWhiteEndGame);
            for (int sq = 0; sq < 64; ++sq)
            {
                Option::KnightInValueWhite[0][sq] = Option::KnightInValueWhiteMiddleGame[sq];
                Option::KnightInValueWhite[2][sq] = Option::KnightInValueWhiteEndGame[sq];
                Option::KnightInValueBlack[0][sq] = Option::KnightInValueWhiteMiddleGame[mirrorSquare(sq)];
                Option::KnightInValueBlack[2][sq] = Option::KnightInValueWhiteEndGame[mirrorSquare(sq)];
            }
        }
        else if (name.find("Bishop") != std::string::npos)
        {
            PieceSquareModel::GenerateMinor(Option::CandBishopPieceSquareMiddleGameParameters, Option::BishopInValueWhiteMiddleGame);
            PieceSquareModel::GenerateMinor(Option::CandBishopPieceSquareEndGameParameters, Option::BishopInValueWhiteEndGame);
            for (int sq = 0; sq < 64; ++sq)
            {
                Option::BishopInValueWhite[0][sq] = Option::BishopInValueWhiteMiddleGame[sq];
                Option::BishopInValueWhite[2][sq] = Option::BishopInValueWhiteEndGame[sq];
                Option::BishopInValueBlack[0][sq] = Option::BishopInValueWhiteMiddleGame[mirrorSquare(sq)];
                Option::BishopInValueBlack[2][sq] = Option::BishopInValueWhiteEndGame[mirrorSquare(sq)];
            }
        }
        else if (name.find("Rook") != std::string::npos)
        {
            PieceSquareModel::GenerateCandMajor(Option::CandRookPieceSquareMiddleGameParameters, Option::RookInValueWhiteMiddleGame);
            PieceSquareModel::GenerateCandMajor(Option::CandRookPieceSquareEndGameParameters, Option::RookInValueWhiteEndGame);
            for (int sq = 0; sq < 64; ++sq)
            {
                Option::RookInValueWhite[0][sq] = Option::RookInValueWhiteMiddleGame[sq];
                Option::RookInValueWhite[2][sq] = Option::RookInValueWhiteEndGame[sq];
                Option::RookInValueBlack[0][sq] = Option::RookInValueWhiteMiddleGame[mirrorSquare(sq)];
                Option::RookInValueBlack[2][sq] = Option::RookInValueWhiteEndGame[mirrorSquare(sq)];
            }
        }
        else if (name.find("Queen") != std::string::npos)
        {
            PieceSquareModel::GenerateCandMajor(Option::CandQueenPieceSquareMiddleGameParameters, Option::QueenInValueWhiteMiddleGame);
            PieceSquareModel::GenerateCandMajor(Option::CandQueenPieceSquareEndGameParameters, Option::QueenInValueWhiteEndGame);
            for (int sq = 0; sq < 64; ++sq)
            {
                Option::QueenInValueWhite[0][sq] = Option::QueenInValueWhiteMiddleGame[sq];
                Option::QueenInValueWhite[2][sq] = Option::QueenInValueWhiteEndGame[sq];
                Option::QueenInValueBlack[0][sq] = Option::QueenInValueWhiteMiddleGame[mirrorSquare(sq)];
                Option::QueenInValueBlack[2][sq] = Option::QueenInValueWhiteEndGame[mirrorSquare(sq)];
            }
        }
        else if (name.find("King") != std::string::npos)
        {
            PieceSquareModel::GenerateKing(Option::KingPieceSquareMiddleGameParameters, Option::KingInValueWhiteMiddleGame);
            PieceSquareModel::GenerateKing(Option::KingPieceSquareEndGameParameters, Option::KingInValueWhiteEndGame);
            for (int sq = 0; sq < 64; ++sq)
            {
                Option::KingInValueWhite[0][sq] = Option::KingInValueWhiteMiddleGame[sq];
                Option::KingInValueWhite[2][sq] = Option::KingInValueWhiteEndGame[sq];
                Option::KingInValueBlack[0][sq] = Option::KingInValueWhiteMiddleGame[mirrorSquare(sq)];
                Option::KingInValueBlack[2][sq] = Option::KingInValueWhiteEndGame[mirrorSquare(sq)];
            }
        }
    }
    else if (family == "KnightMobility")
    {
        MobilityV2::GenerateKnight(Option::KnightMobilityMiddleGameParameters, Option::KnightMoveCountValueMiddleGame);
        MobilityV2::GenerateKnight(Option::KnightMobilityEndGameParameters, Option::KnightMoveCountValueEndGame);
        for (int i = 0; i < 9; ++i)
        {
            Option::KnightMoveCountValue[0][i] = Option::KnightMoveCountValueMiddleGame[i];
            Option::KnightMoveCountValue[2][i] = Option::KnightMoveCountValueEndGame[i];
        }
    }
    else if (family == "BishopMobility")
    {
        MobilityV2::GenerateBishop(Option::BishopMobilityMiddleGameParameters, Option::BishopMoveCountValueMiddleGame);
        MobilityV2::GenerateBishop(Option::BishopMobilityEndGameParameters, Option::BishopMoveCountValueEndGame);
        for (int i = 0; i < 14; ++i)
        {
            Option::BishopMoveCountValue[0][i] = Option::BishopMoveCountValueMiddleGame[i];
            Option::BishopMoveCountValue[2][i] = Option::BishopMoveCountValueEndGame[i];
        }
    }
    else if (family == "RookMobility")
    {
        MobilityV2::GenerateRook(Option::RookMobilityMiddleGameParameters, Option::RookMoveCountValueMiddleGame);
        MobilityV2::GenerateRook(Option::RookMobilityEndGameParameters, Option::RookMoveCountValueEndGame);
        for (int i = 0; i < 15; ++i)
        {
            Option::RookMoveCountValue[0][i] = Option::RookMoveCountValueMiddleGame[i];
            Option::RookMoveCountValue[2][i] = Option::RookMoveCountValueEndGame[i];
        }
    }
    else if (family == "QueenMobility")
    {
        MobilityV2::GenerateCandQueenMiddleGame(Option::CandQueenMobilityMiddleGameParameters, Option::QueenMoveCountValueMiddleGame);
        MobilityV2::GenerateCandQueenEndGame(Option::CandQueenMobilityEndGameParameters, Option::QueenMoveCountValueEndGame);
        for (int i = 0; i < 28; ++i)
        {
            Option::QueenMoveCountValue[0][i] = Option::QueenMoveCountValueMiddleGame[i];
            Option::QueenMoveCountValue[2][i] = Option::QueenMoveCountValueEndGame[i];
        }
    }
}
} // namespace

int main(int argc, char* argv[])
{
    try
    {
        std::string corpusPath = "evaluator-analysis/tuning/bishop-pair/train-sample.tsv";
        std::string outputPath = "evaluator-analysis/geometry/train_responses_192.tsv";
        std::string paramListPath = "evaluator-analysis/geometry/parameters_192.tsv";
        std::size_t limit = 0; // 0 means no limit

        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "--corpus" && i + 1 < argc) corpusPath = argv[++i];
            else if (arg == "--output" && i + 1 < argc) outputPath = argv[++i];
            else if (arg == "--param-list" && i + 1 < argc) paramListPath = argv[++i];
            else if (arg == "--limit" && i + 1 < argc) limit = std::stoull(argv[++i]);
        }

        InitializeEngine();
        auto registry = BuildCanonical192Registry();
        if (registry.size() != 192)
        {
            std::cerr << "Error: Registry size is " << registry.size() << " instead of 192\n";
            return 1;
        }

        if (!paramListPath.empty())
        {
            std::ofstream pFile(paramListPath);
            pFile << "param_id\tname\tfamily\tbaseline_value\n";
            for (std::size_t i = 0; i < registry.size(); ++i)
            {
                pFile << i << "\t" << registry[i].name << "\t" << registry[i].family << "\t" << *registry[i].targetPtr << "\n";
            }
            pFile.close();
        }

        std::ifstream corpus(corpusPath);
        if (!corpus.is_open())
        {
            std::cerr << "Failed to open corpus: " << corpusPath << "\n";
            return 1;
        }

        std::string line;
        std::vector<std::string> fens;
        std::vector<std::string> posIds;
        int fenCol = -1, idCol = -1;
        bool header = true;

        while (std::getline(corpus, line))
        {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string col;
            std::vector<std::string> cols;
            while (std::getline(ss, col, '\t')) cols.push_back(col);

            if (header)
            {
                header = false;
                for (std::size_t c = 0; c < cols.size(); ++c)
                {
                    if (cols[c] == "fen") fenCol = c;
                    if (cols[c] == "position_id") idCol = c;
                }
                continue;
            }

            if (fenCol >= 0 && fenCol < (int)cols.size())
            {
                fens.push_back(cols[fenCol]);
                posIds.push_back(idCol >= 0 && idCol < (int)cols.size() ? cols[idCol] : "pos_" + std::to_string(fens.size()));
            }

            if (limit > 0 && fens.size() >= limit) break;
        }
        corpus.close();

        std::cout << "Loaded " << fens.size() << " positions from " << corpusPath << "\n";
        std::ofstream out(outputPath);
        out << "position_id\tfen";
        for (std::size_t j = 0; j < registry.size(); ++j)
            out << "\t" << registry[j].name;
        out << "\n";

        for (std::size_t posIdx = 0; posIdx < fens.size(); ++posIdx)
        {
            std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fens[posIdx]));
            if (!board)
            {
                std::cerr << "Warning: Failed to make board for FEN: " << fens[posIdx] << "\n";
                continue;
            }

            out << posIds[posIdx] << "\t" << fens[posIdx];

            for (std::size_t paramIdx = 0; paramIdx < registry.size(); ++paramIdx)
            {
                const auto& p = registry[paramIdx];
                int* ptr = p.targetPtr;
                int originalVal = *ptr;

                // Symmetric perturbation around baseline: v0 - 1 and v0 + 1
                // 1. Evaluate at v0 - 1
                *ptr = originalVal - 1;
                RegenerateTablesForParam(p.family, p.name);
                EvaluationLogic::ClearEvalCacheForTesting();
                board->ZobristHashCode = 1000 + paramIdx * 2;
                int evalMinus = EvaluationLogic::Evaluate(*board);

                // 2. Evaluate at v0 + 1
                *ptr = originalVal + 1;
                RegenerateTablesForParam(p.family, p.name);
                EvaluationLogic::ClearEvalCacheForTesting();
                board->ZobristHashCode = 1000 + paramIdx * 2 + 1;
                int evalPlus = EvaluationLogic::Evaluate(*board);

                // Restore original value
                *ptr = originalVal;
                RegenerateTablesForParam(p.family, p.name);

                // Symmetrically perturbed response
                int featureResponse = evalPlus - evalMinus;
                out << "\t" << featureResponse;
            }
            out << "\n";

            if ((posIdx + 1) % 100 == 0 || posIdx + 1 == fens.size())
            {
                std::cout << "Evaluated " << (posIdx + 1) << " / " << fens.size() << " positions...\n" << std::flush;
            }
        }

        out.close();
        std::cout << "Successfully generated " << outputPath << "\n";
        CleanupEngine();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}
