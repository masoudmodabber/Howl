#include "StructuredNNUEEvaluator.h"
#include "Move.h"
#include "Option.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#endif

namespace {
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
__attribute__((target("avx2")))
void FirstLayerAVX2(const float* input, const float* weights, const float* bias, float* output) {
    __m256 sums0 = _mm256_loadu_ps(bias);
    __m256 sums1 = _mm256_loadu_ps(bias + 8);
    __m256 sums2 = _mm256_loadu_ps(bias + 16);
    __m256 sums3 = _mm256_loadu_ps(bias + 24);
    for (int j = 0; j < 512; ++j) {
        const __m256 x = _mm256_set1_ps(input[j]);
        const float* row = weights + j * 32;
        sums0 = _mm256_add_ps(sums0, _mm256_mul_ps(x, _mm256_loadu_ps(row)));
        sums1 = _mm256_add_ps(sums1, _mm256_mul_ps(x, _mm256_loadu_ps(row + 8)));
        sums2 = _mm256_add_ps(sums2, _mm256_mul_ps(x, _mm256_loadu_ps(row + 16)));
        sums3 = _mm256_add_ps(sums3, _mm256_mul_ps(x, _mm256_loadu_ps(row + 24)));
    }
    _mm256_storeu_ps(output, sums0);
    _mm256_storeu_ps(output + 8, sums1);
    _mm256_storeu_ps(output + 16, sums2);
    _mm256_storeu_ps(output + 24, sums3);
}

bool HasAVX2() {
    static const bool supported = [] { __builtin_cpu_init(); return __builtin_cpu_supports("avx2"); }();
    return supported;
}
#endif

void FirstLayerOrdered(const float* input, const float* transposedWeights, const float* bias, float* output) {
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
    if (HasAVX2()) {
        FirstLayerAVX2(input, transposedWeights, bias, output);
        return;
    }
#endif
    std::copy(bias, bias + 32, output);
    for (int j = 0; j < 512; ++j)
        for (int i = 0; i < 32; ++i)
            output[i] += input[j] * transposedWeights[j * 32 + i];
}
}
void StructuredNNUEEvaluator::Load(const std::string& p){weights.Load(p);const float* f1=tensor("f1.weight");for(int i=0;i<32;++i)for(int j=0;j<512;++j)transposedF1Weights[j*32+i]=f1[i*512+j];}
float* StructuredNNUEEvaluator::tensor(const char* n) const { for(const auto& t:weights.tensors)if(t.name==n)return const_cast<float*>(t.data.data()); throw std::runtime_error("missing structured tensor"); }
int StructuredNNUEEvaluator::cls(int p,bool w){int c=-1;switch(p){case 1:c=0;break;case 2:c=1;break;case 3:c=2;break;case 4:c=3;break;case 5:c=4;break;case 9:c=0;break;case 10:c=1;break;case 11:c=2;break;case 12:c=3;break;case 13:c=4;break;default:return -1;}bool own=w?p<9:p>8;return c+(own?0:5);}
void StructuredNNUEEvaluator::AddPiece(Board& b,int piece,int square) const {
    int cw=cls(piece,true); if(cw<0 || square<0 || square>=64) return; int cb=cls(piece,false);
    auto* ft=tensor("ft.weight");
    int wr=((b.nnueState.whiteKingSquare*10+cw)*64+square)*256;
    int ms=square^63, mk=b.nnueState.blackKingSquare^63;
    int br=((mk*10+cb)*64+ms)*256;
    for(int u=0;u<256;++u){b.nnueState.whiteAccumulator[u]+=ft[wr+u];b.nnueState.blackAccumulator[u]+=ft[br+u];}
}
void StructuredNNUEEvaluator::RemovePiece(Board& b,int piece,int square) const {
    int cw=cls(piece,true); if(cw<0 || square<0 || square>=64) return; int cb=cls(piece,false);
    auto* ft=tensor("ft.weight");
    int wr=((b.nnueState.whiteKingSquare*10+cw)*64+square)*256;
    int ms=square^63, mk=b.nnueState.blackKingSquare^63;
    int br=((mk*10+cb)*64+ms)*256;
    for(int u=0;u<256;++u){b.nnueState.whiteAccumulator[u]-=ft[wr+u];b.nnueState.blackAccumulator[u]-=ft[br+u];}
}
void StructuredNNUEEvaluator::MovePiece(Board& b,int piece,int from,int to) const {RemovePiece(b,piece,from);AddPiece(b,piece,to);}
void StructuredNNUEEvaluator::RebuildWhite(Board& b) const {
    auto* ft=tensor("ft.weight");auto* bias=tensor("ft_bias");int wk=b.nnueState.whiteKingSquare;
    for(int u=0;u<256;++u)b.nnueState.whiteAccumulator[u]=bias[u];
    for(int sq=0;sq<64;++sq){int p=b.positionCore.pieceAt[sq],c=cls(p,true);if(c<0)continue;int f=((wk*10+c)*64+sq)*256;for(int u=0;u<256;++u)b.nnueState.whiteAccumulator[u]+=ft[f+u];}
}
void StructuredNNUEEvaluator::RebuildBlack(Board& b) const {
    auto* ft=tensor("ft.weight");auto* bias=tensor("ft_bias");int bk=b.nnueState.blackKingSquare^63;
    for(int u=0;u<256;++u)b.nnueState.blackAccumulator[u]=bias[u];
    for(int sq=0;sq<64;++sq){int p=b.positionCore.pieceAt[sq],c=cls(p,false);if(c<0)continue;int ms=sq^63,f=((bk*10+c)*64+ms)*256;for(int u=0;u<256;++u)b.nnueState.blackAccumulator[u]+=ft[f+u];}
}
void StructuredNNUEEvaluator::Rebuild(Board& b) const {b.nnueState.whiteKingSquare=b.positionCore.kingSquare[0];b.nnueState.blackKingSquare=b.positionCore.kingSquare[1];RebuildWhite(b);RebuildBlack(b);b.nnueState.initialized=true;}
void StructuredNNUEEvaluator::UpdateAfterMove(Board& b,const Move& m,const NNUEState& previous) const {
    if(!previous.initialized){Rebuild(b);return;}
    bool castle=(m.CastleFlag&(Option::PowerTwo[0]|Option::PowerTwo[1]|Option::PowerTwo[2]|Option::PowerTwo[3]))!=0;
    int after=b.positionCore.pieceAt[m.endPlace]; bool white=after>0&&after<9,ep=(m.PublicFlag&Option::PowerTwo[6])!=0,capture=m.endPiece>0,promotion=m.promotionPiece>0,king=after==6||after==14;
    if(!castle&&!king){
        if(promotion){if(capture)RemovePiece(b,m.endPiece,m.endPlace);RemovePiece(b,white?1:9,m.beginPlace);AddPiece(b,m.promotionPiece,m.endPlace);}
        else if(ep){MovePiece(b,after,m.beginPlace,m.endPlace);RemovePiece(b,white?9:1,white?m.endPlace-8:m.endPlace+8);}
        else {if(capture)RemovePiece(b,m.endPiece,m.endPlace);MovePiece(b,after,m.beginPlace,m.endPlace);}
    } else {
        int oldWhite=previous.whiteKingSquare,oldBlack=previous.blackKingSquare;
        if(white){
            b.nnueState.whiteKingSquare=b.positionCore.kingSquare[0];RebuildWhite(b);
            if(king&&capture){int p=m.endPiece,c=cls(p,false);if(c>=0){auto* ft=tensor("ft.weight");int ms=m.endPlace^63,br=(((oldBlack^63)*10+c)*64+ms)*256;for(int u=0;u<256;++u)b.nnueState.blackAccumulator[u]-=ft[br+u];}}
            if(castle){int os=m.beginPlace+((m.CastleFlag&(Option::PowerTwo[3]|Option::PowerTwo[1]))?3:-4),ns=m.beginPlace+((m.CastleFlag&(Option::PowerTwo[3]|Option::PowerTwo[1]))?1:-1),c=cls(4,false);auto* ft=tensor("ft.weight");for(int u=0;u<256;++u){int ro=(((oldBlack^63)*10+c)*64+(os^63))*256+u,rn=(((oldBlack^63)*10+c)*64+(ns^63))*256+u;b.nnueState.blackAccumulator[u]-=ft[ro];b.nnueState.blackAccumulator[u]+=ft[rn];}}
        } else {
            b.nnueState.blackKingSquare=b.positionCore.kingSquare[1];RebuildBlack(b);
            if(king&&capture){int p=m.endPiece,c=cls(p,true);if(c>=0){auto* ft=tensor("ft.weight");int br=((oldWhite*10+c)*64+m.endPlace)*256;for(int u=0;u<256;++u)b.nnueState.whiteAccumulator[u]-=ft[br+u];}}
            if(castle){int os=m.beginPlace+((m.CastleFlag&(Option::PowerTwo[3]|Option::PowerTwo[1]))?3:-4),ns=m.beginPlace+((m.CastleFlag&(Option::PowerTwo[3]|Option::PowerTwo[1]))?1:-1),c=cls(12,true);auto* ft=tensor("ft.weight");for(int u=0;u<256;++u){int ro=((oldWhite*10+c)*64+os)*256+u,rn=((oldWhite*10+c)*64+ns)*256+u;b.nnueState.whiteAccumulator[u]-=ft[ro];b.nnueState.whiteAccumulator[u]+=ft[rn];}}
        }
    }
    b.nnueState.whiteKingSquare=b.positionCore.kingSquare[0];b.nnueState.blackKingSquare=b.positionCore.kingSquare[1];b.nnueState.initialized=true;
}
#if 0
float StructuredNNUEEvaluator::Evaluate(Board& b) const {
    if(!b.nnueState.initialized) Rebuild(b);int wk=b.nnueState.whiteKingSquare,bk=b.nnueState.blackKingSquare;auto* sb=tensor("scalar.weight");auto* sr=tensor("sres.weight");std::array<float,256> a=b.nnueState.whiteAccumulator,o=b.nnueState.blackAccumulator;bool black=b.sideToMove;const auto& s=black?o:a;const auto& q=black?a:o;std::array<float,32> h{},h2{};auto* w1=tensor("f1.weight");auto* b1=tensor("f1.bias");auto* w2=tensor("f2.weight");auto* b2=tensor("f2.bias");auto* wo=tensor("out.weight");float bo=tensor("out.bias")[0];for(int i=0;i<32;++i){float z=b1[i];for(int j=0;j<256;++j)z+=w1[i*512+j]*s[j]+w1[i*512+256+j]*q[j];h[i]=std::max(0.f,z);}for(int i=0;i<32;++i){float z=b2[i];for(int j=0;j<32;++j)z+=w2[i*32+j]*h[j];h2[i]=std::max(0.f,z);}float n=bo;for(int i=0;i<32;++i)n+=wo[i]*h2[i];float sc=0;int king=black?bk:wk;for(int sq=0;sq<64;++sq){int p=b.positionCore.pieceAt[sq],c=cls(p,!black);if(c<0)continue;int rs=(((king^63)*10+c)*64+(sq^63));int bs=c*64+(sq^63);if(!black){rs=((king*10+c)*64+sq);bs=c*64+sq;}sc+=sb[bs]+sr[rs];}return n+std::tanh(sc);}
#endif
float StructuredNNUEEvaluator::Evaluate(Board& b) const {
    if(!b.nnueState.initialized) Rebuild(b); auto* b1=tensor("f1.bias");auto* w2=tensor("f2.weight");auto* b2=tensor("f2.bias");auto* wo=tensor("out.weight");float bo=tensor("out.bias")[0];
    const auto& wa=b.nnueState.whiteAccumulator;const auto& ba=b.nnueState.blackAccumulator;bool stmBlack=b.sideToMove;const auto& s=stmBlack?ba:wa;const auto& q=stmBlack?wa:ba;std::array<float,512> input{};std::array<float,32>h{},h2{};
    for(int j=0;j<256;++j){input[j]=std::clamp(s[j],0.f,1.f);input[j+256]=std::clamp(q[j],0.f,1.f);}
    FirstLayerOrdered(input.data(),transposedF1Weights.data(),b1,h.data());for(int i=0;i<32;++i)h[i]=std::max(0.f,h[i]);
    for(int i=0;i<32;++i){float z=b2[i];for(int j=0;j<32;++j)z+=w2[i*32+j]*h[j];h2[i]=std::max(0.f,z);}
    float result=bo;for(int i=0;i<32;++i)result+=wo[i]*h2[i];return result;
}
