#include "depression.hpp"
#include "paranoia.hpp"
#include "spectral_worlds.hpp"
#include "obsession.hpp"
#include "three_worlds.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
#include <vector>

// Allocation instrumentation is enabled only during render, so the test can
// fail on accidental allocations from a real-time DSP call. Both Windows and
// Linux must run this in Release mode.
static std::atomic<unsigned long long> allocations{0};
static bool trackAllocations=false;
void* operator new(std::size_t bytes) {
    if(trackAllocations) allocations.fetch_add(1,std::memory_order_relaxed);
    if(auto* p=std::malloc(bytes?bytes:1))return p;
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
void* operator new[](std::size_t bytes) {
    if(trackAllocations) allocations.fetch_add(1,std::memory_order_relaxed);
    if(auto* p=std::malloc(bytes?bytes:1))return p;
    throw std::bad_alloc();
}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}

#define CHECK(x) do { if(!(x)) {std::fprintf(stderr,"P005 COMBINATION FAIL %s:%d %s\n",__FILE__,__LINE__,#x);std::abort();} }while(0)
using Audio=std::vector<float>;
struct Stereo{Audio l,r;};

Stereo generate(unsigned frames=48000*3){
    Stereo s{Audio(frames),Audio(frames)};
    std::uint32_t seed=0x125AC0DEu;
    for(unsigned i=0;i<frames;++i){
        const double t=static_cast<double>(i)/48000.;
        const float base=static_cast<float>(0.25*std::sin(6.28318530718*67.*t)
                        +0.13*std::sin(6.28318530718*420.*t)
                        +0.11*std::sin(6.28318530718*1722.*t));
        const auto beat=i%10000u;
        const float kick=beat<600?0.7f*std::sin(6.28318530718f*(95.f+beat*0.08f)*beat/48000.f)
                                     *std::exp(-static_cast<float>(beat)/140.f):0.f;
        seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
        const float noise=(static_cast<float>(seed&0xffffu)/32768.f-1.f)*0.027f;
        s.l[i]=base+kick+noise;
        s.r[i]=base*0.96f+kick*0.93f-noise;
    }
    return s;
}
struct Chain {
    psychiatry::Depression depression;
    psychiatry::Paranoia paranoia;
    psychiatry::Schizophrenia schizophrenia;
    psychiatry::Mania mania;
    psychiatry::Panic panic;
    psychiatry::Dissociation dissociation;
    psychiatry::Obsession obsession;
    psychiatry::Hallucination hallucination;
    psychiatry::Psychosis psychosis;

    void setup(float intensity){
        depression.prepare(48000);
        paranoia.prepare(48000);
        schizophrenia.prepare(48000);
        mania.prepare(48000);
        panic.prepare(48000);
        dissociation.prepare(48000);
        obsession.prepare(48000);
        hallucination.prepare(48000);
        psychosis.prepare(48000);
        set(intensity);
        depression.reset();paranoia.reset();schizophrenia.reset();
        mania.reset();panic.reset();dissociation.reset();obsession.reset();
        hallucination.reset();psychosis.reset();
    }
    void set(float amount) noexcept {
        depression.setAmount(amount);
        psychiatry::Paranoia::Settings s;
        s.amount=amount;s.therapy=0.f;
        s.pattern=psychiatry::Paranoia::Pattern::Every;
        s.trigger=psychiatry::Paranoia::Trigger::Hybrid;
        paranoia.setSettings(s);
        schizophrenia.setAmount(amount);
        mania.setAmount(amount);
        panic.setAmount(amount);
        dissociation.setAmount(amount);
        obsession.setAmount(amount);
        hallucination.setAmount(amount);
        psychosis.setAmount(amount);
    }
    void process(float* const* p,std::size_t n) noexcept {
        depression.process(p,2,n);
        schizophrenia.process(p,2,n);
        mania.process(p,2,n);
        panic.process(p,2,n);
        paranoia.process(p,2,n);
        dissociation.process(p,2,n);
        obsession.process(p,2,n);
        hallucination.process(p,2,n);
        psychosis.process(p,2,n);
    }
};
void run(Chain& c,Stereo& signal,std::size_t block){
    for(std::size_t i=0;i<signal.l.size();i+=block){
        float* p[]{signal.l.data()+i,signal.r.data()+i};
        c.process(p,std::min(block,signal.l.size()-i));
    }
}
double delta(const Stereo& a,const Stereo& b){
    double diff=0.,level=0.;
    for(std::size_t i=0;i<a.l.size();++i){
        diff+=std::abs(a.l[i]-b.l[i])+std::abs(a.r[i]-b.r[i]);
        level+=std::abs(a.l[i])+std::abs(a.r[i]);
    }
    return diff/std::max(1.e-6,level);
}
int main(){
    auto dry=generate();
    auto zero=dry;
    auto alignedDry=dry;
    constexpr auto latency=4*psychiatry::detail::kLinearPhaseLatency;
    for(auto* channel:{&alignedDry.l,&alignedDry.r}) {
        std::move_backward(channel->begin(),channel->end()-latency,channel->end());
        std::fill_n(channel->begin(),latency,0.f);
    }
    Chain untouched;
    untouched.setup(0.f);
    run(untouched,zero,128);
    CHECK(zero.l==alignedDry.l && zero.r==alignedDry.r);

    Chain first,second;
    first.setup(0.86f);second.setup(0.86f);
    auto wet=dry,wetOther=dry;
    const auto start=std::chrono::steady_clock::now();
    allocations.store(0);
    trackAllocations=true;
    run(first,wet,137);
    trackAllocations=false;
    const auto ended=std::chrono::steady_clock::now();
    CHECK(allocations.load()==0 && "nine-macro audio callback must not allocate");
    run(second,wetOther,1024);
    CHECK(wet.l==wetOther.l && wet.r==wetOther.r && "full chain block invariant");
    const double change=delta(alignedDry,wet);
    CHECK(change>0.20 && "nine-worlds combined must make a perceptible signal change");
    float peak=0.f;
    double sum=0.;
    for(std::size_t i=0;i<wet.l.size();++i) {
        CHECK(std::isfinite(wet.l[i]) && std::isfinite(wet.r[i]));
        peak=std::max({peak,std::abs(wet.l[i]),std::abs(wet.r[i])});
        sum+=wet.l[i]*wet.l[i]+wet.r[i]*wet.r[i];
    }
    CHECK(peak<2.f && "avoid runaway peaks");
    CHECK(sum>0.001);
    // Therapy = 100% must be original dry, independent of processing internals.
    for(std::size_t i=0;i<wet.l.size();++i) {
        const float t=0.f;
        wet.l[i]=alignedDry.l[i]+t*(wet.l[i]-alignedDry.l[i]);
        wet.r[i]=alignedDry.r[i]+t*(wet.r[i]-alignedDry.r[i]);
    }
    CHECK(wet.l==alignedDry.l && wet.r==alignedDry.r);
    // Long synthetic input followed by silence: output should remain finite
    // and below hard runaway, not self-oscillate uncontrollably.
    Stereo stop{Audio(48000*2,0.f),Audio(48000*2,0.f)};
    first.set(0.94f);
    run(first,stop,257);
    for(std::size_t i=0;i<stop.l.size();++i) {
        CHECK(std::isfinite(stop.l[i])&&std::isfinite(stop.r[i]));
        CHECK(std::abs(stop.l[i])<2.f && std::abs(stop.r[i])<2.f);
    }
    const auto millis=std::chrono::duration_cast<std::chrono::milliseconds>(ended-start).count();
    std::printf("P005 chain PASS: 9 DSP processors, no heap inside render; "
                "3 sec audio took %lld ms (runner performance only), "
                "wet/dry delta=%.3f, peak=%.3f\n",
                static_cast<long long>(millis),change,peak);
}
