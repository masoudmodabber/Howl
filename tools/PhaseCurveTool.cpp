#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <cstdint>

#include "BoardMaker.h"
#include "EvaluationLogic.h"
#include "BoardInitializer.h"
#include "Option.h"
#include "AttackPlaces.h"
#include "PieceMoves.h"
#include "MoveLogic.h"
#include "KingSetup.h"
#include "PassedPawnSetup.h"
#include "HashMemoryBudget.h"
#include "UCI.h"

namespace
{

void InitializeEngine()
{
    UCI::IsRelease = true;
    Option::Initialize();
    AttackPlaces::Initialize();
    BoardInitializer::Initialize();
    PieceMoves::Initialize();
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();
    std::ostringstream diagnostics;
    HashMemoryBudget::EnsureDefaultConfigured(diagnostics);
}

struct PositionRecord
{
    int phase;              // 0..24
    int oldBinaryDecision;  // 0 (MG) or 1 (EG)
    int group1_mg;
    int group1_eg;
    int group2_mg;
    int group2_eg;
    int group3_mg;
    int group3_eg;
};

// Evaluate the required terms for a given board position
PositionRecord EvaluateRecord(Board& board)
{
    PositionRecord rec{};
    rec.phase = EvaluationLogic::CalculatePhase(board);

    // Old binary decision:
    // state = 2 (EG) if whitePieceEvaluation <= 1700 || blackPieceEvaluation <= 1700 || total <= 4400
    const int whitePieceEvaluation =
        board.pieces[1].size() * Option::PawnValue +
        board.pieces[2].size() * Option::KnightValue +
        board.pieces[3].size() * (Option::BishopValue + (8 - (board.pieces[1].size() + board.pieces[9].size())) * 2) +
        board.pieces[4].size() * Option::RookValue +
        board.pieces[5].size() * Option::QueenValue;

    const int blackPieceEvaluation =
        board.pieces[9].size() * Option::PawnValue +
        board.pieces[10].size() * Option::KnightValue +
        board.pieces[11].size() * (Option::BishopValue + (8 - (board.pieces[1].size() + board.pieces[9].size())) * 2) +
        board.pieces[12].size() * Option::RookValue +
        board.pieces[13].size() * Option::QueenValue;

    rec.oldBinaryDecision = (whitePieceEvaluation <= 1700 ||
                             blackPieceEvaluation <= 1700 ||
                             whitePieceEvaluation + blackPieceEvaluation <= 4400) ? 1 : 0;

    const long long wholeBoard = board.whitePieces | board.blackPieces;
    const int* mainBoard = board.mainBoard;

    // Group 1: Steady / Fast
    // - Knight mobility
    // - Bishop mobility
    // - Queen mobility
    // - Pawn attacked-piece bonuses
    // - Knight attacked-piece bonuses
    // - Bishop attacked-piece bonuses
    // - Queen attacked-piece bonuses
    // - Tempo
    int g1_white_mg = 0;
    int g1_white_eg = 0;
    int g1_black_mg = 0;
    int g1_black_eg = 0;

    // White Pawns (attacked-piece bonuses)
    for (int sq : board.pieces[1])
    {
        if (PieceMoves::WhitePawnMoves[sq][6] != nullptr && sq + 7 == board.unpassentPlace)
        {
            g1_white_mg += Option::PawnAttackValueMiddleGame[9];
            g1_white_eg += Option::PawnAttackValueEndGame[9];
        }
        if (PieceMoves::WhitePawnMoves[sq][7] != nullptr && sq + 9 == board.unpassentPlace)
        {
            g1_white_mg += Option::PawnAttackValueMiddleGame[9];
            g1_white_eg += Option::PawnAttackValueEndGame[9];
        }
        if (PieceMoves::WhitePawnMoves[sq][8] != nullptr && (Option::PowerTwo[sq + 7] & board.blackPieces) != 0)
        {
            g1_white_mg += Option::PawnAttackValueMiddleGame[mainBoard[sq + 7]];
            g1_white_eg += Option::PawnAttackValueEndGame[mainBoard[sq + 7]];
        }
        if (PieceMoves::WhitePawnMoves[sq][13] != nullptr && (Option::PowerTwo[sq + 9] & board.blackPieces) != 0)
        {
            g1_white_mg += Option::PawnAttackValueMiddleGame[mainBoard[sq + 9]];
            g1_white_eg += Option::PawnAttackValueEndGame[mainBoard[sq + 9]];
        }
    }

    // White Knights (mobility + attacks)
    static const int knightOffsets[8] = {17, 10, 15, 6, -10, -17, -15, -6};
    static const int knightDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
    for (int sq : board.pieces[2])
    {
        int moveCount = 0;
        for (int i = 0; i < 8; ++i)
        {
            int endPlace = sq + knightOffsets[i];
            int dir = knightDirs[i];
            if (PieceMoves::KnightMoves[sq][dir] != nullptr)
            {
                if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                else if ((Option::PowerTwo[endPlace] & board.blackPieces) != 0)
                {
                    g1_white_mg += Option::KnightAttackValueMiddleGame[mainBoard[endPlace]];
                    g1_white_eg += Option::KnightAttackValueEndGame[mainBoard[endPlace]];
                }
            }
        }
        g1_white_mg += Option::KnightMoveCountValueMiddleGame[moveCount];
        g1_white_eg += Option::KnightMoveCountValueEndGame[moveCount];
    }

    // White Bishops (mobility + attacks)
    for (int sq : board.pieces[3])
    {
        int moveCount = 0;
        for (int direction = 0; direction <= 6; direction += 2)
        {
            const auto& ray = PieceMoves::BishopMoves[sq][direction];
            for (std::size_t counter = 0; counter < ray.size(); ++counter)
            {
                int endPos = ray[counter]->endPlace;
                int endPiece = mainBoard[endPos];
                if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                else if ((Option::PowerTwo[endPos] & board.blackPieces) != 0)
                {
                    g1_white_mg += Option::BishopAttackValueMiddleGame[endPiece];
                    g1_white_eg += Option::BishopAttackValueEndGame[endPiece];
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        g1_white_mg += Option::BishopMoveCountValueMiddleGame[moveCount];
        g1_white_eg += Option::BishopMoveCountValueEndGame[moveCount];
    }

    // White Queens (mobility + attacks)
    for (int sq : board.pieces[5])
    {
        int moveCount = 0;
        for (int direction = 0; direction <= 14; direction += 2)
        {
            const auto& ray = PieceMoves::QueenMoves[sq][direction];
            for (std::size_t counter = 0; counter < ray.size(); ++counter)
            {
                int endPos = ray[counter]->endPlace;
                int endPiece = mainBoard[endPos];
                if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                else if ((Option::PowerTwo[endPos] & board.blackPieces) != 0)
                {
                    g1_white_mg += Option::QueenAttackValueMiddleGame[endPiece];
                    g1_white_eg += Option::QueenAttackValueEndGame[endPiece];
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        g1_white_mg += Option::QueenMoveCountValueMiddleGame[moveCount];
        g1_white_eg += Option::QueenMoveCountValueEndGame[moveCount];
    }

    // Black Pawns (attacked-piece bonuses)
    for (int sq : board.pieces[9])
    {
        if (PieceMoves::BlackPawnMoves[sq][6] != nullptr && sq - 7 == board.unpassentPlace)
        {
            g1_black_mg += Option::PawnAttackValueMiddleGame[1];
            g1_black_eg += Option::PawnAttackValueEndGame[1];
        }
        if (PieceMoves::BlackPawnMoves[sq][7] != nullptr && sq - 9 == board.unpassentPlace)
        {
            g1_black_mg += Option::PawnAttackValueMiddleGame[1];
            g1_black_eg += Option::PawnAttackValueEndGame[1];
        }
        if (PieceMoves::BlackPawnMoves[sq][8] != nullptr && (Option::PowerTwo[sq - 7] & board.whitePieces) != 0)
        {
            g1_black_mg += Option::PawnAttackValueMiddleGame[mainBoard[sq - 7]];
            g1_black_eg += Option::PawnAttackValueEndGame[mainBoard[sq - 7]];
        }
        if (PieceMoves::BlackPawnMoves[sq][13] != nullptr && (Option::PowerTwo[sq - 9] & board.whitePieces) != 0)
        {
            g1_black_mg += Option::PawnAttackValueMiddleGame[mainBoard[sq - 9]];
            g1_black_eg += Option::PawnAttackValueEndGame[mainBoard[sq - 9]];
        }
    }

    // Black Knights (mobility + attacks)
    for (int sq : board.pieces[10])
    {
        int moveCount = 0;
        for (int i = 0; i < 8; ++i)
        {
            int endPlace = sq + knightOffsets[i];
            int dir = knightDirs[i];
            if (PieceMoves::KnightMoves[sq][dir] != nullptr)
            {
                if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                else if ((Option::PowerTwo[endPlace] & board.whitePieces) != 0)
                {
                    g1_black_mg += Option::KnightAttackValueMiddleGame[mainBoard[endPlace]];
                    g1_black_eg += Option::KnightAttackValueEndGame[mainBoard[endPlace]];
                }
            }
        }
        g1_black_mg += Option::KnightMoveCountValueMiddleGame[moveCount];
        g1_black_eg += Option::KnightMoveCountValueEndGame[moveCount];
    }

    // Black Bishops (mobility + attacks)
    for (int sq : board.pieces[11])
    {
        int moveCount = 0;
        for (int direction = 0; direction <= 6; direction += 2)
        {
            const auto& ray = PieceMoves::BishopMoves[sq][direction];
            for (std::size_t counter = 0; counter < ray.size(); ++counter)
            {
                int endPos = ray[counter]->endPlace;
                int endPiece = mainBoard[endPos];
                if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                else if ((Option::PowerTwo[endPos] & board.whitePieces) != 0)
                {
                    g1_black_mg += Option::BishopAttackValueMiddleGame[endPiece];
                    g1_black_eg += Option::BishopAttackValueEndGame[endPiece];
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        g1_black_mg += Option::BishopMoveCountValueMiddleGame[moveCount];
        g1_black_eg += Option::BishopMoveCountValueEndGame[moveCount];
    }

    // Black Queens (mobility + attacks)
    static const int blackQueenDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
    for (int sq : board.pieces[13])
    {
        int moveCount = 0;
        for (int dir = 0; dir < 8; ++dir)
        {
            int direction = blackQueenDirs[dir];
            const auto& ray = PieceMoves::QueenMoves[sq][direction];
            for (std::size_t counter = 0; counter < ray.size(); ++counter)
            {
                int endPos = ray[counter]->endPlace;
                int endPiece = mainBoard[endPos];
                if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                else if ((Option::PowerTwo[endPos] & board.whitePieces) != 0)
                {
                    g1_black_mg += Option::QueenAttackValueMiddleGame[endPiece];
                    g1_black_eg += Option::QueenAttackValueEndGame[endPiece];
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        g1_black_mg += Option::QueenMoveCountValueMiddleGame[moveCount];
        g1_black_eg += Option::QueenMoveCountValueEndGame[moveCount];
    }

    // Tempo
    const int tempo_mg = (!board.sideToMove) ? 24 : -24;
    const int tempo_eg = (!board.sideToMove) ? 11 : -11;

    rec.group1_mg = (g1_white_mg - g1_black_mg) + tempo_mg;
    rec.group1_eg = (g1_white_eg - g1_black_eg) + tempo_eg;

    // Group 2: Heavy Delayed
    // - Rook mobility
    // - Rook attack bonuses
    // - Rook and Queen 7th-rank PST
    int g2_white_mg = 0;
    int g2_white_eg = 0;
    int g2_black_mg = 0;
    int g2_black_eg = 0;

    // White Rooks
    for (int sq : board.pieces[4])
    {
        g2_white_mg += Option::RookInValueWhiteMiddleGame[sq];
        g2_white_eg += Option::RookInValueWhiteEndGame[sq];
        int moveCount = 0;
        for (int direction = 0; direction <= 6; direction += 2)
        {
            const auto& ray = PieceMoves::RookMoves[sq][direction];
            for (std::size_t c = 0; c < ray.size(); ++c)
            {
                int endPos = ray[c]->endPlace;
                if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                else if ((Option::PowerTwo[endPos] & board.blackPieces) != 0)
                {
                    g2_white_mg += Option::RookAttackValueMiddleGame[mainBoard[endPos]];
                    g2_white_eg += Option::RookAttackValueEndGame[mainBoard[endPos]];
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        g2_white_mg += Option::RookMoveCountValueMiddleGame[moveCount];
        g2_white_eg += Option::RookMoveCountValueEndGame[moveCount];
    }

    // White Queens (PST only)
    for (int sq : board.pieces[5])
    {
        g2_white_mg += Option::QueenInValueWhiteMiddleGame[sq];
        g2_white_eg += Option::QueenInValueWhiteEndGame[sq];
    }

    // Black Rooks
    for (int sq : board.pieces[12])
    {
        g2_black_mg += Option::RookInValueBlackMiddleGame[sq];
        g2_black_eg += Option::RookInValueBlackEndGame[sq];
        int moveCount = 0;
        for (int direction = 0; direction <= 6; direction += 2)
        {
            const auto& ray = PieceMoves::RookMoves[sq][direction];
            for (std::size_t c = 0; c < ray.size(); ++c)
            {
                int endPos = ray[c]->endPlace;
                if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                else if ((Option::PowerTwo[endPos] & board.whitePieces) != 0)
                {
                    g2_black_mg += Option::RookAttackValueMiddleGame[mainBoard[endPos]];
                    g2_black_eg += Option::RookAttackValueEndGame[mainBoard[endPos]];
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        g2_black_mg += Option::RookMoveCountValueMiddleGame[moveCount];
        g2_black_eg += Option::RookMoveCountValueEndGame[moveCount];
    }

    // Black Queens (PST only)
    for (int sq : board.pieces[13])
    {
        g2_black_mg += Option::QueenInValueBlackMiddleGame[sq];
        g2_black_eg += Option::QueenInValueBlackEndGame[sq];
    }

    rec.group2_mg = g2_white_mg - g2_black_mg;
    rec.group2_eg = g2_white_eg - g2_black_eg;

    // Group 3: Pawn Late
    // - Passed pawns
    // - Ordinary pawn advancement (goForwardPawn)
    int g3_white_mg = 0;
    int g3_white_eg = 0;
    int g3_black_mg = 0;
    int g3_black_eg = 0;

    for (int sq : board.pieces[1])
    {
        if ((PassedPawnSetup::WhitePassedMask[sq] & board.blackPawns) == 0)
        {
            g3_white_mg += Option::WhitePassedPawnValueMiddleGam[sq];
            g3_white_eg += Option::WhitePassedPawnValueEndGame[sq];
        }
        const int egAdvance = (sq / 8) * 2;
        g3_white_eg += egAdvance;
    }

    for (int sq : board.pieces[9])
    {
        if ((PassedPawnSetup::BlackPassedMask[sq] & board.whitePawns) == 0)
        {
            g3_black_mg += Option::BlackPassedPawnValueMiddleGam[sq];
            g3_black_eg += Option::BlackPassedPawnValueEndGam[sq];
        }
        const int egAdvance = (7 - (sq / 8)) * 2;
        g3_black_eg += egAdvance;
    }

    rec.group3_mg = g3_white_mg - g3_black_mg;
    rec.group3_eg = g3_white_eg - g3_black_eg;

    return rec;
}

// ---------------------------------------------------------------------------
// Constrained Monotonic 3-Anchor WLS Solver
// ---------------------------------------------------------------------------
// Solves: min sum_i weight_i * (w(t_i) - y_i)^2
// subject to 0 <= W_25 <= W_50 <= W_75 <= 1
// with fixed endpoints W_0 = 0, W_100 = 1.
// Uses phase-balanced weighting: weight = (V_EG - V_MG)^2 * phase_balance.

struct SolverResult
{
    // Monotonic constrained anchors
    double w0 = 0.0;
    double w25 = 0.0;
    double w50 = 0.0;
    double w75 = 0.0;
    double w100 = 1.0;

    // Unconstrained anchors
    double unconstrained_w25 = 0.0;
    double unconstrained_w50 = 0.0;
    double unconstrained_w75 = 0.0;
    bool unconstrained_solvable = false;

    // Sum of weighted squared errors
    double sse_teacher = 0.0;
    double sse_linear = 0.0;
    double sse_monotonic = 0.0;
    double sse_unconstrained = 0.0;

    // Weighted RMSE
    double rmse_teacher = 0.0;
    double rmse_linear = 0.0;
    double rmse_monotonic = 0.0;
    double rmse_unconstrained = 0.0;

    double totalWeight = 0.0;
    double weightByPhase[25] = {0};
};

// Evaluate quadratic objective: f(w) = w^T M w - 2 v^T w
double EvalObjective(const double M[3][3], const double v[3], double w1, double w2, double w3)
{
    const double w[3] = {w1, w2, w3};
    double res = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            res += w[i] * M[i][j] * w[j];
        }
        res -= 2.0 * v[i] * w[i];
    }
    return res;
}

bool IsFeasible(double w1, double w2, double w3, double tol = 1e-7)
{
    return (w1 >= -tol) && (w2 >= w1 - tol) && (w3 >= w2 - tol) && (w3 <= 1.0 + tol);
}

void ClampFeasible(double& w1, double& w2, double& w3)
{
    w1 = std::clamp(w1, 0.0, 1.0);
    w2 = std::clamp(w2, w1, 1.0);
    w3 = std::clamp(w3, w2, 1.0);
}

// Solve 2x2 symmetric linear system
bool Solve2x2(double A00, double A01, double A11, double b0, double b1, double& x0, double& x1)
{
    double det = A00 * A11 - A01 * A01;
    if (std::abs(det) < 1e-12) return false;
    x0 = (A11 * b0 - A01 * b1) / det;
    x1 = (A00 * b1 - A01 * b0) / det;
    return true;
}

// Solve 3x3 symmetric linear system
bool Solve3x3(const double A[3][3], const double b[3], double x[3])
{
    double det = A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1]) -
                 A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0]) +
                 A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
    if (std::abs(det) < 1e-12) return false;

    double inv[3][3];
    inv[0][0] = (A[1][1] * A[2][2] - A[1][2] * A[2][1]) / det;
    inv[0][1] = (A[0][2] * A[2][1] - A[0][1] * A[2][2]) / det;
    inv[0][2] = (A[0][1] * A[1][2] - A[0][2] * A[1][1]) / det;

    inv[1][0] = (A[1][2] * A[2][0] - A[1][0] * A[2][2]) / det;
    inv[1][1] = (A[0][0] * A[2][2] - A[0][2] * A[2][0]) / det;
    inv[1][2] = (A[0][2] * A[1][0] - A[0][0] * A[1][2]) / det;

    inv[2][0] = (A[1][0] * A[2][1] - A[1][1] * A[2][0]) / det;
    inv[2][1] = (A[0][1] * A[2][0] - A[0][0] * A[2][1]) / det;
    inv[2][2] = (A[0][0] * A[1][1] - A[0][1] * A[1][0]) / det;

    for (int i = 0; i < 3; ++i)
    {
        x[i] = 0.0;
        for (int j = 0; j < 3; ++j)
        {
            x[i] += inv[i][j] * b[j];
        }
    }
    return true;
}

SolverResult FitMonotonicAnchors(const std::vector<PositionRecord>& records,
                                const int countPerPhase[25],
                                int group)
{
    SolverResult result;
    double M[3][3] = {{0}};
    double v[3] = {0};
    double totalWeight = 0.0;
    double constTerm = 0.0;

    for (const auto& rec : records)
    {
        const double v_mg = (group == 1) ? rec.group1_mg : ((group == 2) ? rec.group2_mg : rec.group3_mg);
        const double v_eg = (group == 1) ? rec.group1_eg : ((group == 2) ? rec.group2_eg : rec.group3_eg);
        const double diff = v_eg - v_mg;

        // Phase balance: inversely proportional to position count in this phase
        const double phaseBalance = 1.0 / std::max(1, countPerPhase[rec.phase]);
        const double weight = diff * diff * phaseBalance;
        if (weight <= 0.0) continue;

        totalWeight += weight;
        result.weightByPhase[rec.phase] += weight;

        const double y = rec.oldBinaryDecision; // 0 (MG) or 1 (EG)
        const double t = (24.0 - rec.phase) / 24.0; // progress 0..1

        // Interpolation row vector r and constant offset c such that w(t) = r * w + c
        double r[3] = {0, 0, 0};
        double c = 0.0;

        if (t < 0.25)
        {
            double lambda = t / 0.25;
            r[0] = lambda;
        }
        else if (t < 0.50)
        {
            double lambda = (t - 0.25) / 0.25;
            r[0] = 1.0 - lambda;
            r[1] = lambda;
        }
        else if (t < 0.75)
        {
            double lambda = (t - 0.50) / 0.25;
            r[1] = 1.0 - lambda;
            r[2] = lambda;
        }
        else
        {
            double lambda = (t - 0.75) / 0.25;
            r[2] = 1.0 - lambda;
            c = lambda; // 1.0 * lambda
        }

        // Error = (r * w + c - y)
        // (r * w + c - y)^2 = w^T (r^T r) w + 2 (c - y) r * w + (c - y)^2
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                M[i][j] += weight * r[i] * r[j];
            }
            v[i] -= weight * (c - y) * r[i];
        }
        constTerm += weight * (c - y) * (c - y);
    }

    result.totalWeight = totalWeight;

    // 1. Unconstrained 3x3 solve
    double x3[3];
    result.unconstrained_solvable = Solve3x3(M, v, x3);
    if (result.unconstrained_solvable)
    {
        result.unconstrained_w25 = x3[0];
        result.unconstrained_w50 = x3[1];
        result.unconstrained_w75 = x3[2];
        result.sse_unconstrained = EvalObjective(M, v, x3[0], x3[1], x3[2]) + constTerm;
        result.rmse_unconstrained = std::sqrt(std::max(0.0, result.sse_unconstrained) / std::max(1.0, totalWeight));
    }

    // 2. Active set exploration for exact constrained minimum
    double bestCost = 1e30;

    auto TestCandidate = [&](double w1, double w2, double w3) {
        if (!IsFeasible(w1, w2, w3)) return;
        ClampFeasible(w1, w2, w3);
        double cost = EvalObjective(M, v, w1, w2, w3);
        if (cost < bestCost)
        {
            bestCost = cost;
            result.w25 = w1;
            result.w50 = w2;
            result.w75 = w3;
        }
    };

    if (result.unconstrained_solvable)
    {
        TestCandidate(x3[0], x3[1], x3[2]);
    }

    // Faces (1 active constraint)
    double xA[2];
    if (Solve2x2(M[1][1], M[1][2], M[2][2], v[1], v[2], xA[0], xA[1]))
    {
        TestCandidate(0.0, xA[0], xA[1]);
    }

    double B00 = M[0][0] + 2.0 * M[0][1] + M[1][1];
    double B01 = M[0][2] + M[1][2];
    double B11 = M[2][2];
    double bB0 = v[0] + v[1];
    double bB1 = v[2];
    double xB[2];
    if (Solve2x2(B00, B01, B11, bB0, bB1, xB[0], xB[1]))
    {
        TestCandidate(xB[0], xB[0], xB[1]);
    }

    double C00 = M[0][0];
    double C01 = M[0][1] + M[0][2];
    double C11 = M[1][1] + 2.0 * M[1][2] + M[2][2];
    double bC0 = v[0];
    double bC1 = v[1] + v[2];
    double xC[2];
    if (Solve2x2(C00, C01, C11, bC0, bC1, xC[0], xC[1]))
    {
        TestCandidate(xC[0], xC[1], xC[1]);
    }

    double xD[2];
    if (Solve2x2(M[0][0], M[0][1], M[1][1], v[0] - M[0][2], v[1] - M[1][2], xD[0], xD[1]))
    {
        TestCandidate(xD[0], xD[1], 1.0);
    }

    // Edges (2 active constraints)
    if (M[2][2] > 1e-12)
    {
        double u = v[2] / M[2][2];
        TestCandidate(0.0, 0.0, u);
    }
    double E2_M = M[1][1] + 2.0 * M[1][2] + M[2][2];
    if (E2_M > 1e-12)
    {
        double u = (v[1] + v[2]) / E2_M;
        TestCandidate(0.0, u, u);
    }
    if (M[1][1] > 1e-12)
    {
        double u = (v[1] - M[1][2]) / M[1][1];
        TestCandidate(0.0, u, 1.0);
    }
    double E4_M = M[0][0] + 2.0 * M[0][1] + M[1][1];
    if (E4_M > 1e-12)
    {
        double u = (v[0] + v[1] - (M[0][2] + M[1][2])) / E4_M;
        TestCandidate(u, u, 1.0);
    }
    if (M[0][0] > 1e-12)
    {
        double u = (v[0] - (M[0][1] + M[0][2])) / M[0][0];
        TestCandidate(u, 1.0, 1.0);
    }
    double E6_M = M[0][0] + M[1][1] + M[2][2] + 2.0 * (M[0][1] + M[0][2] + M[1][2]);
    if (E6_M > 1e-12)
    {
        double u = (v[0] + v[1] + v[2]) / E6_M;
        TestCandidate(u, u, u);
    }

    // Vertices (3 active constraints)
    TestCandidate(0.0, 0.0, 0.0);
    TestCandidate(0.0, 0.0, 1.0);
    TestCandidate(0.0, 1.0, 1.0);
    TestCandidate(1.0, 1.0, 1.0);
    TestCandidate(0.25, 0.50, 0.75); // linear reference

    result.w0 = 0.0;
    result.w100 = 1.0;

    // Error comparisons:
    // 1. Binary teacher directly reproduced: error is identically 0
    result.sse_teacher = 0.0;
    result.rmse_teacher = 0.0;

    // 2. Simple linear taper: w(0.25)=0.25, w(0.5)=0.5, w(0.75)=0.75
    result.sse_linear = EvalObjective(M, v, 0.25, 0.50, 0.75) + constTerm;
    result.rmse_linear = std::sqrt(std::max(0.0, result.sse_linear) / std::max(1.0, totalWeight));

    // 3. Fitted monotonic curve
    result.sse_monotonic = bestCost + constTerm;
    result.rmse_monotonic = std::sqrt(std::max(0.0, result.sse_monotonic) / std::max(1.0, totalWeight));

    return result;
}

} // anonymous namespace

int main(int argc, char* argv[])
{
    InitializeEngine();

    std::string fenPath;
    std::string dumpCsvPath;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--fens" && i + 1 < argc)
        {
            fenPath = argv[++i];
        }
        else if (arg == "--dump" && i + 1 < argc)
        {
            dumpCsvPath = argv[++i];
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage: howl_phase_curve_tool --fens <file_or_dash> [--dump <output.csv>]\n\n"
                      << "Options:\n"
                      << "  --fens <path>    Path to file containing FEN strings (or '-' for stdin)\n"
                      << "  --dump <path>    Dump recorded position metrics to CSV\n";
            return 0;
        }
    }

    if (fenPath.empty())
    {
        std::cerr << "Error: --fens <path> is required. Run with --help for usage.\n";
        return 1;
    }

    std::istream* inStream = &std::cin;
    std::ifstream fileStream;
    if (fenPath != "-")
    {
        fileStream.open(fenPath);
        if (!fileStream)
        {
            std::cerr << "Error: unable to open FEN file: " << fenPath << "\n";
            return 1;
        }
        inStream = &fileStream;
    }

    std::vector<PositionRecord> records;
    records.reserve(30000);

    int countPerPhase[25] = {0};
    int oldBinaryCount[2] = {0};

    std::string line;
    int lineNum = 0;
    while (std::getline(*inStream, line))
    {
        lineNum++;
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
        {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#') continue;

        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(line));
        if (!board)
        {
            continue;
        }

        PositionRecord rec = EvaluateRecord(*board);
        records.push_back(rec);
        countPerPhase[rec.phase]++;
        oldBinaryCount[rec.oldBinaryDecision]++;
    }

    std::cout << "============================================================\n";
    std::cout << "Howl Offline Phase-Curve Fitter (Phase-Balanced WLS)\n";
    std::cout << "============================================================\n";
    std::cout << "Parsed positions: " << records.size() << " (from " << lineNum << " lines)\n";
    std::cout << "Teacher binary decisions: MG=" << oldBinaryCount[0]
              << " (" << std::fixed << std::setprecision(1) << (100.0 * oldBinaryCount[0] / std::max(1, (int)records.size())) << "%), "
              << "EG=" << oldBinaryCount[1]
              << " (" << (100.0 * oldBinaryCount[1] / std::max(1, (int)records.size())) << "%)\n\n";

    if (!dumpCsvPath.empty())
    {
        std::ofstream csv(dumpCsvPath);
        if (csv)
        {
            csv << "phase,old_binary,group1_mg,group1_eg,group2_mg,group2_eg,group3_mg,group3_eg\n";
            for (const auto& r : records)
            {
                csv << r.phase << ","
                    << r.oldBinaryDecision << ","
                    << r.group1_mg << ","
                    << r.group1_eg << ","
                    << r.group2_mg << ","
                    << r.group2_eg << ","
                    << r.group3_mg << ","
                    << r.group3_eg << "\n";
            }
            std::cout << "Dumped " << records.size() << " records to: " << dumpCsvPath << "\n\n";
        }
    }

    // Fit Group 1 (Steady / Fast: Knight/Bishop/Queen Mobility & Attacks, Pawn Attacks, Tempo)
    SolverResult fitG1 = FitMonotonicAnchors(records, countPerPhase, 1);
    // Fit Group 2 (Heavy Delayed: Rook Mobility, Rook Attacks, 7th Rank)
    SolverResult fitG2 = FitMonotonicAnchors(records, countPerPhase, 2);
    // Fit Group 3 (Pawn Late: Passed Pawns, Pawn Advancement)
    SolverResult fitG3 = FitMonotonicAnchors(records, countPerPhase, 3);

    auto PrintGroupReport = [](const std::string& title, const SolverResult& fit) {
        std::cout << "============================================================\n";
        std::cout << title << "\n";
        std::cout << "============================================================\n";
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "1. Fitted Monotonic Constrained Anchors:\n";
        std::cout << "  W0   (0%   endgame, phase=24): " << fit.w0   << " (fixed MG endpoint)\n";
        std::cout << "  W25  (25%  endgame, phase=18): " << fit.w25  << "\n";
        std::cout << "  W50  (50%  endgame, phase=12): " << fit.w50  << "\n";
        std::cout << "  W75  (75%  endgame, phase=6):  " << fit.w75  << "\n";
        std::cout << "  W100 (100% endgame, phase=0):  " << fit.w100 << " (fixed EG endpoint)\n\n";

        std::cout << "2. Unconstrained Anchors:\n";
        if (fit.unconstrained_solvable)
        {
            std::cout << "  W25  (unconstrained): " << fit.unconstrained_w25 << "\n";
            std::cout << "  W50  (unconstrained): " << fit.unconstrained_w50 << "\n";
            std::cout << "  W75  (unconstrained): " << fit.unconstrained_w75 << "\n";
        }
        else
        {
            std::cout << "  (system was singular / non-invertible)\n";
        }

        std::cout << "\n3. Simple Linear Taper Weighted Error:\n";
        std::cout << "  Weighted SSE  = " << std::setw(12) << fit.sse_linear << "\n";
        std::cout << "  Weighted RMSE = " << fit.rmse_linear << "\n";

        std::cout << "\n4. Fitted Monotonic Weighted Error:\n";
        std::cout << "  Weighted SSE  = " << std::setw(12) << fit.sse_monotonic << "\n";
        std::cout << "  Weighted RMSE = " << fit.rmse_monotonic << "\n";
        if (fit.unconstrained_solvable)
        {
            std::cout << "  (Unconstrained SSE = " << fit.sse_unconstrained << ", RMSE = " << fit.rmse_unconstrained << ")\n";
        }

        std::cout << "\n5. Percentage Error Reduction (Linear -> Fitted Monotonic):\n";
        double sseRed = (fit.sse_linear > 0.0) ? (100.0 * (fit.sse_linear - fit.sse_monotonic) / fit.sse_linear) : 0.0;
        double rmseRed = (fit.rmse_linear > 0.0) ? (100.0 * (fit.rmse_linear - fit.rmse_monotonic) / fit.rmse_linear) : 0.0;
        std::cout << "  SSE Reduction:  " << std::setw(6) << std::setprecision(2) << sseRed << "%\n";
        std::cout << "  RMSE Reduction: " << std::setw(6) << std::setprecision(2) << rmseRed << "%\n";

        std::cout << "\n7. Monotonic Constraint Impact on Unconstrained Result:\n";
        if (fit.unconstrained_solvable)
        {
            bool exactlySame = (std::abs(fit.w25 - fit.unconstrained_w25) < 1e-4) &&
                               (std::abs(fit.w50 - fit.unconstrained_w50) < 1e-4) &&
                               (std::abs(fit.w75 - fit.unconstrained_w75) < 1e-4);
            if (exactlySame)
            {
                std::cout << "  The unconstrained optimum naturally satisfies monotonicity (0 <= W25 <= W50 <= W75 <= 1).\n"
                          << "  Constraints do NOT alter the unconstrained solution.\n";
            }
            else
            {
                std::cout << "  The unconstrained optimum violates monotonicity; constraints active at boundaries.\n"
                          << "  Constraint shift: dW25 = " << (fit.w25 - fit.unconstrained_w25)
                          << ", dW50 = " << (fit.w50 - fit.unconstrained_w50)
                          << ", dW75 = " << (fit.w75 - fit.unconstrained_w75) << "\n"
                          << "  SSE penalty due to constraints: " << (fit.sse_monotonic - fit.sse_unconstrained) << "\n";
            }
        }
        std::cout << "\n";
    };

    PrintGroupReport("Group 1: Steady / Fast (N/B/Q Mobility, P/N/B/Q Attacks, Tempo)", fitG1);
    PrintGroupReport("Group 2: Heavy-Piece Delayed (Rook Mobility, Rook Attacks, 7th Rank)", fitG2);
    PrintGroupReport("Group 3: Pawn Late (Passed Pawns, Pawn Advancement)", fitG3);

    std::cout << "============================================================\n";
    std::cout << "6. Effective Sample Weight by Phase After Balancing\n";
    std::cout << "============================================================\n";
    std::cout << "Phase | Positions | G1 Effective Weight (% Total) | G2 Effective Weight (% Total) | G3 Effective Weight (% Total)\n";
    std::cout << "------+-----------+-------------------------------+-------------------------------+-------------------------------\n";
    for (int p = 0; p <= 24; ++p)
    {
        double g1_pct = (fitG1.totalWeight > 0.0) ? (100.0 * fitG1.weightByPhase[p] / fitG1.totalWeight) : 0.0;
        double g2_pct = (fitG2.totalWeight > 0.0) ? (100.0 * fitG2.weightByPhase[p] / fitG2.totalWeight) : 0.0;
        double g3_pct = (fitG3.totalWeight > 0.0) ? (100.0 * fitG3.weightByPhase[p] / fitG3.totalWeight) : 0.0;
        std::cout << "  " << std::setw(2) << p << "  |   "
                  << std::setw(5) << countPerPhase[p] << "   |   "
                  << std::setw(11) << fitG1.weightByPhase[p] << " (" << std::setw(4) << std::setprecision(1) << g1_pct << "%)   |   "
                  << std::setw(11) << fitG2.weightByPhase[p] << " (" << std::setw(4) << std::setprecision(1) << g2_pct << "%)   |   "
                  << std::setw(11) << fitG3.weightByPhase[p] << " (" << std::setw(4) << std::setprecision(1) << g3_pct << "%)\n";
    }
    std::cout << "============================================================\n";

    return 0;
}
