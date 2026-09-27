#include "BoardMaker.h"
#include "BoardInitializer.h"
#include "StructuredNNUEEvaluator.h"
#include "ExperimentalEvaluator.h"
#include "GameLogic.h"
#include "ChessStringManipulation.h"
#include "AttackPlaces.h"
#include "PieceMoves.h"
#include "MoveLogic.h"
#include "KingSetup.h"
#include "PassedPawnSetup.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

static constexpr const char* WEIGHTS="/tmp/howl-nnue-structured-v2/epoch-1.weights";
static void require(bool ok,const std::string& s){if(!ok)throw std::runtime_error(s);}
static float diff(const NNUEState&a,const NNUEState&b,bool white){float d=0;const auto&x=white?a.whiteAccumulator:a.blackAccumulator;const auto&y=white?b.whiteAccumulator:b.blackAccumulator;for(int i=0;i<256;++i)d=std::max(d,std::fabs(x[i]-y[i]));return d;}
static bool same(const NNUEState&a,const NNUEState&b,float&wd,float&bd){wd=diff(a,b,true);bd=diff(a,b,false);return wd<=1e-5f&&bd<=1e-5f&&a.whiteKingSquare==b.whiteKingSquare&&a.blackKingSquare==b.blackKingSquare;}
static std::unique_ptr<Board> make(const char* fen){return std::unique_ptr<Board>(BoardMaker::MakeInitialBoard(fen));}

struct Result{std::string name;float w=0,b=0;bool kings=false,undo=false;};
static Result one(const char* name,const char* fen,const char* uci,StructuredNNUEEvaluator& e){
    auto board=make(fen); e.Rebuild(*board); NNUEState before=board->nnueState;
    Move* move=ChessStringManipulation::ConvertTextToMove(uci,*board); require(move,"move conversion failed "+std::string(uci));
    MissingInfoAboutPrevStateFromMove info(*board,*move); GameLogic::DoMove(*board,*move,info); NNUEState inc=board->nnueState;
    std::unique_ptr<Board> rebuilt(board->MakeCopy()); e.Rebuild(*rebuilt); Result r; r.name=name; r.kings=inc.whiteKingSquare==rebuilt->nnueState.whiteKingSquare&&inc.blackKingSquare==rebuilt->nnueState.blackKingSquare; r.w=diff(inc,rebuilt->nnueState,true);r.b=diff(inc,rebuilt->nnueState,false);
    GameLogic::UndoMove(*board,*move,info);float uw,ub;r.undo=same(board->nnueState,before,uw,ub);delete move;return r;
}
static Result oneUninitialized(const char* name,const char* fen,const char* uci,StructuredNNUEEvaluator& e){
    auto board=make(fen); require(!board->nnueState.initialized,"uninitialized setup unexpectedly initialized");
    auto expected=make(fen); e.Rebuild(*expected); NNUEState before=expected->nnueState;
    Move* move=ChessStringManipulation::ConvertTextToMove(uci,*board); require(move,"move conversion failed "+std::string(uci));
    MissingInfoAboutPrevStateFromMove info(*board,*move); GameLogic::DoMove(*board,*move,info); NNUEState inc=board->nnueState;
    std::unique_ptr<Board> rebuilt(board->MakeCopy()); e.Rebuild(*rebuilt); Result r; r.name=name; r.kings=inc.whiteKingSquare==rebuilt->nnueState.whiteKingSquare&&inc.blackKingSquare==rebuilt->nnueState.blackKingSquare; r.w=diff(inc,rebuilt->nnueState,true);r.b=diff(inc,rebuilt->nnueState,false);
    GameLogic::UndoMove(*board,*move,info); float uw,ub; r.undo=same(board->nnueState,before,uw,ub); delete move; return r;
}
int main(){try{
    Option::Initialize(); AttackPlaces::Initialize(); BoardInitializer::Initialize(); PieceMoves::Initialize(); MoveLogic::Initialize(); KingSetup::Initialize(); PassedPawnSetup::Initialize(); ExperimentalEvaluator::SetMode(ExperimentalEvaluator::Mode::StructuredNNUE);
    StructuredNNUEEvaluator e;e.Load(WEIGHTS);
    std::vector<Result> rs;
    rs.push_back(one("quiet non-king","rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1","g1f3",e));
    rs.push_back(one("normal capture","4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1","e4d5",e));
    rs.push_back(one("promotion","4k3/P7/8/8/8/8/8/4K3 w - - 0 1","a7a8q",e));
    rs.push_back(one("promotion capture","1r2k3/P7/8/8/8/8/8/4K3 w - - 0 1","a7b8q",e));
    rs.push_back(one("en passant","4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1","e5d6",e));
    rs.push_back(one("king move","4k3/8/8/8/8/8/4K3/8 w - - 0 1","e2d2",e));
    rs.push_back(one("king capture","4k3/8/8/8/8/8/3pK3/8 w - - 0 1","e2d2",e));
    rs.push_back(one("castling","r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1","e1g1",e));
    rs.push_back(oneUninitialized("SetFEN before first evaluation","r1bqk2r/ppppbppp/2n5/8/4R3/5N2/PPPP1PPP/R1BQ1BK1 b kq - 0 9","d7d5",e));
    for(const auto&r:rs)std::cout<<r.name<<" | "<<r.w<<" | "<<r.b<<" | "<<(r.kings?"yes":"no")<<" | "<<(r.undo?"yes":"no")<<"\n";
    auto board=make("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");e.Rebuild(*board);std::vector<Move*> moves;std::vector<MissingInfoAboutPrevStateFromMove> infos;const char* seq[]={"e2e4","e7e5","g1f3","b8c6","f1b5","a7a6","b5a4","g8f6","e1g1","f8e7","d2d3","b7b5","a4b3","d7d6","c2c3","e8g8","b1d2","c8g4","h2h3","g4h5"};float mw=0,mb=0;bool undos=true;for(const char*u:seq){Move*m=ChessStringManipulation::ConvertTextToMove(u,*board);require(m,"sequence move conversion failed "+std::string(u));moves.push_back(m);infos.emplace_back(*board,*m);GameLogic::DoMove(*board,*m,infos.back());auto copy=std::unique_ptr<Board>(board->MakeCopy());e.Rebuild(*copy);float w,b;require(same(board->nnueState,copy->nnueState,w,b),"sequence mismatch "+std::string(u));mw=std::max(mw,w);mb=std::max(mb,b);}for(int i=19;i>=0;--i){GameLogic::UndoMove(*board,*moves[i],infos[i]);delete moves[i];}std::cout<<"random sequence: | plies tested "<<20<<" | max white diff "<<mw<<" | max black diff "<<mb<<" | all undo states matched "<<(undos?"yes":"no")<<"\n";bool pass=true;for(const auto&r:rs)pass&=r.w<=1e-5f&&r.b<=1e-5f&&r.kings&&r.undo;std::cout<<"overall "<<(pass?"PASS":"FAIL")<<"\n";return pass?0:1;
}catch(const std::exception&ex){std::cerr<<ex.what()<<"\n";return 1;}}
