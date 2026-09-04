#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "EvaluationLogic.h"
#include "Option.h"
#include "KingSetup.h"
#include "AttackPlaces.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include <algorithm>
#include <iostream>

namespace
{
bool RooksAreConnected(int firstRook, int secondRook, long long occupiedSquares)
{
    return (AttackPlaces::RookAttack[firstRook][secondRook] & occupiedSquares) ==
        Option::PowerTwo[secondRook];
}

bool HasConnectedRooks(const MyList& rooks, long long occupiedSquares)
{
    const int rookCount = rooks.size();
    if (rookCount < 2)
    {
        return false;
    }

    if (RooksAreConnected(rooks[0], rooks[1], occupiedSquares))
    {
        return true;
    }
    if (rookCount == 2)
    {
        return false;
    }

    for (int first = 0; first < rookCount - 1; first++)
    {
        const int firstUncheckedSecond = first == 0 ? 2 : first + 1;
        for (int second = firstUncheckedSecond; second < rookCount; second++)
        {
            if (RooksAreConnected(rooks[first], rooks[second], occupiedSquares))
            {
                return true;
            }
        }
    }
    return false;
}

int RookConnectionValue(MyList (&pieces)[15], long long occupiedSquares)
{
    int value = 0;
    if (HasConnectedRooks(pieces[4], occupiedSquares))
    {
        value += 30;
    }
    if (HasConnectedRooks(pieces[12], occupiedSquares))
    {
        value -= 30;
    }
    return value;
}

struct KingDangerResult
{
    int danger = 0;
    int attackerWeight = 0;
    int defenderWeight = 0;
    int escapeSafety = 0;
    int filePressure = 0;
    int diagonalPressure = 0;
    int pawnShelter = 0;
    int phaseScale = 0;
};

bool IsInsideBoard(int rank, int file)
{
    return rank >= 0 && rank < 8 && file >= 0 && file < 8;
}

bool PieceAttacksSquare(Board& board, int pieceType, bool white,
                        int from, int target)
{
    const int fromRank = from / 8;
    const int fromFile = from % 8;
    const int targetRank = target / 8;
    const int targetFile = target % 8;
    const int rankDistance = targetRank - fromRank;
    const int fileDistance = targetFile - fromFile;

    if (pieceType == 1)
    {
        return rankDistance == (white ? 1 : -1) && std::abs(fileDistance) == 1;
    }
    if (pieceType == 2)
    {
        return (std::abs(rankDistance) == 2 && std::abs(fileDistance) == 1) ||
               (std::abs(rankDistance) == 1 && std::abs(fileDistance) == 2);
    }
    if (pieceType == 6)
    {
        return std::max(std::abs(rankDistance), std::abs(fileDistance)) == 1;
    }

    const bool diagonal = std::abs(rankDistance) == std::abs(fileDistance);
    const bool straight = rankDistance == 0 || fileDistance == 0;
    if ((pieceType == 3 && !diagonal) || (pieceType == 4 && !straight) ||
        (pieceType == 5 && !diagonal && !straight) || from == target)
    {
        return false;
    }

    const int rankStep = (rankDistance > 0) - (rankDistance < 0);
    const int fileStep = (fileDistance > 0) - (fileDistance < 0);
    int rank = fromRank + rankStep;
    int file = fromFile + fileStep;
    while (rank != targetRank || file != targetFile)
    {
        if (board.mainBoard[rank * 8 + file] != 0)
        {
            return false;
        }
        rank += rankStep;
        file += fileStep;
    }
    return true;
}

int CountSideAttacks(Board& board, bool white, int target)
{
    int attackers = 0;
    const int firstPiece = white ? 1 : 9;
    for (int boardPiece = firstPiece; boardPiece < firstPiece + 6; boardPiece++)
    {
        const int pieceType = white ? boardPiece : boardPiece - 8;
        for (int from : board.pieces[boardPiece])
        {
            if (PieceAttacksSquare(board, pieceType, white, from, target))
            {
                attackers++;
            }
        }
    }
    return attackers;
}

std::vector<int> KingZone(int kingSquare)
{
    std::vector<int> zone;
    const int kingRank = kingSquare / 8;
    const int kingFile = kingSquare % 8;
    zone.push_back(kingSquare);
    for (int rankOffset = -1; rankOffset <= 1; rankOffset++)
    {
        for (int fileOffset = -1; fileOffset <= 1; fileOffset++)
        {
            if (rankOffset == 0 && fileOffset == 0)
            {
                continue;
            }
            const int rank = kingRank + rankOffset;
            const int file = kingFile + fileOffset;
            if (IsInsideBoard(rank, file))
            {
                zone.push_back(rank * 8 + file);
            }
        }
    }
    return zone;
}

bool PieceParticipatesInZone(Board& board, int boardPiece, int square,
                             const std::vector<int>& zone)
{
    const bool white = boardPiece < 8;
    const int pieceType = white ? boardPiece : boardPiece - 8;
    for (int target : zone)
    {
        if (PieceAttacksSquare(board, pieceType, white, square, target))
        {
            return true;
        }
    }
    return false;
}

int ShelterDanger(Board& board, bool whiteKing, int kingSquare)
{
    const int direction = whiteKing ? 1 : -1;
    const int kingRank = kingSquare / 8;
    const int kingFile = kingSquare % 8;
    MyList& friendlyPawns = board.pieces[whiteKing ? 1 : 9];
    MyList& enemyPawns = board.pieces[whiteKing ? 9 : 1];
    int danger = 0;

    for (int file = std::max(0, kingFile - 1); file <= std::min(7, kingFile + 1); file++)
    {
        bool firstRankPawn = false;
        bool secondRankPawn = false;
        bool fartherPawn = false;
        for (int pawn : friendlyPawns)
        {
            if (pawn % 8 != file)
            {
                continue;
            }
            const int advance = (pawn / 8 - kingRank) * direction;
            firstRankPawn |= advance == 1;
            secondRankPawn |= advance == 2;
            fartherPawn |= advance > 2;
        }
        if (firstRankPawn)
        {
            continue;
        }
        danger += secondRankPawn ? 5 : (fartherPawn ? 10 : 16);

        bool fileHasEnemyPawn = false;
        for (int pawn : enemyPawns)
        {
            fileHasEnemyPawn |= pawn % 8 == file;
        }
        if (!secondRankPawn && !fartherPawn && !fileHasEnemyPawn)
        {
            danger += 7;
        }
    }
    return danger;
}

KingDangerResult EvaluateKingDanger(Board& board, bool whiteKing)
{
    static constexpr int attackerWeight[7] = {0, 2, 5, 5, 8, 12, 0};
    static constexpr int defenderWeight[7] = {0, 2, 4, 4, 5, 7, 0};
    const int kingSquare = board.pieces[whiteKing ? 6 : 14].front();
    const bool attackingWhite = !whiteKing;
    const std::vector<int> zone = KingZone(kingSquare);
    const int attackerFirst = attackingWhite ? 1 : 9;
    const int defenderFirst = whiteKing ? 1 : 9;
    int attackerParticipation = 0;
    int defenderParticipation = 0;
    int attackerCount = 0;
    int defenderCount = 0;

    for (int boardPiece = attackerFirst; boardPiece < attackerFirst + 5; boardPiece++)
    {
        const int pieceType = attackingWhite ? boardPiece : boardPiece - 8;
        for (int square : board.pieces[boardPiece])
        {
            if (PieceParticipatesInZone(board, boardPiece, square, zone))
            {
                attackerParticipation += attackerWeight[pieceType];
                attackerCount++;
            }
        }
    }
    for (int boardPiece = defenderFirst; boardPiece < defenderFirst + 5; boardPiece++)
    {
        const int pieceType = whiteKing ? boardPiece : boardPiece - 8;
        for (int square : board.pieces[boardPiece])
        {
            if (PieceParticipatesInZone(board, boardPiece, square, zone))
            {
                defenderParticipation += defenderWeight[pieceType];
                defenderCount++;
            }
        }
    }
    const int kingRank = kingSquare / 8;
    const int kingFile = kingSquare % 8;
    int safeEscapes = 0;
    int controlledEscapes = 0;
    int occupiedEscapes = 0;
    int edgeDirections = 0;
    for (int rankOffset = -1; rankOffset <= 1; rankOffset++)
    {
        for (int fileOffset = -1; fileOffset <= 1; fileOffset++)
        {
            if (rankOffset == 0 && fileOffset == 0)
            {
                continue;
            }
            const int rank = kingRank + rankOffset;
            const int file = kingFile + fileOffset;
            if (!IsInsideBoard(rank, file))
            {
                edgeDirections++;
                continue;
            }
            const int target = rank * 8 + file;
            const int occupant = board.mainBoard[target];
            const bool occupiedByDefender = occupant != 0 && (occupant < 8) == whiteKing;
            const bool enemyControlled = CountSideAttacks(board, attackingWhite, target) != 0;
            if (occupiedByDefender)
            {
                occupiedEscapes++;
            }
            else if (enemyControlled)
            {
                controlledEscapes++;
            }
            else
            {
                safeEscapes++;
            }

        }
    }
    int filePressure = 0;
    for (int file = std::max(0, kingFile - 1); file <= std::min(7, kingFile + 1); file++)
    {
        bool friendlyPawn = false;
        bool enemyPawn = false;
        for (int pawn : board.pieces[whiteKing ? 1 : 9]) friendlyPawn |= pawn % 8 == file;
        for (int pawn : board.pieces[whiteKing ? 9 : 1]) enemyPawn |= pawn % 8 == file;
        const int openness = !friendlyPawn ? (!enemyPawn ? 2 : 1) : 0;
        if (openness == 0)
        {
            continue;
        }
        for (int pieceType : {4, 5})
        {
            const int boardPiece = attackingWhite ? pieceType : pieceType + 8;
            for (int square : board.pieces[boardPiece])
            {
                for (int target : zone)
                {
                    if (target % 8 == file && PieceAttacksSquare(board, pieceType, attackingWhite, square, target))
                    {
                        filePressure += openness == 2 ? 10 : 6;
                        break;
                    }
                }
            }
        }
    }
    int diagonalPressure = 0;
    for (int pieceType : {3, 5})
    {
        const int boardPiece = attackingWhite ? pieceType : pieceType + 8;
        for (int square : board.pieces[boardPiece])
        {
            if (PieceAttacksSquare(board, pieceType, attackingWhite, square, kingSquare))
            {
                diagonalPressure += 9;
            }
        }
    }
    const int escapeDanger = controlledEscapes * 6 + occupiedEscapes * 2 +
                             edgeDirections * 2 + std::max(0, 3 - safeEscapes) * 8;
    const int balanceDanger = std::max(0, attackerParticipation - defenderParticipation) * 2 +
                              std::max(0, attackerCount - defenderCount) * 4;
    const int shelterDanger = ShelterDanger(board, whiteKing, kingSquare);
    const int lineDanger = filePressure + diagonalPressure;
    int rawDanger = attackerParticipation * 2 + escapeDanger +
                    lineDanger + shelterDanger + balanceDanger;

    const int queenCount = board.pieces[attackingWhite ? 5 : 13].size();
    const int rookCount = board.pieces[attackingWhite ? 4 : 12].size();
    const int minorCount = board.pieces[attackingWhite ? 2 : 10].size() +
                           board.pieces[attackingWhite ? 3 : 11].size();
    const int attackingMaterialScale = std::min(100, 20 + queenCount * 45 +
                                                     rookCount * 12 + minorCount * 5);
    rawDanger = rawDanger * attackingMaterialScale / 100;
    const int escalatedDanger = rawDanger + rawDanger * rawDanger / 180;
    KingDangerResult result{std::min(escalatedDanger, 450), attackerParticipation,
            defenderParticipation, escapeDanger, filePressure,
            diagonalPressure, shelterDanger, attackingMaterialScale};
    return result;
}
}

EvaluationChessCache EvaluationLogic::EvalCache;
PawnCache EvaluationLogic::PawnEvalCache;

std::size_t EvaluationLogic::EvalCacheSize()
{
    return EvalCache.size();
}

std::size_t EvaluationLogic::PawnEvalCacheSize()
{
    return PawnEvalCache.size();
}

std::size_t EvaluationLogic::EvalCacheCapacityBytes()
{
    return EvalCache.capacityBytes();
}

std::size_t EvaluationLogic::EvalCacheEntryCapacity()
{
    return EvalCache.entryCapacity();
}

std::size_t EvaluationLogic::EvalCacheClusterCount()
{
    return EvalCache.clusterCount();
}

EvaluationCacheStatistics EvaluationLogic::EvalCacheStats()
{
    return EvalCache.statistics();
}

void EvaluationLogic::ResetEvalCacheStats()
{
    EvalCache.resetStatistics();
}

bool EvaluationLogic::ResizeEvalCache(std::size_t capacityBytes)
{
    return EvalCache.resize(capacityBytes);
}

#if HOWL_CORRECTNESS_TESTING
void EvaluationLogic::SetEvalCacheAllocationFailureThresholdForTesting(
    std::size_t capacityBytes)
{
    EvaluationChessCache::SetAllocationFailureThresholdForTesting(capacityBytes);
}
#endif

#if HOWL_CORRECTNESS_TESTING
void EvaluationLogic::ClearEvalCacheForTesting()
{
    EvalCache.clear();
}

int EvaluationLogic::RookConnectionValueForTesting(Board& board)
{
    return RookConnectionValue(board.pieces, board.whitePieces | board.blackPieces);
}
#endif

int EvaluationLogic::CalculatePhase(const Board &thisBoard)
{
    const auto &pieces = thisBoard.pieces;
    int phase = pieces[2].size() * 1 + pieces[3].size() * 1 + pieces[4].size() * 2 + pieces[5].size() * 4
              + pieces[10].size() * 1 + pieces[11].size() * 1 + pieces[12].size() * 2 + pieces[13].size() * 4;
    return std::clamp(phase, 0, 24);
}

namespace
{
int EvaluateInternal(Board &thisBoard, EvaluationBreakdown *breakdown)
{
    long long piecesBinary = thisBoard.whitePieces | thisBoard.blackPieces;
    MyList (&pieces)[15] = thisBoard.pieces;

    int whitePieceEvaluation = pieces[1].size() * Option::PawnValue + pieces[2].size() * Option::KnightValue + pieces[3].size() * (Option::BishopValue + (8 - (pieces[1].size() + pieces[9].size())) * 2) + pieces[4].size() * Option::RookValue + pieces[5].size() * Option::QueenValue;

    int blackPieceEvaluation = pieces[9].size() * Option::PawnValue + pieces[10].size() * Option::KnightValue + pieces[11].size() * (Option::BishopValue + (8 - (pieces[1].size() + pieces[9].size())) * 2) + pieces[12].size() * Option::RookValue + pieces[13].size() * Option::QueenValue;
    double pieceBalance = 1;
    if (whitePieceEvaluation > blackPieceEvaluation)
    {
        pieceBalance = (whitePieceEvaluation + 1500) / (blackPieceEvaluation + 1500);
    }
    else if (whitePieceEvaluation < blackPieceEvaluation)
    {
        pieceBalance = (blackPieceEvaluation + 1500) / (whitePieceEvaluation + 1500);
    }

    if (whitePieceEvaluation > blackPieceEvaluation)
    {
        if (pieces[1].size() == 0)
        {
            pieceBalance *= .7;
        }
        if (pieces[1].size() == 1)
        {
            pieceBalance *= .9;
        }
    }
    else if (whitePieceEvaluation < blackPieceEvaluation)
    {
        if (pieces[9].size() == 0)
        {
            pieceBalance *= .7;
        }
        if (pieces[9].size() == 1)
        {
            pieceBalance *= .9;
        }
    }

    int pieceEvaluation = (int)((whitePieceEvaluation - blackPieceEvaluation) * pieceBalance);
    // Bishop pair
    int whiteBishopPair = 0;
    int blackBishopPair = 0;
    if (pieces[3].size() == 2 && ((pieces[3][0] / 8 + pieces[3][0] % 8) % 2) != ((pieces[3][1] / 8 + pieces[3][1] % 8) % 2))    
    {
        whiteBishopPair = 50;
    }
    if (pieces[11].size() == 2 && ((pieces[11][0] / 8 + pieces[11][0] % 8) % 2) != ((pieces[11][1] / 8 + pieces[11][1] % 8) % 2))
    {
        blackBishopPair = 50;
    }
    int bishopPairVaue = whiteBishopPair - blackBishopPair;

    // Movement
    int state = 0;
    if (whitePieceEvaluation <= 1700 || blackPieceEvaluation <= 1700 || whitePieceEvaluation + blackPieceEvaluation <= 4400)
    {
        state = 2;
    }

    int *movementAndKingSafety = EvaluationLogic::PieceMoveCount(thisBoard, state);
    int movement = movementAndKingSafety[0];
    int kingSafety = movementAndKingSafety[1];
    int center = movementAndKingSafety[2];
    delete[] movementAndKingSafety;

    int phase = EvaluationLogic::CalculatePhase(thisBoard);

    KingDangerResult whiteKingDanger = EvaluateKingDanger(thisBoard, true);
    KingDangerResult blackKingDanger = EvaluateKingDanger(thisBoard, false);
    int kingDangerNet = blackKingDanger.danger - whiteKingDanger.danger;

    int whiteKingPlacement = Option::WhiteKingPlaceSafetyMiddleGame[pieces[6].front()];
    int blackKingPlacement = Option::BlackKingPlaceSafetyMiddleGame[pieces[14].front()];
    whiteKingPlacement = whiteKingPlacement * phase / 24;
    blackKingPlacement = blackKingPlacement * phase / 24;
    int kingPlacementNet = whiteKingPlacement - blackKingPlacement;
    kingSafety = kingDangerNet + kingPlacementNet;

    // Pawn Structure
    int pawnStructure = EvaluationLogic::GetPawnStructureValue(thisBoard, state);

    // Rook Connection
    int rookValue = RookConnectionValue(pieces, piecesBinary);
    // Temp
    int temp = 0;
    if (state == 0)
    {
        temp = (!thisBoard.sideToMove) ? 24 : -24;
    }
    else if (state == 2)
    {
        temp = (!thisBoard.sideToMove) ? 11 : -11;
    }

    double oppositeColorBishop = 1;
    if (pieces[3].size() == 1 && pieces[11].size() == 1 && ((pieces[3].front() / 8 + pieces[3].front() % 8) % 2) != ((pieces[11].front() / 8 + pieces[11].front() % 8) % 2))
    {
        if (state == 0)
        {
            oppositeColorBishop = 0.9;
        }
        else if (state == 2)
        {
            oppositeColorBishop = 0.75;
        }
    }

    int unscaled = pieceEvaluation + bishopPairVaue + movement + pawnStructure + kingSafety + rookValue + center + temp;
    int evaluation = (int)(unscaled * oppositeColorBishop); 
    bool drawAdjustment = false;
    if (evaluation > 0)
    {
        if (pieces[1].size() == 0 && pieces[3].size() == 0 && pieces[4].size() == 0 && pieces[5].size() == 0)
        {
            evaluation = 30;
            drawAdjustment = true;
        }
        else if (pieces[1].size() == 0 && pieces[2].size() == 0 && pieces[3].size() < 2 && pieces[4].size() == 0 && pieces[5].size() == 0)
        {
            evaluation = 30;
            drawAdjustment = true;
        }
    }
    else if (evaluation < 0)
    {
        if (pieces[9].size() == 0 && pieces[11].size() == 0 && pieces[12].size() == 0 && pieces[13].size() == 0)
        {
            evaluation = -30;
            drawAdjustment = true;
        }
        else if (pieces[9].size() == 0 && pieces[10].size() == 0 && pieces[11].size() < 2 && pieces[12].size() == 0 && pieces[13].size() == 0)
        {
            evaluation = -30;
            drawAdjustment = true;
        }
    }

    if (breakdown != nullptr)
    {
        breakdown->phase = phase;
        breakdown->state = state;
        breakdown->whiteMaterial = whitePieceEvaluation;
        breakdown->blackMaterial = blackPieceEvaluation;
        breakdown->materialNet = whitePieceEvaluation - blackPieceEvaluation;
        breakdown->pieceBalance = pieceBalance;
        breakdown->pieceEvaluation = pieceEvaluation;

        breakdown->whiteBishopPair = whiteBishopPair;
        breakdown->blackBishopPair = blackBishopPair;
        breakdown->bishopPairNet = bishopPairVaue;

        breakdown->mobilityNet = movement;
        breakdown->centerNet = center;

        breakdown->kingAttackNet = kingDangerNet;
        breakdown->whiteKingPlacement = whiteKingPlacement;
        breakdown->blackKingPlacement = blackKingPlacement;
        breakdown->kingPlacementNet = kingPlacementNet;
        breakdown->whitePawnShield = 0;
        breakdown->blackPawnShield = 0;
        breakdown->pawnShieldNet = 0;
        breakdown->whiteCentralKingExposure = 0;
        breakdown->blackCentralKingExposure = 0;
        breakdown->centralKingExposureNet = 0;
        breakdown->kingSafetyTotal = kingSafety;
        breakdown->whiteKingDanger = whiteKingDanger.danger;
        breakdown->blackKingDanger = blackKingDanger.danger;
        breakdown->whiteAttackerWeight = whiteKingDanger.attackerWeight;
        breakdown->blackAttackerWeight = blackKingDanger.attackerWeight;
        breakdown->whiteDefenderWeight = whiteKingDanger.defenderWeight;
        breakdown->blackDefenderWeight = blackKingDanger.defenderWeight;
        breakdown->whiteEscapeSafety = whiteKingDanger.escapeSafety;
        breakdown->blackEscapeSafety = blackKingDanger.escapeSafety;
        breakdown->whiteFilePressure = whiteKingDanger.filePressure;
        breakdown->blackFilePressure = blackKingDanger.filePressure;
        breakdown->whiteDiagonalPressure = whiteKingDanger.diagonalPressure;
        breakdown->blackDiagonalPressure = blackKingDanger.diagonalPressure;
        breakdown->whitePawnShelter = whiteKingDanger.pawnShelter;
        breakdown->blackPawnShelter = blackKingDanger.pawnShelter;
        breakdown->whitePhaseScale = whiteKingDanger.phaseScale;
        breakdown->blackPhaseScale = blackKingDanger.phaseScale;

        breakdown->pawnStructureNet = pawnStructure;
        breakdown->rookConnectionNet = rookValue;
        breakdown->tempoNet = temp;

        breakdown->oppositeColorBishopScale = oppositeColorBishop;
        breakdown->unscaledTotal = unscaled;
        breakdown->scaledTotal = (int)(unscaled * oppositeColorBishop);
        breakdown->drawAdjustmentApplied = drawAdjustment;
        breakdown->whitePerspectiveTotal = evaluation;
        breakdown->sideToMoveTotal = thisBoard.sideToMove ? -evaluation : evaluation;
    }

    return evaluation;
}
}

int EvaluationLogic::Evaluate(Board &thisBoard)
{
    const std::uint64_t evaluationKey =
        static_cast<std::uint64_t>(thisBoard.ZobristHashCode);
    std::optional<std::int32_t> cacheEvalResult = EvalCache.getFromCache(evaluationKey);

    if (cacheEvalResult.has_value())
    {
        return cacheEvalResult.value();
    }

    int evaluation = EvaluateInternal(thisBoard, nullptr);
    int finalScore = (!thisBoard.sideToMove) ? evaluation : -evaluation;
    EvalCache.addToCache(evaluationKey, static_cast<std::int32_t>(finalScore));
    return finalScore;
}

EvaluationBreakdown EvaluationLogic::EvaluateDetailed(Board &thisBoard)
{
    EvaluationBreakdown breakdown{};
    EvaluateInternal(thisBoard, &breakdown);
    return breakdown;
}

int EvaluationLogic::GetPawnStructureValue(Board &thisBoard, int state)
{
    long long whitePawns = thisBoard.whitePawns;
    long long blackPawns = thisBoard.blackPawns;
    std::vector<std::vector<int>> whitePawnPerColumn(8);
    std::vector<std::vector<int>> blackPawnPerColumn(8);

    for (int counter = 0; counter < 8; counter++)
    {
        whitePawnPerColumn[counter] = std::vector<int>();
        blackPawnPerColumn[counter] = std::vector<int>();
    }

    int phase = CalculatePhase(thisBoard);
    int whitePawnSum;
    std::optional<int> calculatedPawnEval = PawnEvalCache.GetFromCache(whitePawns, 0, state);
    if (calculatedPawnEval.has_value())
    {
        whitePawnSum = calculatedPawnEval.value();
    }
    else
    {
        for (int item : thisBoard.pieces[1])
        {
            whitePawnPerColumn[item % 8].push_back(item);
        }
        int doubledPawnValueWhite = 0;
        for (int counter = 0; counter < 8; counter++)
        {
            if (whitePawnPerColumn[counter].size() > 1)
            {
                doubledPawnValueWhite += Option::DoubledPawnValue * (whitePawnPerColumn[counter].size() - 1);
            }
        }
        int singlePastWhite = 0;
        for (int pawnPlace : thisBoard.pieces[1])
        {
            if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & blackPawns) == 0)
            {
                int mgVal = Option::WhitePassedPawnValueMiddleGam[pawnPlace];
                int egVal = Option::WhitePassedPawnValueEndGame[pawnPlace];
                singlePastWhite += (mgVal * phase + egVal * (24 - phase)) / 24;
            }
        }
        int goForwardPawnWhite = 0;
        if (state == 2)
        {
            for (int pawnPlace : thisBoard.pieces[1])
            {
                goForwardPawnWhite += pawnPlace / 8 * 2;
            }
        }
        whitePawnSum = doubledPawnValueWhite + singlePastWhite + goForwardPawnWhite;
        PawnEvalCache.Add(whitePawns, whitePawnSum, 0, state);
    }
    int blackPawnSum;
    calculatedPawnEval = PawnEvalCache.GetFromCache(blackPawns, 1, state);
    if (calculatedPawnEval.has_value())
    {
        blackPawnSum = calculatedPawnEval.value();
    }
    else
    {
        for (int item : thisBoard.pieces[9])
        {
            blackPawnPerColumn[item % 8].push_back(item);
        }
        int doubledPawnValueBlack = 0;
        for (int counter = 0; counter < 8; counter++)
        {
            if (blackPawnPerColumn[counter].size() > 1)
            {
                doubledPawnValueBlack += Option::DoubledPawnValue * (blackPawnPerColumn[counter].size() - 1);
            }
        }
        int singlePastBlack = 0;

        for (int pawnPlace : thisBoard.pieces[9])
        {
            if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & whitePawns) == 0)
            {
                int mgVal = Option::BlackPassedPawnValueMiddleGam[pawnPlace];
                int egVal = Option::BlackPassedPawnValueEndGam[pawnPlace];
                singlePastBlack += (mgVal * phase + egVal * (24 - phase)) / 24;
            }
        }
        int goForwardPawnBlack = 0;
        if (state == 2)
        {
            for (int pawnPlace : thisBoard.pieces[9])
            {
                goForwardPawnBlack += (7 - (pawnPlace / 8)) * 2;
            }
        }
        blackPawnSum = doubledPawnValueBlack + singlePastBlack + goForwardPawnBlack;
        PawnEvalCache.Add(blackPawns, blackPawnSum, 1, state);
    }
    whitePawnSum -= blackPawnSum;
    return whitePawnSum;
}

int *EvaluationLogic::PieceMoveCount(Board &thisBoard, int state)
{
    // int pieceMovePosition[2][3][64][7];
    int whitePieceAttack[64] = {0};
    int blackPieceAttack[64] = {0};
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int *mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;
    // int MovementMiddleGame = 0;
    // int MovementEndGame = 0;
    int Movement[3] = {0};
    // int KingSafetyMiddleGame[2][64];
    // int KingSafetyEndGame[2][64];
    int KingSafety[3][2][64] = {{{0}}};
    int centerValue = 0;
    int moveCount;
    {
        for (int piece = 1; piece < 7; piece++)
        {
            switch (piece)
            {
            case 1:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] += Option::PawnInValueWhite[state][piecePoisiion];
                    KingSafety[state][0][piecePoisiion] += Option::PieceArroundTheKing[state][piece];
                    centerValue += Option::PawnInCenterValueWhite[piecePoisiion];
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][0] != nullptr)
                    {
                        if ((PieceMoves::pawnTwoMove[piecePoisiion] & wholeBoard) == 0)
                        {
                            // pieceMovePosition[0][1][piecePoisiion + 16][piece]++;
                            moveCount++;
                            // Movement[state] += Option::PawnMoveValueWhite[state][piecePoisiion + 16];
                            KingSafety[state][0][piecePoisiion + 16] += Option::PieceAttackArroundTheKing[state][piece];
                            // whitePieceAttack
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][1] != nullptr && (Option::PowerTwo[piecePoisiion + 8] & wholeBoard) == 0)
                    {
                        // pieceMovePosition[0][1][piecePoisiion + 8][piece]++;
                        moveCount++;
                        // Movement[state] += Option::PawnMoveValueWhite[state][piecePoisiion + 8];
                        KingSafety[state][0][piecePoisiion + 8] += Option::PieceAttackArroundTheKing[state][piece];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][6] != nullptr && piecePoisiion + 7 == thisBoard.unpassentPlace)
                    {
                        // pieceMovePosition[0][2][piecePoisiion + 7][piece]++;
                        Movement[state] += Option::PawnAttackValue[state][9];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][7] != nullptr && piecePoisiion + 9 == thisBoard.unpassentPlace)
                    {
                        // pieceMovePosition[0][2][piecePoisiion + 9][piece]++;
                        Movement[state] += Option::PawnAttackValue[state][9];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][8] != nullptr && (Option::PowerTwo[piecePoisiion + 7] & blackPieces) != 0)
                    {
                        // pieceMovePosition[0][2][piecePoisiion + 7][piece]++;
                        Movement[state] += Option::PawnAttackValue[state][mainBoard[piecePoisiion + 7]];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][13] != nullptr && (Option::PowerTwo[piecePoisiion + 9] & blackPieces) != 0)
                    {
                        // pieceMovePosition[0][2][piecePoisiion + 9][piece]++;
                        Movement[state] += Option::PawnAttackValue[state][mainBoard[piecePoisiion + 9]];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][8] != nullptr)
                    {
                        centerValue += Option::PawnMoveCenterValueWhite[piecePoisiion + 7];
                        KingSafety[state][0][piecePoisiion + 7] += Option::PieceAttackArroundTheKing[state][piece];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][13] != nullptr)
                    {
                        centerValue += Option::PawnMoveCenterValueWhite[piecePoisiion + 9];
                        KingSafety[state][0][piecePoisiion + 9] += Option::PieceAttackArroundTheKing[state][piece];
                    }
                    Movement[state] += Option::PawnMoveCountValue[state][moveCount];
                }
                break;
            case 2:
                static const int knightOffsets[8] = {17, 10, 15, 6, -10, -17, -15, -6};
                static const int knightDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] += Option::KnightInValueWhite[state][piecePoisiion];
                    KingSafety[state][0][piecePoisiion] += Option::PieceArroundTheKing[state][piece];
                    centerValue += Option::KnightInCenterValueWhite[piecePoisiion];
                    for (int i = 0; i < 8; ++i)
                    {
                        int endPlace = piecePoisiion + knightOffsets[i];
                        int dir = knightDirs[i];
                        if (PieceMoves::KnightMoves[piecePoisiion][dir] != nullptr)
                        {
                            centerValue += Option::KnightMoveCenterValueWhite[endPlace];
                            KingSafety[state][0][endPlace] += Option::PieceAttackArroundTheKing[state][piece];
                            if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                            {
                                Movement[state] += Option::KnightAttackValue[state][mainBoard[endPlace]];
                            }
                        }
                    }
                    Movement[state] += Option::KnightMoveCountValue[state][moveCount];
                }
                break;
            case 3:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] += Option::BishopInValueWhite[state][piecePoisiion];
                    KingSafety[state][0][piecePoisiion] += Option::PieceArroundTheKing[state][piece];
                    centerValue += Option::BishopInCenterValueWhite[piecePoisiion];
                    for (int direction = 0; direction <= 6; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::BishopMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::BishopMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue += Option::BishopMoveCenterValueWhite[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                moveCount++;
                                KingSafety[state][0][endPos] += Option::PieceAttackArroundTheKing[state][piece];
                            }
                            else if ((Option::PowerTwo[endPos] & blackPieces) != 0)
                            {
                                Movement[state] += Option::BishopAttackValue[state][endPiece];
                                KingSafety[state][0][endPos] += Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                            else
                            {
                                KingSafety[state][0][endPos] += Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                        }
                    }
                    Movement[state] += Option::BishopMoveCountValue[state][moveCount];
                }
                break;
            case 4:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] += Option::RookInValueWhite[state][piecePoisiion];
                    KingSafety[state][0][piecePoisiion] += Option::PieceArroundTheKing[state][piece];
                    centerValue += Option::RookInCenterValueWhite[piecePoisiion];
                    for (int direction = 0; direction <= 6; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::RookMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::RookMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue += Option::RookMoveCenterValueWhite[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                moveCount++;
                                KingSafety[state][0][endPos] += Option::PieceAttackArroundTheKing[state][piece];
                            }
                            else if ((Option::PowerTwo[endPos] & blackPieces) != 0)
                            {
                                Movement[state] += Option::RookAttackValue[state][endPiece];
                                KingSafety[state][0][endPos] += Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                            else
                            {
                                KingSafety[state][0][endPos] += Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                        }
                    }
                    Movement[state] += Option::RookMoveCountValue[state][moveCount];
                }
                break;
            case 5:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] += Option::QueenInValueWhite[state][piecePoisiion];
                    KingSafety[state][0][piecePoisiion] += Option::PieceArroundTheKing[state][piece];
                    centerValue += Option::QueenInCenterValueWhite[piecePoisiion];
                    for (int direction = 0; direction <= 14; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::QueenMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::QueenMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue += Option::QueenMoveCenterValueWhite[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                moveCount++;
                                KingSafety[state][0][endPos] += Option::PieceAttackArroundTheKing[state][piece];
                            }
                            else if ((Option::PowerTwo[endPos] & blackPieces) != 0)
                            {
                                Movement[state] += Option::QueenAttackValue[state][endPiece];
                                KingSafety[state][0][endPos] += Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                            else
                            {
                                KingSafety[state][0][endPos] += Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                        }
                    }
                    Movement[state] += Option::QueenMoveCountValue[state][moveCount];
                }
                break;
            case 6:
                static const int kingOffsets[8] = {7, 8, 9, 1, -7, -8, -9, -1};
                static const int kingDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] += Option::KingInValueWhite[state][piecePoisiion];
                    centerValue += Option::KingInCenterValueWhite[piecePoisiion];
                    for (int i = 0; i < 8; ++i)
                    {
                        int endPlace = piecePoisiion + kingOffsets[i];
                        int dir = kingDirs[i];
                        if (PieceMoves::WhiteKingMoves[piecePoisiion][dir] != nullptr)
                        {
                            centerValue += Option::KingMoveCenterValueWhite[endPlace];
                            if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                            {
                                Movement[state] += Option::KingAttackValue[state][mainBoard[endPlace]];
                            }
                        }
                    }
                    Movement[state] += Option::KingMoveCountValue[state][moveCount];
                }
                break;
            }
        }
    }
    {
        for (int piece = 9; piece < 15; piece++)
        {
            switch (piece)
            {
            case 9:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] -= Option::PawnInValueBlack[state][piecePoisiion];
                    KingSafety[state][1][piecePoisiion] -= Option::PieceArroundTheKing[state][piece];
                    centerValue -= Option::PawnInCenterValueBlack[piecePoisiion];

                    if (PieceMoves::BlackPawnMoves[piecePoisiion][0] != nullptr)
                    {
                        if ((PieceMoves::pawnTwoMove[piecePoisiion] & wholeBoard) == 0)
                        {
                            // pieceMovePosition[1][1][piecePoisiion - 20][piece - 8]++;
                            moveCount++;
                            // Movement[state] -= Option::PawnMoveValueBlack[state][piecePoisiion - 16];
                            KingSafety[state][1][piecePoisiion - 16] -= Option::PieceAttackArroundTheKing[state][piece];
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][1] != nullptr && (Option::PowerTwo[piecePoisiion - 8] & wholeBoard) == 0)
                    {
                        // pieceMovePosition[1][1][piecePoisiion - 10][piece - 8]++;
                        moveCount++;
                        // Movement[state] -= Option::PawnMoveValueBlack[state][piecePoisiion - 8];
                        KingSafety[state][1][piecePoisiion - 8] -= Option::PieceAttackArroundTheKing[state][piece];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][6] != nullptr && piecePoisiion - 7 == thisBoard.unpassentPlace)
                    {
                        // pieceMovePosition[1][2][piecePoisiion - 7][piece - 8]++;
                        Movement[state] -= Option::PawnAttackValue[state][1];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][7] != nullptr && piecePoisiion - 9 == thisBoard.unpassentPlace)
                    {
                        // pieceMovePosition[1][2][piecePoisiion - 9][piece - 8]++;
                        Movement[state] -= Option::PawnAttackValue[state][1];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][8] != nullptr && (Option::PowerTwo[piecePoisiion - 7] & whitePieces) != 0)
                    {
                        // pieceMovePosition[1][2][piecePoisiion - 7][piece - 8]++;
                        Movement[state] -= Option::PawnAttackValue[state][mainBoard[piecePoisiion - 7]];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][13] != nullptr && (Option::PowerTwo[piecePoisiion - 9] & whitePieces) != 0)
                    {
                        // pieceMovePosition[1][2][piecePoisiion - 9][piece - 8]++;
                        Movement[state] -= Option::PawnAttackValue[state][mainBoard[piecePoisiion - 9]];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][8] != nullptr)
                    {
                        centerValue -= Option::PawnMoveCenterValueBlack[piecePoisiion - 7];
                        KingSafety[state][1][piecePoisiion - 7] -= Option::PieceAttackArroundTheKing[state][piece];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][13] != nullptr)
                    {
                        centerValue -= Option::PawnMoveCenterValueBlack[piecePoisiion - 9];
                        KingSafety[state][1][piecePoisiion - 9] -= Option::PieceAttackArroundTheKing[state][piece];
                    }
                    Movement[state] -= Option::PawnMoveCountValue[state][moveCount];
                }
                break;
            case 10:
                static const int knightOffsets[8] = {17, 10, 15, 6, -10, -17, -15, -6};
                static const int knightDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] -= Option::KnightInValueBlack[state][piecePoisiion];
                    KingSafety[state][1][piecePoisiion] -= Option::PieceArroundTheKing[state][piece];
                    centerValue -= Option::KnightInCenterValueBlack[piecePoisiion];
                    for (int i = 0; i < 8; ++i)
                    {
                        int endPlace = piecePoisiion + knightOffsets[i];
                        int dir = knightDirs[i];
                        if (PieceMoves::KnightMoves[piecePoisiion][dir] != nullptr)
                        {
                            centerValue -= Option::KnightMoveCenterValueBlack[endPlace];
                            KingSafety[state][1][endPlace] -= Option::PieceAttackArroundTheKing[state][piece];
                            if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                            {
                                Movement[state] -= Option::KnightAttackValue[state][mainBoard[endPlace]];
                            }
                        }
                    }
                    Movement[state] -= Option::KnightMoveCountValue[state][moveCount];
                }
                break;
            case 11:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] -= Option::BishopInValueBlack[state][piecePoisiion];
                    KingSafety[state][1][piecePoisiion] -= Option::PieceArroundTheKing[state][piece];
                    centerValue -= Option::BishopInCenterValueBlack[piecePoisiion];

                    for (int direction = 0; direction <= 6; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::BishopMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::BishopMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue -= Option::BishopMoveCenterValueBlack[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                // pieceMovePosition[1][1][endPos][piece - 8]++;
                                moveCount++;
                                // Movement[state] -= Option::BishopMoveValueBlack[state][endPos];
                                KingSafety[state][1][endPos] -= Option::PieceAttackArroundTheKing[state][piece];
                            }
                            else if ((Option::PowerTwo[endPos] & whitePieces) != 0)
                            {
                                // pieceMovePosition[1][2][endPos][piece - 8]++;
                                Movement[state] -= Option::BishopAttackValue[state][endPiece];
                                KingSafety[state][1][endPos] -= Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                            else
                            {
                                KingSafety[state][1][endPos] -= Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                        }
                    }
                    Movement[state] -= Option::BishopMoveCountValue[state][moveCount];
                }
                break;
            case 12:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] -= Option::RookInValueBlack[state][piecePoisiion];
                    KingSafety[state][1][piecePoisiion] -= Option::PieceArroundTheKing[state][piece];
                    centerValue -= Option::RookInCenterValueBlack[piecePoisiion];

                    for (int direction = 0; direction <= 6; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::RookMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::RookMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue -= Option::RookMoveCenterValueBlack[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                // pieceMovePosition[1][1][endPos][piece - 8]++;
                                moveCount++;
                                // Movement[state] -= Option::RookMoveValueBlack[state][endPos];
                                KingSafety[state][1][endPos] -= Option::PieceAttackArroundTheKing[state][piece];
                            }
                            else if ((Option::PowerTwo[endPos] & whitePieces) != 0)
                            {
                                // pieceMovePosition[1][2][endPos][piece - 8]++;
                                Movement[state] -= Option::RookAttackValue[state][endPiece];
                                KingSafety[state][1][endPos] -= Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                            else
                            {
                                KingSafety[state][1][endPos] -= Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                        }
                    }
                    Movement[state] -= Option::RookMoveCountValue[state][moveCount];
                }
                break;
            case 13:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] -= Option::QueenInValueBlack[state][piecePoisiion];
                    KingSafety[state][1][piecePoisiion] -= Option::PieceArroundTheKing[state][piece];
                    centerValue -= Option::QueenInCenterValueBlack[piecePoisiion];

                    int directions[] = {0, 2, 4, 6, 8, 10, 12, 14};

                    for (int dir = 0; dir < 8; dir++)
                    {
                        int direction = directions[dir];
                        for (int counter = 0; counter < PieceMoves::QueenMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::QueenMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue -= Option::QueenMoveCenterValueBlack[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                moveCount++;
                                KingSafety[state][1][endPos] -= Option::PieceAttackArroundTheKing[state][piece];
                            }
                            else if ((Option::PowerTwo[endPos] & whitePieces) != 0)
                            {
                                Movement[state] -= Option::QueenAttackValue[state][endPiece];
                                KingSafety[state][1][endPos] -= Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                            else
                            {
                                KingSafety[state][1][endPos] -= Option::PieceAttackArroundTheKing[state][piece];
                                break;
                            }
                        }
                    }
                    Movement[state] -= Option::QueenMoveCountValue[state][moveCount];
                }
                break;
            case 14:
                static const int kingOffsets[8] = {7, 8, 9, 1, -7, -8, -9, -1};
                static const int kingDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    Movement[state] -= Option::KingInValueBlack[state][piecePoisiion];
                    centerValue -= Option::KingInCenterValueBlack[piecePoisiion];
                    for (int i = 0; i < 8; ++i)
                    {
                        int endPlace = piecePoisiion + kingOffsets[i];
                        int dir = kingDirs[i];
                        if (PieceMoves::BlackKingMoves[piecePoisiion][dir] != nullptr)
                        {
                            centerValue -= Option::KingMoveCenterValueBlack[endPlace];
                            if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                            {
                                Movement[state] -= Option::KingAttackValue[state][mainBoard[endPlace]];
                            }
                        }
                    }
                    Movement[state] -= Option::KingMoveCountValue[state][moveCount];
                }
                break;
            }
        }
    }
    int *movementAndKingSafetyAndCenter = new int[3];
    int kingSafety = 0;
    for (int counter = 0; counter < 64; counter++)
    {
        kingSafety += Option::ArroundTheKingDangerMiddleGame[KingSetup::DistanceToKing[thisBoard.pieces[14].front()][counter]] * KingSafety[state][0][counter];
        kingSafety += Option::ArroundTheKingDangerMiddleGame[KingSetup::DistanceToKing[thisBoard.pieces[6].front()][counter]] * KingSafety[state][1][counter];
    }
    movementAndKingSafetyAndCenter[0] = Movement[state];
    movementAndKingSafetyAndCenter[1] = kingSafety;
    movementAndKingSafetyAndCenter[2] = centerValue;
    return movementAndKingSafetyAndCenter;
}
