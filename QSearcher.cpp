#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "QSearcher.h"
#include "Search.h"
#include "Option.h"
#include "BoardLogic.h"
#include "EvaluationLogic.h"
#include "MoveLogic.h"
#include "UCI.h"
#include "GameLogic.h"
#include "RepetitionHistory.h"
#include "ChessStringManipulation.h"
#include "MateScore.h"
#include "TranspositionTable.h"

namespace {
constexpr int QSearchLimitCheckMask = 2047;
constexpr int QSearchCheckExtensionLimit = 2;
constexpr int QSearchFrontierDistance = 1;
constexpr int QSearchDeepResolutionDistance = 5;
thread_local int qSearchPoolDepth = 0;

class QSearchMovePoolScope
{
public:
    QSearchMovePoolScope()
        : outermost(qSearchPoolDepth++ == 0)
    {
        if (outermost)
            Move::SetPoolEnabled(true);
    }

    ~QSearchMovePoolScope()
    {
        qSearchPoolDepth--;
        if (outermost)
            Move::SetPoolEnabled(false);
    }

private:
    bool outermost;
};
}

int QSearcher::pieceValue100[15] = {
    0,    // pieceValue100[0]
    100,  // pieceValue100[1]
    350,  // pieceValue100[2]
    350,  // pieceValue100[3]
    550,  // pieceValue100[4]
    975,  // pieceValue100[5]
    2500, // pieceValue100[6]
    0,    // pieceValue100[7]
    0,    // pieceValue100[8]
    100,  // pieceValue100[9]
    350,  // pieceValue100[10]
    350,  // pieceValue100[11]
    550,  // pieceValue100[12]
    975,  // pieceValue100[13]
    2500, // pieceValue100[14]
};

#ifdef HOWL_CORRECTNESS_TESTING
namespace {
QSearchTestStatistics qSearchTestStatistics;
}

void QSearcher::ResetTestStatistics() {
    qSearchTestStatistics = {};
}

QSearchTestStatistics QSearcher::TestStatistics() {
    return qSearchTestStatistics;
}
#endif

MovePrintValue* QSearcher::QSearch(bool isPVNode, int alpha, int beta, Move& prevMove, int depthGone, int lastCheck, bool kick, int depth, Move& move1, Move& move2, Move& move3, Board& board4, bool MAtESearch, int depthQuisStarted, bool nullWindowSearch)
{
    if (Search::stopRequested.load(std::memory_order_relaxed)) {
        MovePrintValue* aborted = new MovePrintValue();
        aborted->bound = SearchBound::Upper;
        aborted->MarkSpeculative(SearchProvenance::Aborted);
        return aborted;
    }
    QSearchMovePoolScope movePoolScope;
    const int origAlpha = alpha;
    const int origBeta = beta;
    const int qsearchDistance = std::max(0, depthGone - depthQuisStarted);
    Search::searchNodeCount++;
    if ((Search::searchNodeCount & QSearchLimitCheckMask) == 0 ||
        (Search::maxNodes > 0 && Search::searchNodeCount >= Search::maxNodes))
    {
        Search::CheckLimits();
    }
    if (Search::stopRequested.load(std::memory_order_relaxed))
    {
        MovePrintValue* abortVal = new MovePrintValue();
        abortVal->value = 0;
        abortVal->bound = SearchBound::Upper;
        abortVal->MarkSpeculative(SearchProvenance::Aborted);
        return abortVal;
    }
    MoveList moveList;
    Move* SelectedMove = nullptr;
    Move selectedMoveStorage{};
    Board* boardCopy;
    int extention = Option::checkExtensionNonPV;
    if (isPVNode) {
        extention = Option::checkExtension;
    }

    int staticEval = 0;
    bool staticEvalKnown = false;
    const auto evaluate = [&]() {
        if (!staticEvalKnown) {
            staticEval = EvaluationLogic::Evaluate(board4);
            staticEvalKnown = true;
        }
        return staticEval;
    };
    
    MovePrintValue* retValue = new MovePrintValue();
    retValue->printString = "";
    retValue->AddProvenance(SearchProvenance::Quiescence);
    
    MovePrintValue* MPValue = nullptr;
    
    int turn;
    if (!board4.sideToMove) {
        turn = 0;
    } else {
        turn = 1;
    }
    
    bool currentSideInCheck = BoardLogic::UnderAttack(
        board4,
        board4.pieces[turn * 8 + 6].front(),
        !board4.sideToMove);

    if (BoardLogic::UnderAttack(board4, board4.pieces[(1 - turn) * 8 + 6].front(), board4.sideToMove)) {
        retValue->value = 160000;
        retValue->MarkSpeculative(SearchProvenance::InvalidMove);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
    
    if (currentSideInCheck && lastCheck < QSearchCheckExtensionLimit &&
        depthGone - depthQuisStarted < extention) {
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        MovePrintValue* extended = QSearch(isPVNode, alpha, beta, prevMove, depthGone, 2, false, 2, move1, move2, move3, board4, MAtESearch, depthQuisStarted, nullWindowSearch);
        // PV-only checking extensions use a different frontier from a scout.
        // Their scores cannot certify a bound for a non-PV TT probe.
        if (isPVNode && qsearchDistance >= Option::checkExtensionNonPV)
            extended->MarkSpeculative(SearchProvenance::ExtendedFrontier);
        return extended;
    }

    if (currentSideInCheck && depth == 0) {
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        return QSearch(isPVNode, alpha, beta, prevMove, depthGone, 1, false, 1, move1, move2, move3, board4, MAtESearch, depthQuisStarted, nullWindowSearch);
    }
    
    int nextLastCheck = 0;
    bool checkChecked = false;
    if (lastCheck > 0) {
        checkChecked = true;
        nextLastCheck = lastCheck - 1;
    } else if (kick) {
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        return QSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, false, 1, move1, move2, move3, board4, MAtESearch, depthQuisStarted, nullWindowSearch);
    }

    const bool frontierPhase = qsearchDistance <= QSearchFrontierDistance;
    const bool deepResolutionPhase = qsearchDistance >= QSearchDeepResolutionDistance;
    const uint8_t phaseState = frontierPhase ? 0 : (deepResolutionPhase ? 2 : 1);
    const uint8_t modeState = currentSideInCheck ? QTT_EVASION
        : (checkChecked ? QTT_CHECK_SEQUENCE : QTT_CAPTURE_ONLY);
    const uint8_t qSearchState = static_cast<uint8_t>(
        modeState | (phaseState << 2) | (isPVNode ? 0x10 : 0));

    QSearchTTEntry qTTEntry;
    const bool qTTHit = TranspositionTable::ProbeQSearch(
        board4.ZobristHashCode, qTTEntry);
    if (qTTHit && qTTEntry.staticEvalValid) {
        staticEval = qTTEntry.staticEval;
        staticEvalKnown = true;
    }

    const bool qTTCompatible = qTTHit && qTTEntry.state == qSearchState &&
        qTTEntry.depth >= depth;
    int qTTScore = 0;
    uint8_t qTTBase = TT_NONE;
    if (qTTCompatible) {
        qTTScore = MateScore::FromTranspositionTable(qTTEntry.score, depthGone);
        qTTBase = TTBaseFlag(qTTEntry.flag);
    }

    if (!isPVNode && qTTCompatible && TTFlagIsRigorous(qTTEntry.flag) &&
        TranspositionTable::CutoffsEnabled() &&
        (qTTBase == TT_EXACT ||
         (qTTBase == TT_LOWER_BOUND && qTTScore >= beta) ||
         (qTTBase == TT_UPPER_BOUND && qTTScore <= alpha))) {
        retValue->value = qTTScore;
        retValue->bound = qTTBase == TT_LOWER_BOUND ? SearchBound::Lower
            : (qTTBase == TT_UPPER_BOUND ? SearchBound::Upper : SearchBound::Exact);
        retValue->SetProof(qTTBase != TT_UPPER_BOUND, qTTBase != TT_LOWER_BOUND);
        retValue->selective = true;
        TranspositionTable::RecordQSearchCutoff();
        return retValue;
    }

    const auto storeQResult = [&](const MovePrintValue& result, uint16_t bestMove) {
        constexpr uint16_t unsafeProvenance =
            static_cast<uint16_t>(SearchProvenance::Repetition) |
            static_cast<uint16_t>(SearchProvenance::Aborted) |
            static_cast<uint16_t>(SearchProvenance::InvalidMove);
        if ((result.provenance & unsafeProvenance) != 0)
            return;
        TranspositionTable::StoreQSearch(
            board4.ZobristHashCode,
            MateScore::ToTranspositionTable(result.value, depthGone),
            static_cast<int8_t>(std::max(-128, std::min(127, depth))),
            qSearchState, TTFlagForResult(result), bestMove,
            staticEval, staticEvalKnown);
    };

    if (depth == 0) {
        retValue->value = evaluate();
        retValue->bound = retValue->value <= origAlpha ? SearchBound::Upper
            : (retValue->value >= origBeta ? SearchBound::Lower : SearchBound::Exact);
        retValue->selective = true;
        storeQResult(*retValue, 0);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
    int valueTemp2 = -200000;
    bool standPatUsesTTLower = false;
    if (!checkChecked && !currentSideInCheck) {
        valueTemp2 = evaluate();
        if (qTTCompatible && TTFlagIsRigorous(qTTEntry.flag) &&
            (qTTBase == TT_EXACT || qTTBase == TT_LOWER_BOUND)) {
            standPatUsesTTLower = qTTBase == TT_LOWER_BOUND && qTTScore > valueTemp2;
            valueTemp2 = std::max(valueTemp2, qTTScore);
        }
        if (valueTemp2 >= beta) {
            retValue->value = valueTemp2;
            retValue->bound = SearchBound::Lower;
            retValue->proof = LowerProof;
            retValue->selective = true;
            storeQResult(*retValue, 0);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
    // Stand pat is a legal QSearch alternative outside forced check resolution.
    // A wider window must preserve the lower bound returned by a stand-pat cutoff.
    const bool canStandPat = !checkChecked && !currentSideInCheck;
    int bestMoveValue = canStandPat ? valueTemp2 : -200000;
    bool allUpperProof = !standPatUsesTTLower;
    bool bestLowerProof = canStandPat;
    if (canStandPat && valueTemp2 > alpha) alpha = valueTemp2;
    DeferredMove deferredMoves[256];
    int deferredCount = 0;
    bool hasDeferredStage2 = false;
    if (!currentSideInCheck) {
        moveList = MoveLogic::QSearchStage1Generator(
            board4, depth, depthGone, deferredMoves, deferredCount, prevMove,
            frontierPhase, deepResolutionPhase);
        hasDeferredStage2 = (deferredCount > 0);
    } else {
        moveList = MoveLogic::MoveGenerator(board4, depth, depthGone, false);
    }

#ifdef HOWL_CORRECTNESS_TESTING
    const bool testRootNode = depthGone == 0 && depthQuisStarted == 0;
    if (testRootNode) {
        qSearchTestStatistics.rootGeneratedMoves = moveList.count + deferredCount;
    }
#endif
    std::string SelectedPV;
    int availMoves = 0;
    int standPot = (!checkChecked && !currentSideInCheck) ? valueTemp2 : evaluate();
    bool firstMove = true;

    int currentStage = 1;
    while (true) {
        for (int i = 0; i < moveList.count; ++i) {
            if (Search::stopRequested.load(std::memory_order_relaxed)) {
                deleteMoveList(moveList);
                if (hasDeferredStage2 && currentStage == 1) {
                    for (int d = 0; d < deferredCount; ++d) {
                        delete deferredMoves[d].templateMove;
                    }
                }
                delete MPValue;
                MPValue = nullptr;
                retValue->value = 0;
                retValue->bound = SearchBound::Upper;
                retValue->MarkSpeculative(SearchProvenance::Aborted);
                return retValue;
            }
            Move* move = moveList.moves[i];
            uint8_t moveProof = NoProof;
            boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
            MissingInfoAboutPrevStateFromMove missingInfoAboutPrevStateFromMove(board4, *move);

            GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone, &missingInfoAboutPrevStateFromMove);
            bool legalMove = !BoardLogic::UnderAttack(
                board4,
                board4.pieces[turn * 8 + 6].front(),
                board4.sideToMove);

            if (!legalMove) {
#ifdef HOWL_CORRECTNESS_TESTING
                if (testRootNode && firstMove) {
                    qSearchTestStatistics.rootIllegalMovesBeforeFirstSearch++;
                }
#endif
                GameLogic::UndoMove(board4, *move, missingInfoAboutPrevStateFromMove);
                if (UCI::IsTest()) {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                continue;
            }

            availMoves++;
#ifdef HOWL_CORRECTNESS_TESTING
            if (testRootNode) {
                qSearchTestStatistics.rootLegalMoves++;
            }
#endif
            bool moveGivesCheck = BoardLogic::UnderAttack(
                board4,
                board4.pieces[(1 - turn) * 8 + 6].front(),
                !board4.sideToMove);



            int pieceValueTemp = pieceValue100[move->endPiece];
            int promotionGain = (move->promotionPiece > 0)
                ? (pieceValue100[move->promotionPiece] - pieceValue100[1])
                : 0;

            bool deltaRejectsMaterial = (!currentSideInCheck &&
                Option::SafetyMargin + standPot + pieceValueTemp + promotionGain <= alpha);

#ifdef HOWL_CORRECTNESS_TESTING
            if (testRootNode && moveGivesCheck && deltaRejectsMaterial) {
                qSearchTestStatistics.checkingMovesExemptedFromDelta++;
            }
#endif

            bool isDeltaPruned = deltaRejectsMaterial && !moveGivesCheck;

            if (isDeltaPruned) {
                allUpperProof = false;
                retValue->AddProvenance(SearchProvenance::ForwardPruning);
                move->value = Option::SafetyMargin + standPot + pieceValueTemp + promotionGain - 1;
                if (move->value > bestMoveValue) {
                    bestMoveValue = move->value;
                    bestLowerProof = false;
                    SelectedMove = move;
                    SelectedPV = "";
                }
                GameLogic::UndoMove(board4, *move, missingInfoAboutPrevStateFromMove);
                if (UCI::IsTest()) {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                continue;
            }

            if (firstMove) {
#ifdef HOWL_CORRECTNESS_TESTING
                if (testRootNode) {
                    qSearchTestStatistics.firstLegalSearchedMoveUsedFullWindow = true;
                }
#endif
                int value;
                std::string movePV = "";
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode)) {
                    value = 0;
                    move->value = 0;
                    retValue->AddProvenance(SearchProvenance::Repetition);
                } else {
                    if (move->endPiece > 0 || move->promotionPiece > 0) {
                        delete MPValue;
                        MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, true, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                        moveProof = InvertProof(MPValue->proof);
                        retValue->provenance |= MPValue->provenance;
                        movePV = MPValue->printString;
                    } else {
                        delete MPValue;
                        MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, false, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                        moveProof = InvertProof(MPValue->proof);
                        retValue->provenance |= MPValue->provenance;
                        movePV = MPValue->printString;
                    }
                    move->value = value;
                }
                allUpperProof = allUpperProof && ((moveProof & UpperProof) != 0);
                if (value > bestMoveValue) {
                    bestMoveValue = value;
                    bestLowerProof = (moveProof & LowerProof) != 0;
                    SelectedMove = move;
                    SelectedPV = movePV;
                }
                GameLogic::UndoMove(board4, *move, missingInfoAboutPrevStateFromMove);
                if (UCI::IsTest()) {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                if (Search::stopRequested.load(std::memory_order_relaxed)) {
                    deleteMoveList(moveList);
                    if (hasDeferredStage2 && currentStage == 1) {
                        for (int d = 0; d < deferredCount; ++d) {
                            delete deferredMoves[d].templateMove;
                        }
                    }
                    delete MPValue;
                    MPValue = nullptr;
                    retValue->value = 0;
                    retValue->bound = SearchBound::Upper;
                    retValue->MarkSpeculative(SearchProvenance::Aborted);
                    return retValue;
                }
                firstMove = false;
                if (value > alpha) {
                    if (value >= beta) {
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + movePV;
                        retValue->value = value;
                        retValue->bound = SearchBound::Lower;
                        retValue->SetProof((moveProof & LowerProof) != 0, false);
                        retValue->selective = true;
                        storeQResult(*retValue, TTMoveHelper::PackMove(*move));
                        deleteMoveList(moveList);
                        if (hasDeferredStage2 && currentStage == 1) {
                            for (int d = 0; d < deferredCount; ++d) {
                                delete deferredMoves[d].templateMove;
                            }
                        }
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    alpha = value;
                }
            }
            else
            {
                bool tempRepeat = false;
                int value;
                std::string movePV = "";
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode)) {
                    tempRepeat = true;
                    value = 0;
                    move->value = 0;
                    retValue->AddProvenance(SearchProvenance::Repetition);
                } else {
                    if (move->endPiece > 0 || move->promotionPiece > 0) {
                        delete MPValue;
                        MPValue = QSearch(false, -alpha - Option::nullWindowSize, -alpha, *move, depthGone + 1, nextLastCheck, true, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                        moveProof = InvertProof(MPValue->proof);
                        retValue->provenance |= MPValue->provenance;
                        movePV = MPValue->printString;
                    } else {
                        delete MPValue;
                        MPValue = QSearch(false, -alpha - Option::nullWindowSize, -alpha, *move, depthGone + 1, nextLastCheck, false, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                        moveProof = InvertProof(MPValue->proof);
                        retValue->provenance |= MPValue->provenance;
                        movePV = MPValue->printString;
                    }
                    move->value = value;
                }
                const bool scoutProvesCutoff = !isPVNode && !tempRepeat &&
                    beta - alpha == Option::nullWindowSize &&
                    MPValue->ProvesUpper(-beta);
                if (value > alpha /* && value < beta */) {
                    if (!tempRepeat && !scoutProvesCutoff) {
#ifdef HOWL_CORRECTNESS_TESTING
                        if (testRootNode) {
                            qSearchTestStatistics.rootFullWindowResearches++;
                        }
#endif
                        if (move->endPiece > 0 || move->promotionPiece > 0) {
                            delete MPValue; 
                            MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, true, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                            value = -MPValue->value;
                            moveProof = InvertProof(MPValue->proof);
                            retValue->provenance |= MPValue->provenance;
                            movePV = MPValue->printString;
                        } else {
                            delete MPValue;
                            MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, false, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                            value = -MPValue->value;
                            moveProof = InvertProof(MPValue->proof);
                            retValue->provenance |= MPValue->provenance;
                            movePV = MPValue->printString;
                        }
                        move->value = value;
                    }
                    if (value > alpha) {
                        alpha = value;
                    }
                }
                GameLogic::UndoMove(board4, *move, missingInfoAboutPrevStateFromMove);
                if (UCI::IsTest()) {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                if (Search::stopRequested.load(std::memory_order_relaxed)) {
                    deleteMoveList(moveList);
                    if (hasDeferredStage2 && currentStage == 1) {
                        for (int d = 0; d < deferredCount; ++d) {
                            delete deferredMoves[d].templateMove;
                        }
                    }
                    delete MPValue;
                    MPValue = nullptr;
                    retValue->value = 0;
                    retValue->bound = SearchBound::Upper;
                    retValue->MarkSpeculative(SearchProvenance::Aborted);
                    return retValue;
                }
                allUpperProof = allUpperProof && ((moveProof & UpperProof) != 0);
                if (value > bestMoveValue) {
                    if (value >= beta) {
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + movePV;
                        retValue->value = value;
                        retValue->bound = SearchBound::Lower;
                        retValue->SetProof((moveProof & LowerProof) != 0, false);
                        retValue->selective = true;
                        storeQResult(*retValue, TTMoveHelper::PackMove(*move));
                        deleteMoveList(moveList);
                        if (hasDeferredStage2 && currentStage == 1) {
                            for (int d = 0; d < deferredCount; ++d) {
                                delete deferredMoves[d].templateMove;
                            }
                        }
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    bestMoveValue = value;
                    bestLowerProof = (moveProof & LowerProof) != 0;
                    SelectedMove = move;
                    SelectedPV = movePV;
                }
            }
        }

        // If Stage 1 completed without beta cutoff, materialize Stage 2
        if (currentStage == 1 && hasDeferredStage2) {
            if (SelectedMove != nullptr) {
                selectedMoveStorage = *SelectedMove;
                SelectedMove = &selectedMoveStorage;
            }
            deleteMoveList(moveList);
            moveList = MoveLogic::MaterializeStage2(board4, depth, depthGone, deferredMoves, deferredCount);
            currentStage = 2;
            hasDeferredStage2 = false;
            continue;
        }

        break;
    }

#ifdef HOWL_CORRECTNESS_TESTING
    if (testRootNode) {
        qSearchTestStatistics.rootAvailableMoves = availMoves;
    }
#endif
    if (availMoves == 0 && !currentSideInCheck) {
        bool hasLegalMove = MoveLogic::HasAnyLegalMove(board4, prevMove, depthGone);

        retValue->value = hasLegalMove ? standPot : 0;
        retValue->bound = retValue->value <= origAlpha ? SearchBound::Upper
            : (retValue->value >= origBeta ? SearchBound::Lower : SearchBound::Exact);
        retValue->selective = true;
        storeQResult(*retValue, 0);
        deleteMoveList(moveList);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    } else if (availMoves == 0 && currentSideInCheck) {
        Move mateMove;
        mateMove.value = MateScore::MatedAtPly(depthGone);
        retValue->value = MateScore::MatedAtPly(depthGone);
        storeQResult(*retValue, 0);
        deleteMoveList(moveList);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    } else {
        retValue->value = bestMoveValue;
        retValue->SetProof(bestLowerProof, allUpperProof);
        retValue->bound = bestMoveValue <= origAlpha ? SearchBound::Upper
            : (bestMoveValue >= origBeta ? SearchBound::Lower : SearchBound::Exact);
        retValue->selective = true;
        retValue->printString = (SelectedMove != nullptr) ? (ChessStringManipulation::PVToString(*SelectedMove, 0, false, board4) + ' ' + SelectedPV) : "";
        const uint16_t packedBestMove = SelectedMove != nullptr
            ? TTMoveHelper::PackMove(*SelectedMove) : 0;
        storeQResult(*retValue, packedBestMove);
        deleteMoveList(moveList);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
}

void QSearcher::deleteMoveList(MoveList moveList) {
    for (int i = 0; i < moveList.count; ++i) {
        delete moveList.moves[i];
    }
}
