#include "StructuredNNUEEvaluator.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
void StructuredNNUEEvaluator::Load(const std::string& p){weights.Load(p);}
float* StructuredNNUEEvaluator::tensor(const char* n) const { for(const auto& t:weights.tensors)if(t.name==n)return const_cast<float*>(t.data.data()); throw std::runtime_error("missing structured tensor"); }
int StructuredNNUEEvaluator::cls(int p,bool w){int c=-1;switch(p){case 1:c=0;break;case 2:c=1;break;case 3:c=2;break;case 4:c=3;break;case 5:c=4;break;case 9:c=0;break;case 10:c=1;break;case 11:c=2;break;case 12:c=3;break;case 13:c=4;break;default:return -1;}bool own=w?p<9:p>8;return c+(own?0:5);}
void StructuredNNUEEvaluator::Rebuild(Board& b) const {
    auto* base=tensor("base.weight");auto* real=tensor("real.weight");auto* bias=tensor("b");int wk=b.pieces[6].front(),bk=b.pieces[14].front();
    b.nnueState.whiteAccumulator.fill(0);b.nnueState.blackAccumulator.fill(0);
    for(int u=0;u<256;++u){b.nnueState.whiteAccumulator[u]=bias[u];b.nnueState.blackAccumulator[u]=bias[u];}
    for(int sq=0;sq<64;++sq){int p=b.mainBoard[sq]; if(cls(p,true)<0)continue; int cw=cls(p,true),cb=cls(p,false);int fw=((wk*10+cw)*64+sq),fb=(((bk^63)*10+cb)*64+(sq^63)); for(int u=0;u<256;++u){b.nnueState.whiteAccumulator[u]+=base[(cw*64+sq)*256+u];b.nnueState.whiteAccumulator[u]+=real[fw*256+u];b.nnueState.blackAccumulator[u]+=base[(cb*64+(sq^63))*256+u];b.nnueState.blackAccumulator[u]+=real[fb*256+u];}}
    b.nnueState.whiteKingSquare=wk;b.nnueState.blackKingSquare=bk;b.nnueState.initialized=true;
}
float StructuredNNUEEvaluator::Evaluate(Board& b) const {
    Rebuild(b);int wk=b.nnueState.whiteKingSquare,bk=b.nnueState.blackKingSquare;auto* base=tensor("base.weight");auto* real=tensor("real.weight");auto* sb=tensor("scalar.weight");auto* sr=tensor("sres.weight");std::array<float,256> a=b.nnueState.whiteAccumulator,o=b.nnueState.blackAccumulator;bool black=b.sideToMove;const auto& s=black?o:a;const auto& q=black?a:o;std::array<float,32> h{},h2{};auto* w1=tensor("f1.weight");auto* b1=tensor("f1.bias");auto* w2=tensor("f2.weight");auto* b2=tensor("f2.bias");auto* wo=tensor("out.weight");float bo=tensor("out.bias")[0];for(int i=0;i<32;++i){float z=b1[i];for(int j=0;j<256;++j)z+=w1[i*512+j]*s[j]+w1[i*512+256+j]*q[j];h[i]=std::max(0.f,z);}for(int i=0;i<32;++i){float z=b2[i];for(int j=0;j<32;++j)z+=w2[i*32+j]*h[j];h2[i]=std::max(0.f,z);}float n=bo;for(int i=0;i<32;++i)n+=wo[i]*h2[i];float sc=0;int king=black?bk:wk;for(int sq=0;sq<64;++sq){int p=b.mainBoard[sq],c=cls(p,!black);if(c<0)continue;int rs=(((king^63)*10+c)*64+(sq^63));int bs=c*64+(sq^63);if(!black){rs=((king*10+c)*64+sq);bs=c*64+sq;}sc+=sb[bs]+sr[rs];}return n+std::tanh(sc);}
