#include "StructuredNNUEWeights.h"
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <array>

namespace { template<class T> T read(std::ifstream& f){ T x{}; f.read(reinterpret_cast<char*>(&x),sizeof(x)); if(!f) throw std::runtime_error("truncated structured NNUE file"); return x; } }
void StructuredNNUEWeights::Load(const std::string& path){
    std::ifstream f(path,std::ios::binary); if(!f) throw std::runtime_error("cannot open structured NNUE file");
    std::array<char,8> magic{}; f.read(magic.data(),8); if(std::string(magic.data(),8)!="HOWLSTRC") throw std::runtime_error("bad structured NNUE magic");
    if(read<std::uint32_t>(f)!=2) throw std::runtime_error("bad structured NNUE version");
    const std::uint32_t count=read<std::uint32_t>(f); if(count!=8) throw std::runtime_error("bad structured NNUE tensor count");
    const std::array<std::string,8> names={"ft.weight","ft_bias","f1.weight","f1.bias","f2.weight","f2.bias","out.weight","out.bias"};
    const std::array<std::vector<std::uint32_t>,8> shapes={std::vector<std::uint32_t>{40960,256},{256},{32,512},{32},{32,32},{32},{1,32},{1}};
    tensors.clear();
    for(std::uint32_t i=0;i<count;++i){ StructuredTensor t; auto n=read<std::uint32_t>(f); auto rank=read<std::uint32_t>(f); t.name.resize(n); f.read(t.name.data(),n); if(!f||t.name!=names[i]) throw std::runtime_error("bad structured NNUE tensor name"); if(rank!=shapes[i].size()) throw std::runtime_error("bad structured NNUE rank"); t.shape.resize(rank); std::uint64_t total=1; for(auto& d:t.shape){d=read<std::uint32_t>(f); total*=d;} if(t.shape!=shapes[i]) throw std::runtime_error("bad structured NNUE shape"); t.data.resize(total); f.read(reinterpret_cast<char*>(t.data.data()),static_cast<std::streamsize>(total*sizeof(float))); if(!f) throw std::runtime_error("truncated structured NNUE tensor"); tensors.push_back(std::move(t)); }
    char extra; if(f.read(&extra,1)) throw std::runtime_error("unexpected structured NNUE trailing data");
}
std::size_t StructuredNNUEWeights::ParameterCount() const { std::size_t n=0; for(const auto& t:tensors)n+=t.data.size(); return n; }
