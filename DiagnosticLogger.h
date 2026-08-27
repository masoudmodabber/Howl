#ifndef DIAGNOSTICLOGGER_H
#define DIAGNOSTICLOGGER_H

#include <string>
#include <fstream>
#include <chrono>
#include <mutex>
#include <atomic>
#include <sstream>
#include "Board.h"

class DiagnosticLogger
{
public:
    static inline std::atomic<uint64_t> sequenceNumber{0};
    static inline std::atomic<uint64_t> currentSearchId{0};
    static inline std::mutex logMutex;
    static inline std::ofstream logFile;
    static inline bool initialized{false};

    static void EnsureInit()
    {
        std::lock_guard<std::mutex> lock(logMutex);
        if (!initialized)
        {
            logFile.open("howl_uci_diagnostic.log", std::ios::out | std::ios::app);
            initialized = true;
        }
    }

    static void Log(const std::string& eventType, const std::string& details, uint64_t searchId = 0)
    {
        EnsureInit();
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        uint64_t seq = ++sequenceNumber;

        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open())
        {
            logFile << "[" << ms << "ms][Seq:" << seq << "][SearchId:" << searchId << "] "
                    << eventType << " | " << details << std::endl;
            logFile.flush();
        }
    }

    static std::string BoardToFen(const Board* b)
    {
        if (b == nullptr)
            return "<null-board>";

        std::ostringstream oss;
        for (int r = 7; r >= 0; --r)
        {
            int emptyCount = 0;
            for (int c = 0; c < 8; ++c)
            {
                int sq = r * 8 + c;
                int p = b->mainBoard[sq];
                if (p == 0)
                {
                    emptyCount++;
                }
                else
                {
                    if (emptyCount > 0)
                    {
                        oss << emptyCount;
                        emptyCount = 0;
                    }
                    const char pieceChars[] = {' ', 'P', 'N', 'B', 'R', 'Q', 'K', ' ', ' ', 'p', 'n', 'b', 'r', 'q', 'k'};
                    if (p >= 1 && p <= 14)
                        oss << pieceChars[p];
                    else
                        oss << '?';
                }
            }
            if (emptyCount > 0)
                oss << emptyCount;
            if (r > 0)
                oss << '/';
        }
        oss << (b->sideToMove ? " b " : " w ");
        std::string castling = "";
        if (b->whiteSmallCastle) castling += "K";
        if (b->whiteBigCastle) castling += "Q";
        if (b->blackSmallCastle) castling += "k";
        if (b->blackBigCastle) castling += "q";
        if (castling.empty()) castling = "-";
        oss << castling << " ";
        if (b->unpassentPlace >= 0 && b->unpassentPlace < 64)
        {
            char f = 'a' + (b->unpassentPlace % 8);
            char rk = '1' + (b->unpassentPlace / 8);
            oss << f << rk;
        }
        else
        {
            oss << "-";
        }
        oss << " " << b->fiftyMoveRule << " " << b->moveNumber;
        return oss.str();
    }
};

#endif // DIAGNOSTICLOGGER_H
