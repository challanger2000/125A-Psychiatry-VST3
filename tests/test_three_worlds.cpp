#include "three_worlds.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>

#define CHECK(x) do { if(!(x)) { std::fprintf(stderr,"P004 TEST FAIL %s:%d: %s\n",__FILE__,__LINE__,#x); std::abort(); } }while(0)
using Audio=std::vector<float>;
struct Stereo {Audio l,r;};

Stereo stimulus(std::size_t n,double rate) {
    Stereo s{Audio(n),Audio(n)};
    for(std::size_t i=0;i<n;++i) {
        const double t=static_cast<double>(i)/rate;
        const auto onset=i%static_cast<std::size_t>(rate/5.);
        const float kick=onset<1200 ? 0.72f*std::exp(-static_cast<float>(onset)*15.f/static_cast<float>(rate))
               *std::sin(6.283185307179586*135.*static_cast<double>(onset)/rate) : 0.f;
        const float base=0.25f*std::sin(6.283185307179586*117.*t)
            +0.10f*std::sin(6.283185307179586*781.*t);
        s.l[i]=base+kick;
        s.r[i]=base*0.95f+kick*0.90f+0.035f*std::sin(6.283185307179586*251.*t);
    }
    return s;
}
template<class FX>
void render(FX& fx,Stereo& signal,std::size_t block) {
    for(std::size_t i=0;i<signal.l.size();i+=block) {
        float* ptrs[]{signal.l.data()+i,signal.r.data()+i};
        fx.process(ptrs,2,std::min(block,signal.l.size()-i));
    }
}
double delta(const Stereo& dry,const Stereo& wet) {
    double d=0.,r=0.;
    for(std::size_t i=0;i<dry.l.size();++i) {
        d+=std::abs(dry.l[i]-wet.l[i])+std::abs(dry.r[i]-wet.r[i]);
        r+=std::abs(dry.l[i])+std::abs(dry.r[i]);
    }
    return d/std::max(r,1.e-9);
}
template<class FX>
double probe(FX& fx,float amount,double minDifference) {
    constexpr double rate=48000.;
    const auto input=stimulus(96000,rate);
    auto dry=input;
    for(auto* channel:{&dry.l,&dry.r}) {
        std::move_backward(channel->begin(),channel->end()-psychiatry::detail::kLinearPhaseLatency,channel->end());
        std::fill_n(channel->begin(),psychiatry::detail::kLinearPhaseLatency,0.f);
    }
    fx.prepare(rate);
    fx.setAmount(amount);
    fx.reset();
    auto wet=input;
    render(fx,wet,137);
    for(std::size_t i=0;i<wet.l.size();++i) {
        CHECK(std::isfinite(wet.l[i])&&std::isfinite(wet.r[i]));
        CHECK(std::abs(wet.l[i])<2.f && std::abs(wet.r[i])<2.f);
    }
    FX second;
    second.prepare(rate);
    second.setAmount(amount);
    second.reset();
    auto other=input;
    render(second,other,512);
    CHECK(wet.l==other.l && wet.r==other.r);
    const double change=delta(dry,wet);
    CHECK(change>=minDifference);
    if(amount==0.f) CHECK(wet.l==dry.l && wet.r==dry.r);
    return change;
}
// P026a behavioral checks: silence, source-driven events, bass harmonics.
void revisedWorlds() {
    constexpr int sr=48000;
    psychiatry::Panic p;p.prepare(sr);p.setAmount(1.f);p.reset();
    Stereo silence{Audio(sr),Audio(sr)};
    render(p,silence,127);
    CHECK(p.impulses()==0);
    for(float v:silence.l)CHECK(v==0.f);
    Stereo bass{Audio(sr*2),Audio(sr*2)};
    for(std::size_t i=0;i<bass.l.size();++i)
        bass.l[i]=bass.r[i]=.25f*std::sin(6.283185307179586*55.*i/sr);
    auto shaken=bass;render(p,shaken,127);
    CHECK(p.impulses()>=4);
    double minRms=1.,maxRms=0.;
    for(int i=sr/4;i<sr*2-960;i+=960){
        double sum=0.;
        for(int j=0;j<960;++j)sum+=shaken.l[i+j]*shaken.l[i+j];
        const double rms=std::sqrt(sum/960.);
        minRms=std::min(minRms,rms);maxRms=std::max(maxRms,rms);
    }
    CHECK(maxRms>minRms*1.5);
    CHECK(shaken.l==shaken.r);
    psychiatry::Psychosis z;z.prepare(sr);z.setAmount(1.f);z.reset();
    render(z,silence,131);for(float v:silence.l)CHECK(v==0.f);
    auto distorted=bass;render(z,distorted,131);
    auto magnitude=[&](int harmonic) {
        double a=0.,b=0.;
        for(int n=sr;n<2*sr;++n){
            double phase=6.283185307179586*55.*harmonic*n/sr;
            a+=distorted.l[n]*std::cos(phase);b+=distorted.l[n]*std::sin(phase);
        }
        return 2.*std::hypot(a,b)/sr;
    };
    const double fundamental=magnitude(1),third=magnitude(3),fifth=magnitude(5);
    CHECK(fundamental>.10); // avoid loss of EBM bass foundation
    CHECK(std::hypot(third,fifth)>.08*fundamental);
    std::printf("P026a PANIC events=%llu rms range %.4f..%.4f; PSYCHOSIS 55Hz %.4f H3 %.4f H5 %.4f\n",
       static_cast<unsigned long long>(p.impulses()),minRms,maxRms,fundamental,third,fifth);
}
// P026i: prove source replay beyond FIR latency and short filter decay,
// and increasing capture rate rather than just faster amplitude modulation.
void maniaRetriggerContract() {
    for(int sr:{44100,48000,96000}) {
        std::uint64_t previous=0;
        for(float m:{.3f,.7f,1.f}) {
            psychiatry::Mania f;f.prepare(sr);f.setAmount(m);f.reset();
            Stereo tone{Audio(2*sr),Audio(2*sr)};
            for(std::size_t i=0;i<tone.l.size();++i)
                tone.l[i]=tone.r[i]=.3f*std::sin(6.283185307179586*117.*i/sr);
            render(f,tone,137);
            CHECK(tone.l==tone.r);
            CHECK(f.retriggers()>previous);previous=f.retriggers();
            for(float x:tone.l)CHECK(std::isfinite(x)&&std::abs(x)<1.f);
        }
        psychiatry::Mania f;f.prepare(sr);f.setAmount(1.f);f.reset();
        Stereo zeros{Audio(sr),Audio(sr)};
        render(f,zeros,127);CHECK(f.retriggers()==0);
        for(float x:zeros.l)CHECK(x==0.f);
        f.reset();
        int i=0;
        for(;i<sr&&f.retriggers()==0;++i) {
            float x=.3f*std::sin(6.283185307179586*117.*i/sr);
            float* mono[]{&x};f.process(mono,1,1);
        }
        CHECK(f.retriggers()==1);
        double sum=0.;
        for(int n=0;n<sr/20;++n) {
            float x=0.f;float* mono[]{&x};f.process(mono,1,1);
            if(n>=sr*15/1000&&n<sr*22/1000)sum+=x*x;
        }
        const double rms=std::sqrt(sum/(sr*.007));
        CHECK(rms>.003); // replay remains after input has gone silent
        std::printf("P026i MANIA %d Hz: full events=%llu, repeat RMS %.6f\n",
          sr,static_cast<unsigned long long>(previous),rms);
    }
}
int main() {
    revisedWorlds();
    maniaRetriggerContract();
    psychiatry::Mania mania;
    psychiatry::Panic panic;
    psychiatry::Psychosis psychosis;
    CHECK(probe(mania,0.f,0.f)==0.);
    CHECK(probe(panic,0.f,0.f)==0.);
    CHECK(probe(psychosis,0.f,0.f)==0.);
    const double maniaLow=probe(mania,0.2f,0.005);
    const double maniaHigh=probe(mania,0.9f,0.05);
    const double panicLow=probe(panic,0.2f,0.001);
    const double panicHigh=probe(panic,0.9f,0.035);
    const double psychoLow=probe(psychosis,0.2f,0.005);
    const double psychoHigh=probe(psychosis,0.9f,0.08);
    CHECK(maniaHigh>maniaLow*1.1);
    CHECK(panicHigh>panicLow*1.1);
    CHECK(psychoHigh>psychoLow*1.1);
    CHECK(panic.impulses()>0);
    // The three worlds must differ, not represent one generic drive.
    const auto dry=stimulus(96000,48000.);
    psychiatry::Mania m; m.prepare(48000);m.setAmount(0.85f);m.reset();
    auto a=dry;render(m,a,128);
    psychiatry::Panic p; p.prepare(48000);p.setAmount(0.85f);p.reset();
    auto b=dry;render(p,b,128);
    psychiatry::Psychosis z; z.prepare(48000);z.setAmount(0.85f);z.reset();
    auto c=dry;render(z,c,128);
    CHECK(delta(a,b)>0.09);
    CHECK(delta(b,c)>0.09);
    CHECK(delta(c,a)>0.09);
    // Mono, dynamic settings, silent input, extreme but accepted sample rates.
    for(const double sr: {44100.,96000.,192000.,1234.568}) {
        psychiatry::Mania mm;psychiatry::Panic pp;psychiatry::Psychosis zz;
        mm.prepare(sr);pp.prepare(sr);zz.prepare(sr);
        mm.setAmount(0.8f);pp.setAmount(0.8f);zz.setAmount(0.8f);
        const auto n=static_cast<std::size_t>(sr*0.1);
        Audio l(n,0.2f);
        float* ptr[]={l.data()};
        for(std::size_t i=0;i<l.size();i+=17){
            const auto count=std::min<std::size_t>(17,l.size()-i);
            float* mono[]={l.data()+i};
            mm.process(mono,1,count);
            pp.process(mono,1,count);
            zz.process(mono,1,count);
        }
        for(float sample:l) CHECK(std::isfinite(sample)&&std::abs(sample)<2.f);
        (void)ptr;
    }
    // Explicit invalid-sample rejection during nonzero effect.
    {
        psychiatry::Mania mm;psychiatry::Panic pp;psychiatry::Psychosis zz;
        mm.prepare(48000);pp.prepare(48000);zz.prepare(48000);
        mm.setAmount(1.f);pp.setAmount(1.f);zz.setAmount(1.f);
        Audio l(8192,0.2f),r=l;
        l[300]=std::numeric_limits<float>::infinity();
        r[200]=std::numeric_limits<float>::quiet_NaN();
        Stereo s{l,r};render(mm,s,71);render(pp,s,71);render(zz,s,71);
        for(float v:s.l)CHECK(std::isfinite(v));
        for(float v:s.r)CHECK(std::isfinite(v));
    }
    std::printf("P004 PASS: MANIA %.3f->%.3f, PANIC %.3f->%.3f, PSYCHOSIS %.3f->%.3f. ONSET events=%llu\n",
        maniaLow,maniaHigh,panicLow,panicHigh,psychoLow,psychoHigh,
        static_cast<unsigned long long>(panic.impulses()));
}
