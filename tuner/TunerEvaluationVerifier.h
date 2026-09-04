#ifndef HOWL_TUNER_EVALUATION_VERIFIER_H
#define HOWL_TUNER_EVALUATION_VERIFIER_H

#include <memory>
#include <string>
#include <vector>
#include "Board.h"
#include "BoardMaker.h"
#include "EvaluationLogic.h"
#include "Option.h"
#include "tuner/TunerEvaluationState.h"
#include "tuner/TunerEvaluator.h"
#include "tuner/TunerParameter.h"

namespace Tuner
{

struct EvaluationVerificationResult
{
    int positionsCompared = 0;
    bool allMatched = true;
    std::string firstMismatchFen = "";
    int firstMismatchIndex = -1;
    int productionScore = 0;
    int tunerScore = 0;
};

class TunerEvaluationVerifier
{
public:
    static EvaluationVerificationResult RunVerification()
    {
        Option::Initialize();
        TunerRegistry registry = TunerRegistry::CreateRegistry();
        TunerEvaluationState state;
        state.LoadFromRegistry(registry);

        const std::vector<std::string> representativeFens = {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
            "8/5k2/8/8/8/8/1P1K4/8 b - - 0 1",
            "8/8/4k3/8/8/4K3/4P3/8 w - - 0 1",
            "r1bqk2r/pp2bppp/2n1pn2/2pp4/2PP4/2NBPN2/PP3PPP/R1BQK2R w KQkq - 0 7"
        };

        EvaluationVerificationResult result;

        for (std::size_t i = 0; i < representativeFens.size(); ++i)
        {
            const auto& fen = representativeFens[i];
            std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
            if (!board)
            {
                continue;
            }

            int prodScore = EvaluationLogic::Evaluate(*board);
            int tunerScore = TunerEvaluator::Evaluate(*board, state);

            result.positionsCompared++;
            if (prodScore != tunerScore)
            {
                result.allMatched = false;
                result.firstMismatchFen = fen;
                result.firstMismatchIndex = static_cast<int>(i);
                result.productionScore = prodScore;
                result.tunerScore = tunerScore;
                return result;
            }
        }

        return result;
    }
};

} // namespace Tuner

#endif // HOWL_TUNER_EVALUATION_VERIFIER_H
