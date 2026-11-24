#include "depression.hpp"
#include "obsession.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>
#include <type_traits>

#define CHECK(expr) do {if(!(expr)){std::fprintf(stderr,"P003 CHECK FAIL %s:%d %s\n",__FILE__,__LINE__,#expr); std::abort();}}while(0)

template<class FX>
void run(FX& fx,std::vector<float>& l,std::vector<float>& r,std::size_t block) {
    for(std::size_t i=0;i<l.size();i+=block) {
        float* data[]{l.data()+i,r.data()+i};
        fx.process(data,2,std::min(block,l.size()-i));
    }
}
std::vector<float> source(std::size_t samples) {
    std::vector<float> out(samples);
    for(std::size_t i=0;i<samples;++i) {
        const double t=static_cast<double>(i)/48000.;
        const float transient=(i%8000<160)?0.7f*
            std::exp(-static_cast<float>(i%8000)*0.025f):0.f;
        out[i]=0.29f*std::sin(6.283185307179586*97.*t)
              +0.11f*std::sin(6.283185307179586*1761.*t)+transient;
    }
    return out;
}
template<class FX>
double check(FX& fx,float macro,double threshold) {
    constexpr std::size_t frames=48000*3;
    const auto input=source(frames);
    auto dry=input;
    if constexpr(std::is_same_v<FX,psychiatry::Depression>) {
        std::move_backward(dry.begin(),dry.end()-psychiatry::detail::kLinearPhaseLatency,dry.end());
        std::fill_n(dry.begin(),psychiatry::detail::kLinearPhaseLatency,0.f);
    }
    auto l=input,r=input;
    fx.prepare(48000);
    fx.setAmount(macro);
    fx.reset();
    run(fx,l,r,137);
    double diff=0,base=0;
    for(std::size_t i=0;i<frames;++i) {
        CHECK(std::isfinite(l[i]) && std::isfinite(r[i]));
        CHECK(std::abs(l[i])<2.f && std::abs(r[i])<2.f);
        diff+=std::abs(l[i]-dry[i])+std::abs(r[i]-dry[i]);
        base+=2.*std::abs(dry[i]);
    }
    const double normalized=diff/base;
    CHECK(normalized>=threshold);
    FX second;
    second.prepare(48000);
    second.setAmount(macro);
    second.reset();
    auto x=input,y=input;
    run(second,x,y,1024);
    CHECK(l==x && r==y && "block-size invariant");
    if(macro==0.f) CHECK(l==dry && r==dry && "macro zero must be bit-exact dry");
    return normalized;
}
int main() {
    psychiatry::Depression depressed;
    psychiatry::Obsession obsessed;
    CHECK(check(depressed,0.f,0.)==0.);
    CHECK(check(obsessed,0.f,0.)==0.);
    const auto d1=check(depressed,0.2f,0.001);
    const auto d2=check(depressed,0.85f,0.08);
    const auto o1=check(obsessed,0.2f,0.005);
    const auto o2=check(obsessed,0.85f,0.08);
    CHECK(d2>d1 && o2>o1);

    // Reverb must leave an audible tail after a nonzero signal.
    {
        psychiatry::Depression fx;fx.prepare(48000);fx.setAmount(1.f);fx.reset();
        std::vector<float> l(96000,0.f),r=l;
        for(std::size_t i=0;i<2000;++i) l[i]=r[i]=0.6f;
        run(fx,l,r,211);
        double tail=0;
        for(std::size_t i=12000;i<24000;++i) tail+=std::abs(l[i])+std::abs(r[i]);
        CHECK(tail>0.01);
        double late=0;
        for(std::size_t i=72000;i<84000;++i) late+=std::abs(l[i])+std::abs(r[i]);
        CHECK(late < tail*0.9 && "reverb feedback must decay after silence");
    }
    {
        psychiatry::Obsession fx;fx.prepare(44100);fx.setAmount(0.9f);fx.reset();
        std::vector<float> l(44100,0.3f),r=l;
        run(fx,l,r,19);
        CHECK(fx.retriggers()>5 && "loop engine must actually retrigger");
    }
    // invalid input and high rate safety.
    for(const auto rate : {44100.0,96000.0,192000.0}) {
        psychiatry::Depression a; psychiatry::Obsession b;
        a.prepare(rate);b.prepare(rate);a.setAmount(1.f);b.setAmount(1.f);
        const auto n=static_cast<std::size_t>(rate*0.08);
        std::vector<float> l(n,0.2f),r=l;
        l[20]=std::numeric_limits<float>::infinity();
        r[30]=std::numeric_limits<float>::quiet_NaN();
        run(a,l,r,199);
        run(b,l,r,97);
        for(std::size_t i=0;i<n;++i)
            CHECK(std::isfinite(l[i])&&std::isfinite(r[i]));
    }
    std::printf("P003 PASS: depression %.3f->%.3f, obsession %.3f->%.3f; stereo, finite, tails, loops, block-invariance\n",
        d1,d2,o1,o2);
}
