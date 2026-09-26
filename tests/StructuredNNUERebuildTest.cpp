#include "StructuredNNUEEvaluator.h"
#include "BoardInitializer.h"
#include <fstream>
#include <iostream>
#include <sstream>
static Board FromFen(const std::string& fen){Board b=*BoardInitializer::beginBoard;for(int i=0;i<64;++i)b.mainBoard[i]=0;for(auto& p:b.pieces)p.count=0;std::istringstream in(fen);std::string placement,stm;in>>placement>>stm;int sq=56;for(char c:placement){if(c=='/'){sq-=16;continue;}if(c>='1'&&c<='8'){sq+=c-'0';continue;}int id=0;switch(c){case'P':id=1;break;case'N':id=2;break;case'B':id=3;break;case'R':id=4;break;case'Q':id=5;break;case'K':id=6;break;case'p':id=9;break;case'n':id=10;break;case'b':id=11;break;case'r':id=12;break;case'q':id=13;break;case'k':id=14;break;}b.mainBoard[sq]=id;b.pieces[id].push_back(sq);++sq;}b.sideToMove=(stm=="b");return b;}
int main(int argc,char**argv){try{if(argc!=4)return 2;BoardInitializer::Initialize();StructuredNNUEEvaluator e;e.Load(argv[1]);std::ifstream in(argv[2]);std::ofstream out(argv[3]);std::string line;int n=0;while(n<100&&std::getline(in,line)){if(line.empty()||line.rfind("fen",0)==0)continue;auto tab=line.find('\t');if(tab!=std::string::npos)line.resize(tab);Board b=FromFen(line);out<<e.Evaluate(b)<<'\n';++n;}std::cout<<"parameter_count "<<e.ParameterCount()<<"\nfen_count "<<n<<"\n";return n==100?0:1;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
