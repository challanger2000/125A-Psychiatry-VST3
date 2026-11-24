#include "creative_worlds.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"P026k FAIL line %d: %s\n",__LINE__,#x);std::abort();}}while(0)
using Audio=std::vector<float>;
template<class FX> Audio render(const Audio& in,int sr,float amount,int block){
    FX f;f.prepare(sr);f.setAmount(amount);f.reset();auto out=in;
    for(std::size_t i=0;i<out.size();i+=block){
        float* p[]{out.data()+i};f.process(p,1,std::min<std::size_t>(block,out.size()-i));
    }
    for(float v:out)CHECK(std::isfinite(v)&&std::abs(v)<4.f);
    return out;
}
double magnitude(const Audio& a,int from,int to,int sr,double hz){
    double r=0.,i=0.;for(int n=from;n<to;++n){
        const double phase=2.*psychiatry::creative::pi*hz*n/sr;r+=a[n]*std::cos(phase);i+=a[n]*std::sin(phase);
    }return 2.*std::hypot(r,i)/(to-from);
}
template<class FX> void common(int sr,const Audio& input){
    CHECK(render<FX>(input,sr,0.f,127)==input);
    const auto wet=render<FX>(input,sr,1.f,127);
    CHECK(wet==render<FX>(input,sr,1.f,509));
    Audio zero(input.size());CHECK(render<FX>(zero,sr,1.f,71)==zero);
    double energy=0.,delta=0.;for(std::size_t i=0;i<wet.size();++i){energy+=double(wet[i])*wet[i];delta+=std::abs(wet[i]-input[i]);}
    CHECK(energy>1.e-4&&delta>1.);
}
int main(){
    for(int sr:{44100,48000,96000}){
        Audio input(4*sr);
        for(int n=0;n<4*sr;++n)input[n]=.22f*std::sin(2.*psychiatry::creative::pi*700.*n/sr);
        common<psychiatry::Delirium>(sr,input);
        common<psychiatry::Catatonia>(sr,input);
        common<psychiatry::Amnesia>(sr,input);
        const auto delirium=render<psychiatry::Delirium>(input,sr,1.f,137);
        int minimum=100000,maximum=0;
        for(int start=sr/2;start<3*sr;start+=sr/10){
            int crossings=0;for(int n=start+1;n<start+sr/10;++n)
                if(delirium[n]>=0.f&&delirium[n-1]<0.f)++crossings;
            minimum=std::min(minimum,crossings);maximum=std::max(maximum,crossings);
        }
        CHECK(minimum<63||maximum>77);CHECK(maximum-minimum>14);
        Audio changed=input;
        for(int n=sr/3;n<4*sr;++n)changed[n]=.22f*std::sin(2.*psychiatry::creative::pi*1300.*n/sr);
        const auto freeze=render<psychiatry::Catatonia>(changed,sr,1.f,173);
        double held=0.,fresh=0.;
        for(int hz=660;hz<=740;hz+=2)held=std::max(held,magnitude(freeze,sr/2,sr,sr,hz));
        for(int hz=1260;hz<=1340;hz+=2)fresh=std::max(fresh,magnitude(freeze,sr/2,sr,sr,hz));
        CHECK(held>.025&&held>5.*fresh);
        CHECK(magnitude(changed,sr/2,sr,sr,1300.)>.20);
        Audio dc(4*sr,.3f);const auto missing=render<psychiatry::Amnesia>(dc,sr,1.f,137);
        const auto extremes=std::minmax_element(missing.begin()+sr/2,missing.end());
        CHECK(*extremes.first<.012f&&*extremes.second>.27f);
        double average=0.;for(int n=sr/2;n<4*sr;++n)average+=std::abs(missing[n]);
        CHECK(average/(3.5*sr)>.05);
        std::printf("P026k %d Hz: DELIRIUM crossings %d..%d, CATATONIA held %.5f fresh %.5f, AMNESIA min %.5f max %.5f\n",
            sr,minimum,maximum,held,fresh,*extremes.first,*extremes.second);
    }
}
