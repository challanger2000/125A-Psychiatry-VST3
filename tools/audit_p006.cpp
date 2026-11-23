// P006: reproducible objective quality audit. These are synthesized surrogate
// sources (NOT a human listening test), and timing is runner-dependent.
#include "depression.hpp"
#include "paranoia.hpp"
#include "spectral_worlds.hpp"
#include "obsession.hpp"
#include "three_worlds.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <vector>

namespace {
constexpr double pi=3.14159265358979323846;
constexpr double sr=48000.0;
constexpr std::size_t frames=72000;
using Audio=std::vector<float>;
struct Stereo {Audio l,r;};
float sine(double hz,double time) {return static_cast<float>(std::sin(2*pi*hz*time));}
Stereo stimulus(int id) {
    Stereo out{Audio(frames),Audio(frames)};
    std::uint32_t seed=0x125A7654u;
    for(std::size_t i=0;i<frames;++i){
        const auto t=static_cast<double>(i)/sr;
        float l=0.f,r=0.f;
        if(id==0) { // clean synth-bass with harmonics and envelope
            const auto pulse=static_cast<double>(i%12000)/sr;
            const float env=0.36f+0.64f*std::exp(-static_cast<float>(pulse*5.));
            l=env*(0.45f*sine(73.416,t)+0.13f*sine(146.832,t)+
                   0.06f*sine(367.08,t));
            r=env*(0.44f*sine(73.416,t)+0.11f*sine(146.832,t)+
                   0.055f*sine(366.8,t));
        }else if(id==1){ // drums: percussive kick, snare, noise-like cymbal
            const auto beat=i%12000u;
            const auto snare=i%24000u;
            const float kick=beat<2200 ? 0.60f*
                std::exp(-static_cast<float>(beat)/720.f)*
                sine(96.+45.*std::exp(-static_cast<double>(beat)/230.),t):0.f;
            seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
            const float noise=static_cast<float>(seed&65535u)/32768.f-1.f;
            const float sn=snare>=12000 && snare<13800?
                0.25f*std::exp(-static_cast<float>(snare-12000)/350.f)*noise:0.f;
            const float hat=(i%6000u)<520?
                0.20f*std::exp(-static_cast<float>(i%6000u)/130.f)*noise:0.f;
            l=kick+sn+hat;
            r=0.96f*kick+0.88f*sn-0.70f*hat;
        }else if(id==2){ // synthesized voice-like harmonic vowel phrases
            const float phrasing=0.28f+0.72f*
               std::pow(std::max(0.f,sine(1.4,t)),2.f);
            const float f0=135.f+9.f*sine(2.1,t);
            float result=0.f;
            for(int k=1;k<=9;++k){
                const float freq=f0*k;
                const float formant1=std::exp(-std::pow((freq-530.f)/280.f,2.f));
                const float formant2=std::exp(-std::pow((freq-1510.f)/340.f,2.f));
                result+= (0.03f+0.10f*formant1+0.08f*formant2)
                         *sine(freq,t);
            }
            l=phrasing*result;
            r=phrasing*(result*0.96f+0.018f*sine(271.0,t));
        }else { // dark cinematic atmosphere, low notes and deterministic air
            seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
            const float noise=static_cast<float>(seed&65535u)/32768.f-1.f;
            const float slow=0.4f+0.6f*sine(0.35,t);
            l=0.22f*sine(41.2,t)+0.17f*sine(61.0,t)+
                0.08f*sine(412.0,t)+0.046f*slow*noise;
            r=0.21f*sine(41.8,t)+0.16f*sine(60.8,t)+
                0.09f*sine(415.0,t)-0.041f*slow*noise;
        }
        out.l[i]=l;out.r[i]=r;
    }
    return out;
}
struct Suite {
    psychiatry::Depression depression;
    psychiatry::Paranoia paranoia;
    psychiatry::Schizophrenia schizophrenia;
    psychiatry::Mania mania;
    psychiatry::Panic panic;
    psychiatry::Dissociation dissociation;
    psychiatry::Obsession obsession;
    psychiatry::Hallucination hallucination;
    psychiatry::Psychosis psychosis;

    void prepare() {
        depression.prepare(sr); paranoia.prepare(sr); schizophrenia.prepare(sr);
        mania.prepare(sr);panic.prepare(sr);dissociation.prepare(sr);
        obsession.prepare(sr);hallucination.prepare(sr);psychosis.prepare(sr);
    }
    void configure(int selected,float intensity) noexcept {
        const auto amount=[&](int n){return selected==9||selected==n?intensity:0.f;};
        depression.setAmount(amount(0));
        psychiatry::Paranoia::Settings settings;
        settings.amount=amount(1);
        settings.therapy=0.f;
        settings.pattern=psychiatry::Paranoia::Pattern::Every;
        settings.trigger=psychiatry::Paranoia::Trigger::Hybrid;
        settings.intervalMs=225.f;
        paranoia.setSettings(settings);
        schizophrenia.setAmount(amount(2));
        mania.setAmount(amount(3));
        panic.setAmount(amount(4));
        dissociation.setAmount(amount(5));
        obsession.setAmount(amount(6));
        hallucination.setAmount(amount(7));
        psychosis.setAmount(amount(8));
        depression.reset();paranoia.setSeed(0x125AABCDEFULL);paranoia.reset();
        schizophrenia.reset();mania.reset();panic.reset();dissociation.reset();
        obsession.reset();hallucination.reset();psychosis.reset();
    }
    void process(int selected,float* const* ptrs,std::size_t n) noexcept {
        if(selected==9 || selected==0)depression.process(ptrs,2,n);
        if(selected==9 || selected==2)schizophrenia.process(ptrs,2,n);
        if(selected==9 || selected==3)mania.process(ptrs,2,n);
        if(selected==9 || selected==4)panic.process(ptrs,2,n);
        if(selected==9 || selected==1)paranoia.process(ptrs,2,n);
        if(selected==9 || selected==5)dissociation.process(ptrs,2,n);
        if(selected==9 || selected==6)obsession.process(ptrs,2,n);
        if(selected==9 || selected==7)hallucination.process(ptrs,2,n);
        if(selected==9 || selected==8)psychosis.process(ptrs,2,n);
    }
};
struct Measurements {
    double dryRms=0,wetRms=0,peak=0,dc=0,delta=0,peakStep=0,wallMs=0;
};
Measurements measure(const Stereo& dry,const Stereo& wet,double milliseconds){
    Measurements m;
    double dryPower=0,wetPower=0,errorPower=0,dc=0;
    for(std::size_t i=0;i<dry.l.size();++i) {
        const std::array<float,2> a{dry.l[i],dry.r[i]};
        const std::array<float,2> b{wet.l[i],wet.r[i]};
        for(unsigned ch=0;ch<2;++ch) {
            const float v=b[ch], ref=a[ch];
            if(!std::isfinite(v)||std::abs(v)>4.f) {
                std::fprintf(stderr,"P006 FAIL: runaway or non-finite output at %zu\n",i);
                std::exit(2);
            }
            dryPower+=static_cast<double>(ref)*ref;
            wetPower+=static_cast<double>(v)*v;
            errorPower+=std::pow(static_cast<double>(v)-ref,2);
            dc+=v;
            m.peak=std::max(m.peak,static_cast<double>(std::abs(v)));
            if(i>0){
                const auto prior=ch==0?wet.l[i-1]:wet.r[i-1];
                m.peakStep=std::max(m.peakStep,static_cast<double>(std::abs(v-prior)));
            }
        }
    }
    const double n=2.*dry.l.size();
    m.dryRms=std::sqrt(dryPower/n);
    m.wetRms=std::sqrt(wetPower/n);
    m.delta=std::sqrt(errorPower/std::max(dryPower,1.e-15));
    m.dc=dc/n;m.wallMs=milliseconds;
    return m;
}
}
int main(){
    constexpr std::array<const char*,4> types{
        "synth_bass","drums","synthetic_voice_like","atmosphere"};
    constexpr std::array<const char*,10> names{
        "DEPRESSION","PARANOIA","SCHIZOPHRENIA","MANIA","PANIC",
        "DISSOCIATION","OBSESSION","HALLUCINATION","PSYCHOSIS","ALL_NINE"};
    constexpr std::array<float,3> amounts{0.20f,0.50f,0.85f};
    Suite suite;
    suite.prepare(); // outside timed audio
    std::cout<<"source,effect,amount,rms_change_db,wet_peak_dbfs,delta_rms_ratio,dc_offset,peak_step,render_ms,x_realtime,review\n";
    int warnings=0;
    int unsafeLevels=0;
    for(unsigned input=0;input<types.size();++input){
        const Stereo dry=stimulus(static_cast<int>(input));
        for(unsigned effect=0;effect<names.size();++effect){
            for(const float amount:amounts){
                auto wet=dry;
                suite.configure(static_cast<int>(effect),amount);
                const auto start=std::chrono::steady_clock::now();
                for(std::size_t i=0;i<wet.l.size();i+=128){
                    float* channels[]{wet.l.data()+i,wet.r.data()+i};
                    suite.process(static_cast<int>(effect),channels,
                                  std::min<std::size_t>(128,wet.l.size()-i));
                }
                const auto elapsed=std::chrono::duration<double,std::milli>(
                    std::chrono::steady_clock::now()-start).count();
                const auto m=measure(dry,wet,elapsed);
                const double gain=20.*std::log10(std::max(m.wetRms,1.e-15)/
                                                 std::max(m.dryRms,1.e-15));
                const double peakDb=20.*std::log10(std::max(m.peak,1.e-15));
                const double xRealtime=elapsed/(1000.*static_cast<double>(frames)/sr);
                // Warning flags are diagnostics, not a subjective quality grade.
                const bool quiet=(gain < -12.0 && amount>=0.50f);
                const bool weak=(m.delta < 0.10 && amount>=0.50f);
                const bool weakOnset=(m.delta < 0.025 && amount<=0.20f);
                const bool hot=(peakDb>0.);
                // P006 reference input is <0 dBFS. A processed signal above
                // digital full scale or a gratuitous >6 dB level increase is
                // a regression, not an artistic "extreme" effect.
                if(hot || gain>6.0)++unsafeLevels;
                const char* review=quiet?"WARN_LOW_LEVEL":
                                   weak?"WARN_LOW_DELTA":
                                   weakOnset?"WARN_WEAK_ONSET":
                                   hot?"WARN_OVER_0DBFS":"MEASURED";
                if(quiet||weak||weakOnset||hot) {
                    ++warnings;
                    std::fprintf(stderr,
                        "P006 REVIEW: %s %s at %.2f => gain=%+.2f dB, "
                        "delta=%.4f, peak=%+.2f dBFS [%s]\n",
                        types[input],names[effect],
                        static_cast<double>(amount),gain,m.delta,peakDb,review);
                }
                std::cout<<types[input]<<","<<names[effect]<<","
                         <<std::fixed<<std::setprecision(4)<<amount<<","
                         <<gain<<","<<peakDb<<","<<m.delta<<","
                         <<m.dc<<","<<m.peakStep<<","<<m.wallMs<<","
                         <<xRealtime<<","<<review<<"\n";
            }
        }
    }
    // A high warning count is intentionally NOT a failing CTest:
    // human audition must decide whether the strong transformations are useful.
    std::fprintf(stderr,"P006 objective audit completed: %d review flags out of 120 source/macro/intensity cases; %d safety-level violations\n",
                 warnings,unsafeLevels);
    if(unsafeLevels>0) {
        std::fputs("P006 FAIL: clipping or excessive gain detected by reference audit\n",stderr);
        return 3;
    }
    return 0;
}
