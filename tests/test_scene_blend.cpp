#include "../vst3/scene_blend.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
using namespace psychiatry::vst3;
int main(){
    for(int l=0;l<=100;++l)for(int f=0;f<=1000;++f){
        const float level=l/100.f,fade=f/1000.f;
        const auto a=sceneAlpha(level,fade);
        const double oldWeight=a.previous*(1.-a.current),newWeight=a.current;
        const double expectedOld=double(level)*(1.-fade),expectedNew=double(level)*fade;
        if(std::abs(oldWeight-expectedOld)>2.e-7||std::abs(newWeight-expectedNew)>2.e-7||
           std::abs(oldWeight+newWeight-level)>3.e-7) return 1;
    }
    // Audit example: old white -> new black at level .75.
    const auto nearEnd=sceneAlpha(.75f,.9999f);
    if(nearEnd.previous*(1.f-nearEnd.current)>.0001f)return 2;
    std::puts("P028k A08 101101 compositing cases PASS");
}
