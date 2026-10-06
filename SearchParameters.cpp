#include "SearchParameters.h"

SearchParameters::RuntimeValues& SearchParameters::Runtime()
{
    return runtimeValues;
}

SearchParameters::RuntimeValues SearchParameters::runtimeValues{};

bool SearchParameters::SetRuntimeParameter(const std::string& name,
                                            const std::string& value)
{
    try
    {
        auto& p = Runtime();
        if (name == "LMRLogarithmicScale") p.lmrLogarithmicScale = std::stod(value);
        else if (name == "LMRNonImprovingThreshold") p.lmrNonImprovingThreshold = std::stoi(value);
        else if (name == "LMRTtPvAdjustment") p.lmrTtPvAdjustment = std::stoi(value);
        else if (name == "LMRPreviousMoveCountThreshold") p.lmrPreviousMoveCountThreshold = std::stoi(value);
        else if (name == "LMRSingularAdjustment") p.lmrSingularAdjustment = std::stoi(value);
        else if (name == "LMRCutNodeAdjustment") p.lmrCutNodeAdjustment = std::stoi(value);
        else if (name == "LMRStatScoreDivisor") p.lmrStatScoreDivisor = std::stoi(value);
        else if (name == "NullMoveBase") p.nullMoveBase = std::stoi(value);
        else if (name == "NullMoveDepthSlope") p.nullMoveDepthSlope = std::stoi(value);
        else if (name == "NullMoveStatScoreLimit") p.nullMoveStatScoreLimit = std::stoi(value);
        else if (name == "NullMoveVerificationDepth") p.nullMoveVerificationDepth = std::stoi(value);
        else if (name == "ReverseFutilityDepthMargin") p.reverseFutilityDepthMargin = std::stoi(value);
        else if (name == "ParentFutilityBaseMargin") p.parentFutilityBaseMargin = std::stoi(value);
        else if (name == "ParentFutilityDepthMargin") p.parentFutilityDepthMargin = std::stoi(value);
        else if (name == "ProbCutBetaMargin") p.probCutBetaMargin = std::stoi(value);
        else if (name == "ProbCutImprovingMargin") p.probCutImprovingMargin = std::stoi(value);
        else if (name == "SETacticalDepthMargin") p.seeTacticalDepthMargin = std::stoi(value);
        else if (name == "SEEQuietBase") p.seeQuietBase = std::stoi(value);
        else if (name == "HistoryLmrStatOffset") p.historyLmrStatOffset = std::stoi(value);
        else return false;
        return true;
    }
    catch (...) { return false; }
}
