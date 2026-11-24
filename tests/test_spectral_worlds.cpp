#include "spectral_worlds.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>

#define CHECK(x) do { if(!(x)) { std::fprintf(stderr,"P005 TEST FAIL %s:%d: %s\n",__FILE__,__LINE__,#x); std::abort(); } }while(0)
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
    const auto dry=stimulus(96000,rate);
    fx.prepare(rate);
    fx.setAmount(amount);
    fx.reset();
    auto wet=dry;
    render(fx,wet,137);
    for(std::size_t i=0;i<wet.l.size();++i) {
        CHECK(std::isfinite(wet.l[i])&&std::isfinite(wet.r[i]));
        CHECK(std::abs(wet.l[i])<2.f && std::abs(wet.r[i])<2.f);
    }
    FX second;
    second.prepare(rate);
    second.setAmount(amount);
    second.reset();
    auto other=dry;
    render(second,other,512);
    CHECK(wet.l==other.l && wet.r==other.r);
    const double change=delta(dry,wet);
    CHECK(change>=minDifference);
    if(amount==0.f) CHECK(wet.l==dry.l && wet.r==dry.r);
    return change;
}
int main() {
    psychiatry::Schizophrenia schizophrenia;
    psychiatry::Dissociation dissociation;
    psychiatry::Hallucination hallucination;
    CHECK(probe(schizophrenia,0.f,0.f)==0.);
    CHECK(probe(dissociation,0.f,0.f)==0.);
    CHECK(probe(hallucination,0.f,0.f)==0.);
    const double schizophreniaLow=probe(schizophrenia,0.2f,0.005);
    const double schizophreniaHigh=probe(schizophrenia,0.9f,0.05);
    const double dissociationLow=probe(dissociation,0.2f,0.001);
    const double dissociationHigh=probe(dissociation,0.9f,0.035);
    const double hallucLow=probe(hallucination,0.2f,0.005);
    const double hallucHigh=probe(hallucination,0.9f,0.08);
    CHECK(schizophreniaHigh>schizophreniaLow*1.1);
    CHECK(dissociationHigh>dissociationLow*1.1);
    CHECK(hallucHigh>hallucLow*1.1);
    // The three worlds must differ, not represent one generic drive.
    const auto dry=stimulus(96000,48000.);
    psychiatry::Schizophrenia m; m.prepare(48000);m.setAmount(0.85f);m.reset();
    auto a=dry;render(m,a,128);
    psychiatry::Dissociation p; p.prepare(48000);p.setAmount(0.85f);p.reset();
    auto b=dry;render(p,b,128);
    psychiatry::Hallucination z; z.prepare(48000);z.setAmount(0.85f);z.reset();
    auto c=dry;render(z,c,128);
    CHECK(delta(a,b)>0.09);
    CHECK(delta(b,c)>0.09);
    CHECK(delta(c,a)>0.09);
    // Mono, dynamic settings, silent input, extreme but accepted sample rates.
    for(const double sr: {44100.,96000.,192000.,1234.568}) {
        psychiatry::Schizophrenia mm;psychiatry::Dissociation pp;psychiatry::Hallucination zz;
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
    // P019: full wet from first sample, not a dry startup bypass.
    for(double sr:{44100.,48000.,96000.}) {
        psychiatry::Schizophrenia fx;fx.prepare(sr);fx.setAmount(1.f);fx.reset();
        Stereo x{Audio(64,0.4f),Audio(64,-0.4f)};
        render(fx,x,17);
        for(std::size_t n=0;n<x.l.size();++n) {
            CHECK(std::abs(x.l[n]-1.10f*std::tanh(0.33f*0.4f))<1.e-6f);
            CHECK(std::abs(x.r[n]-1.10f*std::tanh(-0.33f*0.4f))<1.e-6f);
        }
    }
    // P026h: a single 700 Hz tone must generate three separated
    // pitch regions. A gain-only or fixed-delay output cannot pass.
    // Short windowed heads spread energy into nearby sidebands, so
    // inspect peak magnitude within each distinct target region.
    {
        constexpr int sr=48000;
        Stereo x{Audio(sr*2),Audio(sr*2)};
        for(std::size_t i=0;i<x.l.size();++i)
            x.l[i]=x.r[i]=.15f*std::sin(6.283185307179586*700.*i/sr);
        auto dry=x;
        psychiatry::Schizophrenia fx;fx.prepare(sr);fx.setAmount(1.f);fx.reset();
        // Mono eliminates pan as a possible explanation of spectral change.
        for(std::size_t i=0;i<x.l.size();i+=127) {
            float* p[]={x.l.data()+i};
            fx.process(p,1,std::min<std::size_t>(127,x.l.size()-i));
        }
        const auto peak=[&](const Audio& samples,int centre){
            double best=0.;
            for(int hz=centre-45;hz<=centre+45;++hz){
                double a=0.,b=0.;
                const double d=6.283185307179586*hz/sr;
                const double cs=std::cos(d),sn=std::sin(d);
                double c=1.,s=0.;
                for(int n=sr;n<2*sr;++n){
                    a+=samples[n]*c;b+=samples[n]*s;
                    const double next=c*cs-s*sn;
                    s=s*cs+c*sn;c=next;
                }
                best=std::max(best,2.*std::hypot(a,b)/sr);
            }
            return best;
        };
        for(int centre:{467,832,1049}) {
            const double original=peak(dry.l,centre);
            const double shifted=peak(x.l,centre);
            CHECK(original<.0001&&shifted>.018);
            std::printf("P026h SCHIZOPHRENIA band %d Hz dry %.6f wet %.6f\n",centre,original,shifted);
        }
        Stereo stereo=dry;
        fx.reset();render(fx,stereo,193);
        double lr=0.;
        for(int n=sr;n<2*sr;++n)lr+=std::abs(stereo.l[n]-stereo.r[n]);
        CHECK(lr/sr>.01);
    }
    // P026j: launched grains remain immutable while fresh material arrives.
    // Unlike the old frozen-loop contract, new material MUST enter at
    // later launches. Two independent signal paths test both conditions.
    for(int sr:{44100,48000,96000}) {
        const auto hop=static_cast<std::size_t>(std::round(sr*.048));
        const auto change=hop*12+1,total=hop*24;
        psychiatry::Dissociation a,b;
        a.prepare(sr);b.prepare(sr);a.setAmount(1.f);b.setAmount(1.f);a.reset();b.reset();
        Stereo x{Audio(total,.3f),Audio(total,.3f)},y=x;
        for(std::size_t n=change;n<total;++n)y.l[n]=y.r[n]=-.3f;
        render(a,x,137);render(b,y,509);
        for(std::size_t n=0;n<13*hop;++n) {
            CHECK(x.l[n]==y.l[n]&&x.r[n]==y.r[n]);
        }
        // Constant input after fill: complementary windows leave no gaps,
        // allpasses settle to unity DC; no fixed repeated silent interval.
        for(std::size_t n=16*hop;n<total;++n) {
            CHECK(x.l[n]>.30f&&x.l[n]<.65f);
            CHECK(x.r[n]>.30f&&x.r[n]<.65f);
        }
        CHECK(y.l.back()<-.30f&&y.r.back()<-.30f);
        a.reset();
        Stereo zero{Audio(total),Audio(total)};render(a,zero,193);
        for(float v:zero.l)CHECK(v==0.f);
        for(float v:zero.r)CHECK(v==0.f);
        // An isolated impulse must be causally dispersed, then die away.
        a.reset();
        Stereo impulse{Audio(total),Audio(total)};
        impulse.l[hop]=impulse.r[hop]=.5f;
        render(a,impulse,71);
        double energy=0.,difference=0.;
        for(std::size_t n=0;n<total;++n) {
            if(n<hop)CHECK(impulse.l[n]==0.f&&impulse.r[n]==0.f);
            energy+=impulse.l[n]*impulse.l[n]+impulse.r[n]*impulse.r[n];
            difference+=std::abs(impulse.l[n]-impulse.r[n]);
        }
        CHECK(energy>1.e-5&&difference>1.e-3);
        for(std::size_t n=total-hop;n<total;++n)
            CHECK(std::abs(impulse.l[n])<1.e-5f&&std::abs(impulse.r[n])<1.e-5f);
        std::printf("P026j DISSOCIATION %d Hz: continuous OLA, immutable heads, new capture, impulse energy %.6f\n",sr,energy);
    }
    // Explicit invalid-sample rejection during nonzero effect.
    {
        psychiatry::Schizophrenia mm;psychiatry::Dissociation pp;psychiatry::Hallucination zz;
        mm.prepare(48000);pp.prepare(48000);zz.prepare(48000);
        mm.setAmount(1.f);pp.setAmount(1.f);zz.setAmount(1.f);
        Audio l(8192,0.2f),r=l;
        l[300]=std::numeric_limits<float>::infinity();
        r[200]=std::numeric_limits<float>::quiet_NaN();
        Stereo s{l,r};render(mm,s,71);render(pp,s,71);render(zz,s,71);
        for(float v:s.l)CHECK(std::isfinite(v));
        for(float v:s.r)CHECK(std::isfinite(v));
    }
    std::printf("P005 PASS: SCHIZOPHRENIA %.3f->%.3f, DISSOCIATION %.3f->%.3f, HALLUCINATION %.3f->%.3f.\n",
        schizophreniaLow,schizophreniaHigh,dissociationLow,dissociationHigh,hallucLow,hallucHigh );
}
