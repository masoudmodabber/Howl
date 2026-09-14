#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <sstream>
#include <chrono>
#include <fstream>
#include <vector>
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

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: howl_eval_breakdown \"<FEN>\" or howl_eval_breakdown --throughput [file.json]\n";
        return 1;
    }

    if (std::string(argv[1]) == "--throughput")
    {
        InitializeEngine();
        std::string jsonPath = "tools/static_eval_calibration.json";
        if (argc >= 3) jsonPath = argv[2];
        std::ifstream f(jsonPath);
        if (!f.is_open())
        {
            std::cerr << "Failed to open " << jsonPath << '\n';
            return 1;
        }
        std::string line;
        std::vector<std::string> fens;
        while (std::getline(f, line))
        {
            auto pos = line.find("\"resulting_fen\": \"");
            if (pos != std::string::npos)
            {
                auto start = pos + 18;
                auto end = line.find("\"", start);
                if (end != std::string::npos)
                {
                    fens.push_back(line.substr(start, end - start));
                }
            }
        }
        std::vector<std::unique_ptr<Board>> boards;
        for (const auto& fenStr : fens)
        {
            std::unique_ptr<Board> b(BoardMaker::MakeInitialBoard(fenStr));
            if (b) boards.push_back(std::move(b));
        }
        std::cout << "Benchmarking uncached eval on " << boards.size() << " positions...\n";

        // Warmup
        for (auto& b : boards)
        {
            volatile int s = EvaluationLogic::Evaluate(*b);
            (void)s;
        }

        constexpr int iterations = 1000;
        const auto startTime = std::chrono::high_resolution_clock::now();
        long long totalEvals = 0;
        uint64_t keyCounter = 1;
        for (int it = 0; it < iterations; ++it)
        {
            for (auto& b : boards)
            {
                // Unique key forces cache miss on every eval (pure uncached evaluation throughput)
                b->ZobristHashCode = static_cast<long long>(keyCounter++);
                volatile int s = EvaluationLogic::Evaluate(*b);
                (void)s;
                totalEvals++;
            }
        }
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        double usPerEval = static_cast<double>(elapsedUs) / totalEvals;
        double evalsPerSec = (totalEvals * 1e6) / elapsedUs;

        std::cout << "Total evaluations: " << totalEvals << "\n";
        std::cout << "Elapsed time: " << elapsedUs / 1000.0 << " ms\n";
        std::cout << "Throughput: " << std::fixed << std::setprecision(3) << usPerEval << " us/eval ("
                  << std::setprecision(0) << evalsPerSec << " evals/sec)\n";
        return 0;
    }

    std::string fen;
    for (int i = 1; i < argc; ++i)
    {
        if (i > 1) fen += " ";
        fen += argv[i];
    }

    InitializeEngine();
    EvaluationLogic::ClearEvalCacheForTesting();

    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    if (!board)
    {
        std::cerr << "Error: invalid FEN\n";
        return 1;
    }

    EvaluationBreakdown bd = EvaluationLogic::EvaluateDetailed(*board);

    std::cout << "FEN: " << fen << "\n";
    std::cout << "Side to move: " << (board->sideToMove ? "Black" : "White")
              << " | Phase: " << bd.phase << "/24\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << std::left << std::setw(22) << "Term" 
              << std::right << std::setw(10) << "White" 
              << std::setw(10) << "Black" 
              << std::setw(10) << "Net" << "\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << std::left << std::setw(22) << "Material" 
              << std::right << std::setw(10) << bd.whiteMaterial 
              << std::setw(10) << bd.blackMaterial 
              << std::setw(10) << bd.materialNet << "\n";
    std::cout << std::left << std::setw(22) << "Piece Eval (Balance)" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.pieceEvaluation << "\n";
    std::cout << std::left << std::setw(22) << "Lone King Guidance"
              << std::right << std::setw(10) << "-"
              << std::setw(10) << "-"
              << std::setw(10) << bd.loneKingMateGuidance << "\n";
    std::cout << std::left << std::setw(22) << "Bishop Pair" 
              << std::right << std::setw(10) << bd.whiteBishopPair 
              << std::setw(10) << bd.blackBishopPair 
              << std::setw(10) << bd.bishopPairNet << "\n";
    std::cout << std::left << std::setw(22) << "Mobility" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.mobilityNet << "\n";
    std::cout << std::left << std::setw(22) << "Center" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.centerNet << "\n";
    std::cout << std::left << std::setw(22) << "King Attack" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.kingAttackNet << "\n";
    std::cout << std::left << std::setw(22) << "King Placement" 
              << std::right << std::setw(10) << bd.whiteKingPlacement 
              << std::setw(10) << bd.blackKingPlacement 
              << std::setw(10) << bd.kingPlacementNet << "\n";
    std::cout << std::left << std::setw(22) << "Pawn Shield" 
              << std::right << std::setw(10) << bd.whitePawnShield 
              << std::setw(10) << bd.blackPawnShield 
              << std::setw(10) << bd.pawnShieldNet << "\n";
    std::cout << std::left << std::setw(22) << "Central Attack Pressure"
              << std::right << std::setw(10) << bd.whiteCentralKingAttackPressure
              << std::setw(10) << bd.blackCentralKingAttackPressure
              << std::setw(10) << bd.centralKingAttackPressureNet << "\n";
    std::cout << "Central attack pressure: centre=" << bd.centralGeneralOpenness << "/8"
              << " effective=" << bd.centralEffectiveOpenness << "/16"
              << " state=" << (bd.centralCentreLocked ? "locked" :
                                (bd.centralCentreOpen ? "open" : "mixed"))
              << " d/e=" << bd.centralDFileExposure << "/" << bd.centralEFileExposure
              << " | White active=" << (bd.whiteCentralKingActive ? "yes" : "no")
              << " heavy=" << bd.whiteHeavyLinePressure
              << " lines=" << bd.whiteDirectHeavyLines << "/"
              << bd.whiteOneBlockerHeavyLines << "/" << bd.whiteMultiBlockerHeavyLines
              << " bishop=" << bd.whiteBishopDiagonalPressure
              << " bishop-lines=" << bd.whiteDirectBishopLines << "/"
              << bd.whiteOneBlockerBishopLines << "/" << bd.whiteMultiBlockerBishopLines
              << " inner=" << bd.whiteInnerAttackers << "(" << bd.whiteInnerAttackContribution << ")"
              << " outer=" << bd.whiteOuterAttackers << "(" << bd.whiteOuterAttackContribution << ")"
              << " escalation=" << bd.whiteNonlinearEscalation
              << " castle=" << (bd.whiteImmediateCastling ? "yes" : "no")
              << " mitigation=" << bd.whiteCastlingMitigation
              << " | Black active=" << (bd.blackCentralKingActive ? "yes" : "no")
              << " heavy=" << bd.blackHeavyLinePressure
              << " lines=" << bd.blackDirectHeavyLines << "/"
              << bd.blackOneBlockerHeavyLines << "/" << bd.blackMultiBlockerHeavyLines
              << " bishop=" << bd.blackBishopDiagonalPressure
              << " bishop-lines=" << bd.blackDirectBishopLines << "/"
              << bd.blackOneBlockerBishopLines << "/" << bd.blackMultiBlockerBishopLines
              << " inner=" << bd.blackInnerAttackers << "(" << bd.blackInnerAttackContribution << ")"
              << " outer=" << bd.blackOuterAttackers << "(" << bd.blackOuterAttackContribution << ")"
              << " escalation=" << bd.blackNonlinearEscalation
              << " castle=" << (bd.blackImmediateCastling ? "yes" : "no")
              << " mitigation=" << bd.blackCastlingMitigation << "\n";
    std::cout << std::left << std::setw(22) << "King Danger"
              << std::right << std::setw(10) << bd.whiteKingDanger
              << std::setw(10) << bd.blackKingDanger
              << std::setw(10) << bd.kingAttackNet << "\n";
    std::cout << "  White KD: attWt=" << bd.whiteAttackerWeight << " defWt=" << bd.whiteDefenderWeight
              << " esc=" << bd.whiteEscapeSafety << " file=" << bd.whiteFilePressure
              << " diag=" << bd.whiteDiagonalPressure << " shelter=" << bd.whitePawnShelter
              << " phaseScale=" << bd.whitePhaseScale << "%\n";
    std::cout << "  Black KD: attWt=" << bd.blackAttackerWeight << " defWt=" << bd.blackDefenderWeight
              << " esc=" << bd.blackEscapeSafety << " file=" << bd.blackFilePressure
              << " diag=" << bd.blackDiagonalPressure << " shelter=" << bd.blackPawnShelter
              << " phaseScale=" << bd.blackPhaseScale << "%\n";
    std::cout << std::left << std::setw(22) << "King Safety Total" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.kingSafetyTotal << "\n";
    std::cout << std::left << std::setw(22) << "Pawn Structure" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.pawnStructureNet << "\n";
    std::cout << "  Pawn details: base=" << bd.pawnBaseNet
              << " kingRace=" << bd.passedPawnKingRaceNet
              << " minorAcc=" << bd.passedPawnMinorAccessibilityNet
              << " corridor=" << bd.passedPawnCorridorSafetyNet
              << " rookBehind=" << bd.rookBehindPassedPawnNet << "\n";
    std::cout << std::left << std::setw(22) << "Rook Connectivity" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.rookConnectionNet << "\n";
    std::cout << std::left << std::setw(22) << "Tempo" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.tempoNet << "\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << std::left << std::setw(22) << "Unscaled Total" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.unscaledTotal << "\n";
    if (bd.oppositeColorBishopScale != 1.0)
    {
        std::cout << std::left << std::setw(22) << "OCB Scale" 
                  << std::right << std::setw(10) << "-" 
                  << std::setw(10) << "-" 
                  << std::setw(10) << bd.oppositeColorBishopScale << "\n";
    }
    std::cout << std::left << std::setw(22) << "White Perspective" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.whitePerspectiveTotal << "\n";
    std::cout << std::left << std::setw(22) << "Side-to-move Total" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.sideToMoveTotal << "\n";
    std::cout << "--------------------------------------------------------\n";

    return 0;
}
