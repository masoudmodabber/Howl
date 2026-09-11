#include "Tablebase.h"

#include "Board.h"
#include "third_party/fathom/src/tbprobe.h"

#include <cstdint>

namespace
{
    bool initialized = false;
    bool available = false;

    uint64_t PiecesOfType(const Board &board, int whitePiece, int blackPiece)
    {
        uint64_t result = 0;
        for (int i = 0; i < board.pieces[whitePiece].count; ++i)
            result |= UINT64_C(1) << board.pieces[whitePiece].data[i];
        for (int i = 0; i < board.pieces[blackPiece].count; ++i)
            result |= UINT64_C(1) << board.pieces[blackPiece].data[i];
        return result;
    }

    uint64_t SidePieces(const Board &board, int offset)
    {
        uint64_t result = 0;
        for (int piece = 1; piece <= 6; ++piece)
            for (int i = 0; i < board.pieces[offset + piece].count; ++i)
                result |= UINT64_C(1) << board.pieces[offset + piece].data[i];
        return result;
    }
}

bool Tablebase::Initialize(const std::string &path)
{
    if (initialized)
        tb_free();
    initialized = false;
    available = false;

    if (path.empty())
        return true;

    initialized = tb_init(path.c_str());
    available = initialized && TB_LARGEST > 0;
    return available;
}

bool Tablebase::IsAvailable()
{
    return available;
}

int Tablebase::MaxPieces()
{
    return available ? static_cast<int>(TB_LARGEST) : 0;
}

int Tablebase::PieceCount(const Board &position)
{
    int count = 0;
    for (int offset : {0, 8})
        for (int piece = 1; piece <= 6; ++piece)
            count += static_cast<int>(position.pieces[offset + piece].size());
    return count;
}

bool Tablebase::IsPositionStateSupported(const Board &position)
{
    if (position.whiteSmallCastle || position.whiteBigCastle ||
        position.blackSmallCastle || position.blackBigCastle)
        return false;
    // Fathom's WDL API deliberately rejects a non-zero rule-50 clock. Do not
    // discard that state and accidentally claim an exact result.
    if (position.fiftyMoveRule != 0)
        return false;
    return true;
}

bool Tablebase::IsProbeEligible(const Board &position, int probeLimit)
{
    if (!available || probeLimit <= 0 || !IsPositionStateSupported(position))
        return false;
    const int pieces = PieceCount(position);
    return pieces <= probeLimit && pieces <= MaxPieces();
}

std::optional<Tablebase::Wdl> Tablebase::ProbeWdl(const Board &position,
                                                   int probeLimit)
{
    if (!IsProbeEligible(position, probeLimit))
        return std::nullopt;

    const uint64_t white = SidePieces(position, 0);
    const uint64_t black = SidePieces(position, 8);
    const unsigned result = tb_probe_wdl(
        white, black,
        PiecesOfType(position, 6, 14),
        PiecesOfType(position, 5, 13),
        PiecesOfType(position, 4, 12),
        PiecesOfType(position, 3, 11),
        PiecesOfType(position, 2, 10),
        PiecesOfType(position, 1, 9),
        static_cast<unsigned>(position.fiftyMoveRule), 0,
        static_cast<unsigned>(position.unpassentPlace),
        !position.sideToMove);

    if (result == TB_WIN)
        return Wdl::Win;
    if (result == TB_DRAW)
        return Wdl::Draw;
    if (result == TB_LOSS)
        return Wdl::Loss;
    return std::nullopt;
}

int Tablebase::Score(Wdl result)
{
    return static_cast<int>(result) * WinScore;
}
