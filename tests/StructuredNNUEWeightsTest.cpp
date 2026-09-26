#include "StructuredNNUEWeights.h"
#include <iostream>
int main(int argc,char**argv){try{if(argc!=2)return 2;StructuredNNUEWeights w;w.Load(argv[1]);std::cout<<"parameter_count "<<w.ParameterCount()<<"\ntensor_count "<<w.tensors.size()<<'\n';for(const auto&t:w.tensors)std::cout<<t.name<<' '<<t.data.front()<<' '<<t.data.back()<<'\n';return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
