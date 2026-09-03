#ifndef SEARCH_ACCOUNTING_H
#define SEARCH_ACCOUNTING_H

#include <cstdint>
#include <string>
#include <map>
#include <iostream>

struct SearchAccounting
{
    uint64_t pvsNodes = 0;
    uint64_t qsearchNodes = 0;
    std::map<std::string, uint64_t> rootMoveNodes;
    uint64_t lmrReducedNodes = 0;
    uint64_t lmrRetryNodes = 0;
    uint64_t lmrInitialCalls = 0;
    uint64_t lmrSmallerRetryCalls = 0;
    uint64_t lmrFullRetryCalls = 0;
    uint64_t mateLmrSmallerRetryCalls = 0;
    uint64_t mateLmrFullRetryCalls = 0;
    uint64_t mateLmrSmallerRetryNodes = 0;
    uint64_t mateLmrFullRetryNodes = 0;
    uint64_t preprobeNodes = 0;
    uint64_t nullMoveNodes = 0;
    uint64_t preprobeQSearchNodes = 0;
    bool preprobeActive = false;
    uint64_t ttProbes = 0;
    uint64_t ttHits = 0;
    uint64_t ttCutoffs = 0;

    void reset()
    {
        pvsNodes = 0;
        qsearchNodes = 0;
        rootMoveNodes.clear();
        lmrReducedNodes = 0;
        lmrRetryNodes = 0;
        lmrInitialCalls = 0;
        lmrSmallerRetryCalls = 0;
        lmrFullRetryCalls = 0;
        mateLmrSmallerRetryCalls = 0;
        mateLmrFullRetryCalls = 0;
        mateLmrSmallerRetryNodes = 0;
        mateLmrFullRetryNodes = 0;
        preprobeNodes = 0;
        nullMoveNodes = 0;
        preprobeQSearchNodes = 0;
        preprobeActive = false;
        ttProbes = 0;
        ttHits = 0;
        ttCutoffs = 0;
    }

    void printSummary() const
    {
        std::cout << "info accounting pvs " << pvsNodes
                  << " qs " << qsearchNodes
                  << " lmr_reduced " << lmrReducedNodes
                  << " lmr_retry " << lmrRetryNodes
                  << " lmr_initial_calls " << lmrInitialCalls
                  << " lmr_smaller_retry_calls " << lmrSmallerRetryCalls
                  << " lmr_full_retry_calls " << lmrFullRetryCalls
                  << " mate_lmr_smaller_retry_calls " << mateLmrSmallerRetryCalls
                  << " mate_lmr_full_retry_calls " << mateLmrFullRetryCalls
                  << " mate_lmr_smaller_retry_nodes " << mateLmrSmallerRetryNodes
                  << " mate_lmr_full_retry_nodes " << mateLmrFullRetryNodes
                  << " preprobe " << preprobeNodes
                  << " null " << nullMoveNodes
                  << " preprobe_qs=" << preprobeQSearchNodes
                  << " tt_probes " << ttProbes
                  << " tt_hits " << ttHits
                  << " tt_cutoffs " << ttCutoffs
                  << " root_moves:";
        for (const auto &kv : rootMoveNodes)
        {
            std::cout << " " << kv.first << ":" << kv.second;
        }
        std::cout << std::endl;
    }
};

extern SearchAccounting g_searchAccounting;

#endif // SEARCH_ACCOUNTING_H
