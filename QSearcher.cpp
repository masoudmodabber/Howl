#include "QSearcher.h"
#include "BoardLogic.h"
#include "ChessStringManipulation.h"
#include "EvaluationLogic.h"
#include "GameLogic.h"
#include "MateScore.h"
#include "MissingInfoAboutPrevStateFromMove.h"
#include "Option.h"
#include "PVSSearch.h"
#include "RepetitionHistory.h"
#include "Search.h"
#include "SearchParameters.h"
#include "TranspositionTable.h"
#include <algorithm>
#include <array>

namespace {
constexpr int QChecks=0, QNoChecks=-1, QRecaptures=-5, Infinity=200000;
constexpr int PieceValue[7]={0,100,350,350,550,975,2500};
int Type(int p){ return p>8?p-8:p; }
bool Same(const Move& m,uint16_t p){return p&&m.beginPlace==TTMoveHelper::UnpackFrom(p)&&m.endPlace==TTMoveHelper::UnpackTo(p)&&(TTMoveHelper::UnpackPromotion(p)==0?m.promotionPiece<=0:m.promotionPiece==TTMoveHelper::UnpackPromotion(p));}
void Delete(MoveList& l){for(int i=0;i<l.count;++i)delete l.moves[i];l.count=0;}
struct Result{int value=0;std::string pv;};

class QMovePicker {
public:
    QMovePicker(Board& boardValue,int qDepthValue,int plyValue,bool inCheckValue,
                const Move& previousValue,uint16_t ttMoveValue)
        : board(boardValue),qDepth(qDepthValue),ply(plyValue),inCheck(inCheckValue),
          previous(previousValue),ttMove(ttMoveValue) {}
    ~QMovePicker(){Delete(tacticalMoves);Delete(checkMoves);Delete(evasionMoves);}
    Move* Next(){
        while(true){
            switch(stage){
            case Stage::TT:
                stage=inCheck?Stage::Evasions:Stage::Tacticals;
                if(Move* m=FindTT())return m;
                break;
            case Stage::Evasions:
                PrepareEvasions();
                while(evasionCursor<evasionCount){Move* m=evasions[evasionCursor++].move;
                    if(!Same(*m,returnedTT))return m;}
                stage=Stage::Done;break;
            case Stage::Tacticals:
                PrepareTacticals();
                while(tacticalCursor<tacticalCount){Move* m=tacticals[tacticalCursor++].move;
                    if(!Same(*m,returnedTT))return m;}
                stage=qDepth>=QChecks?Stage::Checks:Stage::Done;break;
            case Stage::Checks:
                PrepareChecks();
                while(checkCursor<checkCount){Move* m=checks[checkCursor++].move;
                    if(!Same(*m,returnedTT))return m;}
                stage=Stage::Done;break;
            case Stage::Done:return nullptr;
            }
        }
    }
private:
    struct Entry{Move* move=nullptr;int score=0;};
    enum class Stage{TT,Evasions,Tacticals,Checks,Done};
    bool EligibleTactical(const Move& m) const {
        const bool capture=m.endPiece>0||(m.PublicFlag&Option::PowerTwo[6]);
        const bool promotion=m.promotionPiece>0;
        return (capture||promotion)&&
            (qDepth>QRecaptures||m.endPlace==previous.endPlace);
    }
    int TacticalScore(const Move& m) const {
        return PVSSearch::QCaptureOrderingScore(board,m);
    }
    int EvasionScore(const Move& m) const {
        const bool capture=m.endPiece>0||(m.PublicFlag&Option::PowerTwo[6]);
        if(capture)return PieceValue[Type(m.endPiece)]-Type(board.mainBoard[m.beginPlace]);
        return PVSSearch::QQuietEvasionOrderingScore(
            board.sideToMove?1:0,ply,board,m)-(1<<28);
    }
    void Sort(Entry* entries,int count){for(int i=1;i<count;++i){Entry key=entries[i];int j=i;
        while(j>0&&key.score>entries[j-1].score){entries[j]=entries[j-1];--j;}entries[j]=key;}}
    void PrepareEvasions(){if(evasionsReady)return;evasionsReady=true;
        evasionMoves=MoveLogic::MoveGenerator(board,qDepth,ply,false);
        for(int i=0;i<evasionMoves.count;++i)evasions[evasionCount++]={evasionMoves.moves[i],EvasionScore(*evasionMoves.moves[i])};
        Sort(evasions.data(),evasionCount);}
    void PrepareTacticals(){if(tacticalsReady)return;tacticalsReady=true;
        tacticalMoves=MoveLogic::MoveGenerator(board,qDepth,ply,true,false);
        for(int i=0;i<tacticalMoves.count;++i)if(EligibleTactical(*tacticalMoves.moves[i]))
            tacticals[tacticalCount++]={tacticalMoves.moves[i],TacticalScore(*tacticalMoves.moves[i])};
        Sort(tacticals.data(),tacticalCount);}
    void PrepareChecks(){if(checksReady)return;checksReady=true;
        checkMoves=MoveLogic::MoveGenerator(board,qDepth,ply,true,true);
        for(int i=0;i<checkMoves.count;++i){Move* m=checkMoves.moves[i];
            const bool tactical=m->endPiece>0||m->promotionPiece>0||(m->PublicFlag&Option::PowerTwo[6]);
            if(!tactical){m->givesCheck=MoveLogic::MoveGivesCheck(board,*m);m->givesCheckComputed=true;
                if(m->givesCheck)checks[checkCount++]={m,0};}}
    }
    Move* FindTT(){if(!ttMove)return nullptr;
        if(inCheck){PrepareEvasions();for(int i=0;i<evasionCount;++i)if(Same(*evasions[i].move,ttMove))
            {returnedTT=ttMove;return evasions[i].move;}return nullptr;}
        PrepareTacticals();for(int i=0;i<tacticalCount;++i)if(Same(*tacticals[i].move,ttMove))
            {returnedTT=ttMove;return tacticals[i].move;}
        if(qDepth>=QChecks){PrepareChecks();for(int i=0;i<checkCount;++i)if(Same(*checks[i].move,ttMove))
            {returnedTT=ttMove;return checks[i].move;}}
        return nullptr;
    }
    Board& board;int qDepth;int ply;bool inCheck;const Move& previous;uint16_t ttMove;
    uint16_t returnedTT=0;MoveList tacticalMoves{},checkMoves{},evasionMoves{};
    std::array<Entry,256> tacticals{},checks{},evasions{};
    int tacticalCount=0,checkCount=0,evasionCount=0;
    int tacticalCursor=0,checkCursor=0,evasionCursor=0;
    bool tacticalsReady=false,checksReady=false,evasionsReady=false;
    Stage stage=Stage::TT;
};

Result SearchQ(Board& b,Move& prev,int alpha,int beta,int ply,int qDepth,bool pv)
{
    if(Search::stopRequested.load(std::memory_order_relaxed))return {0,{}};
    if(ply>0&&RepetitionHistory::IsRepetition(b.ZobristHashCode))return {0,{}};
    ++Search::searchNodeCount;if((Search::searchNodeCount&2047)==0)Search::CheckLimits();
    int side=b.sideToMove?1:0;
    bool check=BoardLogic::UnderAttack(b,b.pieces[side*8+6].front(),!b.sideToMove);
    if(ply>=PVSSearch::MaxKillerPly-1)
        return {check?0:EvaluationLogic::Evaluate(b),{}};
    int ttDepth=check||qDepth>=QChecks?QChecks:QNoChecks;
    int oldAlpha=alpha,staticEval=TT_NO_STATIC_EVAL,best=-Infinity;
    TTEntry tt{};bool hit=TranspositionTable::Probe(b.ZobristHashCode,tt);int ttValue=0;
    if(hit){ttValue=MateScore::FromTranspositionTable(tt.score,ply);uint8_t f=TTBaseFlag(tt.flag);if(!pv&&tt.depth>=ttDepth&&(f==TT_EXACT||(f==TT_LOWER_BOUND&&ttValue>=beta)||(f==TT_UPPER_BOUND&&ttValue<=alpha)))return {ttValue,{}};}
    if(!check){staticEval=hit&&tt.staticEval!=TT_NO_STATIC_EVAL?tt.staticEval:EvaluationLogic::Evaluate(b);best=staticEval;if(hit){uint8_t f=TTBaseFlag(tt.flag);if(f==TT_LOWER_BOUND&&ttValue>best)best=ttValue;else if(f==TT_UPPER_BOUND&&ttValue<best)best=ttValue;}if(best>=beta){TranspositionTable::Store(b.ZobristHashCode,MateScore::ToTranspositionTable(best,ply),ttDepth,TT_LOWER_BOUND,0,staticEval,pv);return {best,{}};}if(best>alpha)alpha=best;}
    bool includeChecks=qDepth>=QChecks;
    QMovePicker picker(b,qDepth,ply,check,prev,hit?tt.bestMove:0);
    int futilityBase=best+SearchParameters::QSearch::FutilityMargin,legal=0,moveCount=0;uint16_t bestMove=0;std::string bestPv;
    while(Move*m=picker.Next()){++moveCount;bool capture=m->endPiece>0||(m->PublicFlag&Option::PowerTwo[6]);bool promo=m->promotionPiece>0;if(!m->givesCheckComputed){m->givesCheck=MoveLogic::MoveGivesCheck(b,*m);m->givesCheckComputed=true;}bool givesCheck=m->givesCheck;if(!check&&!capture&&!promo&&!(includeChecks&&givesCheck))continue;
        const bool advancedPawn=Type(b.mainBoard[m->beginPlace])==1&&
            ((side==0&&m->endPlace/8>=5)||(side==1&&m->endPlace/8<=2));
        if(!check&&!givesCheck&&futilityBase>-MateScore::Threshold&&!advancedPawn){int victim=capture?PieceValue[Type(m->endPiece)]:0;int gain=promo?PieceValue[Type(m->promotionPiece)]-PieceValue[1]:0;if(futilityBase+victim+gain<=alpha){best=std::max(best,futilityBase+victim+gain);continue;}if(futilityBase<=alpha&&!MoveLogic::SEE_GE(b,*m,1)){best=std::max(best,futilityBase);continue;}}
        const bool evasionPrunable=check&&(qDepth!=0||moveCount>2)&&
            best>MateScore::MatedAtPly(PVSSearch::MaxKillerPly-1)&&!capture;
        if((!check||evasionPrunable)&&!MoveLogic::SEE_GE(b,*m,0))continue;
        MissingInfoAboutPrevStateFromMove u(b,*m);GameLogic::DoMove(b,*m,prev,qDepth,ply,&u);bool ok=!BoardLogic::UnderAttack(b,b.pieces[side*8+6].front(),b.sideToMove);if(!ok){GameLogic::UndoMove(b,*m,u);--moveCount;continue;}++legal;Result child=SearchQ(b,*m,-beta,-alpha,ply+1,qDepth-1,pv);int value=-child.value;GameLogic::UndoMove(b,*m,u);if(Search::stopRequested.load(std::memory_order_relaxed))break;if(value>best){best=value;bestMove=TTMoveHelper::PackMove(*m);bestPv=ChessStringManipulation::PVToString(*m,0,false,b)+(child.pv.empty()?"":" "+child.pv);if(value>alpha){alpha=value;if(value>=beta)break;}}}
    if(check&&legal==0)best=MateScore::MatedAtPly(ply);
    uint8_t flag=best>=beta?TT_LOWER_BOUND:(best<=oldAlpha?TT_UPPER_BOUND:TT_EXACT);
    TranspositionTable::Store(b.ZobristHashCode,MateScore::ToTranspositionTable(best,ply),ttDepth,flag,bestMove,staticEval,pv);return {best,bestPv};
}
#ifdef HOWL_CORRECTNESS_TESTING
QSearchTestStatistics stats;
#endif
}

int QSearcher::pieceValue100[15]={0,100,350,350,550,975,2500,0,0,100,350,350,550,975,2500};
MovePrintValue* QSearcher::QSearch(bool pv,int alpha,int beta,Move&prev,int ply,int,bool,int depth,Move&,Move&,Move&,Board&b,bool,int,bool){Result r=SearchQ(b,prev,alpha,beta,ply,depth,pv);auto*out=new MovePrintValue();out->value=r.value;out->printString=r.pv;out->bound=r.value>=beta?SearchBound::Lower:(r.value<=alpha?SearchBound::Upper:SearchBound::Exact);out->proof=ExactProof;out->selective=false;return out;}
void QSearcher::deleteMoveList(MoveList l){for(int i=0;i<l.count;++i)delete l.moves[i];}
#ifdef HOWL_CORRECTNESS_TESTING
void QSearcher::ResetTestStatistics(){stats={};}
QSearchTestStatistics QSearcher::TestStatistics(){return stats;}
#endif
