#ifndef TABLEBASE_H
#define TABLEBASE_H

#include <optional>
#include <string>

class Board;

class Tablebase
{
public:
    enum class Wdl
    {
        Loss = -1,
        Draw = 0,
        Win = 1
    };

    static bool Initialize(const std::string &path);
    static bool IsAvailable();
    static int MaxPieces();
    static int PieceCount(const Board &position);
    static bool IsPositionStateSupported(const Board &position);
    static bool IsProbeEligible(const Board &position, int probeLimit);
    static std::optional<Wdl> ProbeWdl(const Board &position, int probeLimit);

    static constexpr int WinScore = 150000;
    static constexpr int LossScore = -WinScore;
    static int Score(Wdl result);
};

#endif
