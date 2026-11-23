#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"P006 Hilbert regression failure line %d\n",__LINE__);std::abort();}}while(0)
int main(){
    // Guard the actual FIR coefficient optimization, including tap signs.
    constexpr std::array<int,8> k{-7,-5,-3,-1,1,3,5,7};
    constexpr std::array<float,8> taps{
      -0.010460155335f,-0.046341542172f,-0.151947214432f,-0.614328266812f,
       0.614328266812f, 0.151947214432f, 0.046341542172f, 0.010460155335f
    };
    constexpr double pi=3.14159265358979323846;
    for(std::size_t i=0;i<k.size();++i){
        const float w=0.54f+0.46f*std::cos(static_cast<float>(pi)*static_cast<float>(k[i])/8.f);
        const float legacy=2.f/(static_cast<float>(pi)*static_cast<float>(k[i]))*w;
        CHECK(std::abs(legacy-taps[i])<2.e-7f);
        CHECK(std::abs(taps[i]+taps[k.size()-1-i])<1.e-7f);
    }
    std::puts("P006 PASS: Hilbert taps match original Hamming window within float precision");
}
