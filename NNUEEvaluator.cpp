#include "NNUEEvaluator.h"
#include "Move.h"
#include "Option.h"

#include <stdexcept>
#include <array>
#include <cstdint>
#include <fstream>
#include <vector>
#include <chrono>
#include <iostream>
#include <cstdlib>


namespace
{
constexpr int DenseHiddenSize = 32;
std::vector<float> firstLayer;
std::array<float, NNUEEvaluator::HiddenSize> firstBias{};
std::array<float, DenseHiddenSize * NNUEEvaluator::HiddenSize * 2> denseWeights{};
std::array<float, DenseHiddenSize> denseBias{};
std::array<float, DenseHiddenSize * DenseHiddenSize> dense2Weights{};
std::array<float, DenseHiddenSize> dense2Bias{};
std::array<float, DenseHiddenSize> outputWeights{};
float outputBias = 0.0f;
bool initialized = false;
NNUEEvaluator::Profile profile;

float DummyWeight(unsigned seed)
{
    return static_cast<float>(static_cast<int>(seed % 2001u) - 1000) / 100000.0f;
}

void EnsureInitialized()
{
    if (initialized)
        return;
    firstLayer.resize(static_cast<std::size_t>(NNUEEvaluator::FeatureCount) * NNUEEvaluator::HiddenSize);
    for (std::size_t i = 0; i < firstLayer.size(); ++i)
        firstLayer[i] = DummyWeight(static_cast<unsigned>(i * 1315423911u));
    for (int i = 0; i < NNUEEvaluator::HiddenSize; ++i)
        firstBias[i] = DummyWeight(static_cast<unsigned>(i * 2654435761u));
    for (std::size_t i = 0; i < denseWeights.size(); ++i)
        denseWeights[i] = DummyWeight(static_cast<unsigned>(i * 2246822519u));
    for (int i = 0; i < DenseHiddenSize; ++i)
    {
        denseBias[i] = DummyWeight(static_cast<unsigned>(i * 3266489917u));
        outputWeights[i] = DummyWeight(static_cast<unsigned>(i * 668265263u));
    }
    for (std::size_t i = 0; i < dense2Weights.size(); ++i) dense2Weights[i] = DummyWeight(static_cast<unsigned>(i * 747796405u));
    for (int i = 0; i < DenseHiddenSize; ++i) dense2Bias[i] = DummyWeight(static_cast<unsigned>(i * 2891336453u));
    outputBias = 0.0f;
    initialized = true;
}

void CheckStream(bool good, const char* message)
{
    if (!good) throw std::runtime_error(message);
}
}

int NNUEEvaluator::FeatureIndex(int anchorKingSquare, int coloredPieceClass, int pieceSquare)
{
    if (anchorKingSquare < 0 || anchorKingSquare >= KingAnchors ||
        coloredPieceClass < 0 || coloredPieceClass >= ColoredPieceClasses ||
        pieceSquare < 0 || pieceSquare >= Squares)
        throw std::out_of_range("NNUE feature coordinate");
    return ((anchorKingSquare * ColoredPieceClasses) + coloredPieceClass) * Squares + pieceSquare;
}

int NNUEEvaluator::ColoredPieceClass(int pieceId)
{
    switch (pieceId)
    {
    case 1: return 0;
    case 2: return 1;
    case 3: return 2;
    case 4: return 3;
    case 5: return 4;
    case 9: return 5;
    case 10: return 6;
    case 11: return 7;
    case 12: return 8;
    case 13: return 9;
    default: return -1;
    }
}

int NNUEEvaluator::FeatureIndexForPerspective(int kingSquare, bool whitePerspective, int pieceId, int pieceSquare)
{
    bool own = whitePerspective ? pieceId < 9 : pieceId > 8;
    int cls = ColoredPieceClass(pieceId);
    if (cls < 0) return -1;
    cls = (cls % 5) + (own ? 0 : 5);
    int anchor = whitePerspective ? kingSquare : (kingSquare ^ 63);
    int square = whitePerspective ? pieceSquare : (pieceSquare ^ 63);
    return FeatureIndex(anchor, cls, square);
}

float NNUEEvaluator::Weight(int feature, int unit)
{
    EnsureInitialized();
    return firstLayer[static_cast<std::size_t>(feature) * HiddenSize + unit];
}

void NNUEEvaluator::Rebuild(Board& board)
{
    auto t=std::chrono::steady_clock::now(); ++profile.rebuildCalls;
    EnsureInitialized();
    const int whiteKingSquare = board.pieces[6].front();
    const int blackKingSquare = board.pieces[14].front();
    board.nnueState.whiteAccumulator = firstBias;
    board.nnueState.blackAccumulator = firstBias;

    for (int square = 0; square < Squares; ++square)
    {
        const int pieceId = board.mainBoard[square];
        const int pieceClass = ColoredPieceClass(pieceId);
        if (pieceClass < 0)
            continue;
        const int whiteFeature = FeatureIndexForPerspective(whiteKingSquare, true, pieceId, square);
        const int blackFeature = FeatureIndexForPerspective(blackKingSquare, false, pieceId, square);
        for (int unit = 0; unit < HiddenSize; ++unit)
        {
            board.nnueState.whiteAccumulator[unit] += Weight(whiteFeature, unit);
            board.nnueState.blackAccumulator[unit] += Weight(blackFeature, unit);
        }
    }
    board.nnueState.whiteKingSquare = whiteKingSquare;
    board.nnueState.blackKingSquare = blackKingSquare;
    board.nnueState.initialized = true;
    profile.rebuildMs += std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();
}

void NNUEEvaluator::UpdateAfterMove(Board& board, const Move& move, const NNUEState& previous)
{
    auto t=std::chrono::steady_clock::now(); ++profile.updateCalls;
    EnsureInitialized();
    board.nnueState = previous;
    if (!previous.initialized) {
        profile.updateMs += std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();
        return;
    }
    if (move.promotionPiece < 0)
    {
        board.nnueState.whiteKingSquare = board.pieces[6].front();
        board.nnueState.blackKingSquare = board.pieces[14].front();
        board.nnueState.initialized = true;
        return;
    }
    const bool castle = (move.CastleFlag & (Option::PowerTwo[0] | Option::PowerTwo[1] |
                                            Option::PowerTwo[2] | Option::PowerTwo[3])) != 0;
    auto apply = [&](int anchor, bool whitePerspective, int piece, int square, float sign, std::array<float, HiddenSize>& acc) {
        int cls = ColoredPieceClass(piece);
        if (cls < 0) return;
        int f = FeatureIndexForPerspective(anchor, whitePerspective, piece, square);
        for (int u = 0; u < HiddenSize; ++u) acc[u] += sign * Weight(f, u);
    };
    auto rebuildAnchor = [&](int anchor, bool whitePerspective, std::array<float, HiddenSize>& acc) {
        acc = firstBias;
        for (int sq = 0; sq < Squares; ++sq) apply(anchor, whitePerspective, board.mainBoard[sq], sq, 1.0f, acc);
    };
    const int oldWhite = previous.whiteKingSquare, oldBlack = previous.blackKingSquare;
    const int movedPieceAfter = board.mainBoard[move.endPlace];
    const bool moverWhite = movedPieceAfter > 0 && movedPieceAfter < 9;
    const int moving = (move.beginPlace >= 0 && move.beginPlace < 64) ?
        (move.CastleFlag ? (moverWhite ? 6 : 14) : board.mainBoard[move.endPlace]) : 0;
    const bool movingKing = moving == 6 || moving == 14;
    if (!castle && move.promotionPiece > 0)
    {
        rebuildAnchor(oldWhite, true, board.nnueState.whiteAccumulator);
        rebuildAnchor(oldBlack, false, board.nnueState.blackAccumulator);
    }
    else if (!castle && !movingKing) {
        int piece = board.mainBoard[move.endPlace];
        if (move.promotionPiece > 0) piece = move.promotionPiece;
        int oldPiece = (move.promotionPiece > 0) ? (piece > 8 ? 9 : 1) : piece;
        apply(oldWhite, true, oldPiece, move.beginPlace, -1.0f, board.nnueState.whiteAccumulator);
        apply(oldBlack, false, oldPiece, move.beginPlace, -1.0f, board.nnueState.blackAccumulator);
        apply(oldWhite, true, piece, move.endPlace, 1.0f, board.nnueState.whiteAccumulator);
        apply(oldBlack, false, piece, move.endPlace, 1.0f, board.nnueState.blackAccumulator);
        if (move.endPiece > 0 && (move.PublicFlag & Option::PowerTwo[6]) == 0) {
            apply(oldWhite, true, move.endPiece, move.endPlace, -1.0f, board.nnueState.whiteAccumulator);
            apply(oldBlack, false, move.endPiece, move.endPlace, -1.0f, board.nnueState.blackAccumulator);
        }
        if ((move.PublicFlag & Option::PowerTwo[6]) != 0) {
            int capSq = moverWhite ? move.endPlace - 8 : move.endPlace + 8;
            int capPiece = moverWhite ? 9 : 1;
            apply(oldWhite, true, capPiece, capSq, -1.0f, board.nnueState.whiteAccumulator);
            apply(oldBlack, false, capPiece, capSq, -1.0f, board.nnueState.blackAccumulator);
        }
    } else if (movingKing || castle) {
        const bool white = moverWhite;
        if (white) { rebuildAnchor(board.pieces[6].front(), true, board.nnueState.whiteAccumulator); board.nnueState.whiteKingSquare = board.pieces[6].front(); }
        else { rebuildAnchor(board.pieces[14].front(), false, board.nnueState.blackAccumulator); board.nnueState.blackKingSquare = board.pieces[14].front(); }
        if (castle) {
            int oldRook = white ? 4 : 12;
            int oldSq = move.beginPlace + ((move.CastleFlag & (Option::PowerTwo[3] | Option::PowerTwo[1])) ? 3 : -4);
            int newSq = move.beginPlace + ((move.CastleFlag & (Option::PowerTwo[3] | Option::PowerTwo[1])) ? 1 : -1);
            auto& opposite = white ? board.nnueState.blackAccumulator : board.nnueState.whiteAccumulator;
            int anchor = white ? oldBlack : oldWhite;
            apply(anchor, !white, oldRook, oldSq, -1.0f, opposite); apply(anchor, !white, oldRook, newSq, 1.0f, opposite);
        }
    }
    board.nnueState.whiteKingSquare = board.pieces[6].front();
    board.nnueState.blackKingSquare = board.pieces[14].front();
    board.nnueState.initialized = true;
    profile.updateMs += std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();
}

float NNUEEvaluator::Evaluate(const Board& board)
{
    auto t=std::chrono::steady_clock::now(); ++profile.forwardCalls;
    EnsureInitialized();
    const auto& stm = board.sideToMove ? board.nnueState.blackAccumulator : board.nnueState.whiteAccumulator;
    const auto& opp = board.sideToMove ? board.nnueState.whiteAccumulator : board.nnueState.blackAccumulator;
    std::array<float, DenseHiddenSize> hidden{};
    std::array<float, DenseHiddenSize> hidden2{};
    for (int i = 0; i < DenseHiddenSize; ++i)
    {
        float value = denseBias[i];
        for (int j = 0; j < HiddenSize; ++j)
            value += denseWeights[i * HiddenSize * 2 + j] * stm[j];
        for (int j = 0; j < HiddenSize; ++j)
            value += denseWeights[i * HiddenSize * 2 + HiddenSize + j] * opp[j];
        hidden[i] = value > 0.0f ? value : 0.0f;
    }
    for (int i = 0; i < DenseHiddenSize; ++i) { float value = dense2Bias[i]; for (int j=0;j<DenseHiddenSize;++j) value += dense2Weights[i*DenseHiddenSize+j]*hidden[j]; hidden2[i]=value>0?value:0; }
    float result = outputBias;
    for (int i = 0; i < DenseHiddenSize; ++i) result += outputWeights[i] * hidden2[i];
    profile.forwardMs += std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count(); return result;
}


void NNUEEvaluator::ResetProfile() { profile = {}; }
NNUEEvaluator::Profile NNUEEvaluator::GetProfile() { return profile; }
void NNUEEvaluator::SaveSnapshot(Board& board)
{
    NNUEHistory& history = *board.nnueHistory;
    if (history.snapshotCount >= NNUEHistory::MaxSnapshots)
    {
        std::cerr << "NNUE SNAPSHOT OVERFLOW: "
                  << history.snapshotCount << std::endl;
        std::abort();
    }

    history.snapshots[history.snapshotCount++] = board.nnueState;
    ++profile.snapshotSaveCalls;
}

NNUEState NNUEEvaluator::RestoreSnapshot(Board& board)
{
    ++profile.snapshotRestoreCalls;

    NNUEHistory& history = *board.nnueHistory;

    if (history.snapshotCount <= 0)
    {
        std::cerr << "NNUE SNAPSHOT UNDERFLOW" << std::endl;
        std::abort();
    }

    return history.snapshots[history.snapshotCount - 1];
}

void NNUEEvaluator::SaveWeights(const std::string& path)
{
    EnsureInitialized();
    std::ofstream out(path, std::ios::binary);
    CheckStream(out.good(), "cannot open NNUE weights for writing");
    const std::uint32_t magic = 0x3145554Eu;
    const std::uint32_t version = 1;
    out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(firstLayer.data()), static_cast<std::streamsize>(firstLayer.size() * sizeof(float)));
    out.write(reinterpret_cast<const char*>(firstBias.data()), sizeof(firstBias));
    out.write(reinterpret_cast<const char*>(denseWeights.data()), sizeof(denseWeights));
    out.write(reinterpret_cast<const char*>(denseBias.data()), sizeof(denseBias));
    out.write(reinterpret_cast<const char*>(dense2Weights.data()), sizeof(dense2Weights));
    out.write(reinterpret_cast<const char*>(dense2Bias.data()), sizeof(dense2Bias));
    out.write(reinterpret_cast<const char*>(outputWeights.data()), sizeof(outputWeights));
    out.write(reinterpret_cast<const char*>(&outputBias), sizeof(outputBias));
    CheckStream(out.good(), "cannot write NNUE weights");
}


void NNUEEvaluator::LoadWeights(const std::string& path)
{
    EnsureInitialized();
    std::ifstream in(path, std::ios::binary);
    CheckStream(in.good(), "cannot open NNUE weights for reading");
    std::uint32_t magic = 0;
    std::uint32_t version = 0;
    in.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    CheckStream(magic == 0x3145554Eu && version == 1, "invalid NNUE weights header");
    in.read(reinterpret_cast<char*>(firstLayer.data()), static_cast<std::streamsize>(firstLayer.size() * sizeof(float)));
    in.read(reinterpret_cast<char*>(firstBias.data()), sizeof(firstBias));
    in.read(reinterpret_cast<char*>(denseWeights.data()), sizeof(denseWeights));
    in.read(reinterpret_cast<char*>(denseBias.data()), sizeof(denseBias));
    in.read(reinterpret_cast<char*>(dense2Weights.data()), sizeof(dense2Weights));
    in.read(reinterpret_cast<char*>(dense2Bias.data()), sizeof(dense2Bias));
    in.read(reinterpret_cast<char*>(outputWeights.data()), sizeof(outputWeights));
    in.read(reinterpret_cast<char*>(&outputBias), sizeof(outputBias));
    CheckStream(in.good(), "cannot read NNUE weights");
}
