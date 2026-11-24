#include "three_worlds.hpp"
#include "parameter_smoothing.hpp"
#include <algorithm>
#include <cmath>

namespace psychiatry {
namespace {
constexpr float kTau=6.2831853071795864769f;
float norm(float x) noexcept {
    return std::isfinite(x) ? std::clamp(x,0.f,1.f) : 0.f;
}
float sane(float x) noexcept {
    return std::isfinite(x) ? std::clamp(x,-4.f,4.f) : 0.f;
}
double rate(double sr) noexcept {
    return std::isfinite(sr) && sr>=1000. && sr<=2000000. ? sr : 48000.;
}
float pole(double hz,double sr) noexcept {
    return static_cast<float>(1.-std::exp(-kTau*hz/sr));
}
bool valid(float* const* audio,unsigned c) noexcept {
    return audio && audio[0] && (c==1 || (c==2 && audio[1]));
}
// Continuous triangle wavefolder: alternating turning points at odd integers.
float fold(float x) noexcept {
    const float p=x+1.f-4.f*std::floor((x+1.f)*0.25f);
    return 1.f-std::abs(p-2.f);
}
float smooth01(float x) noexcept {
    x=std::clamp(x,0.f,1.f);return x*x*(3.f-2.f*x);
}
float asymClip(float x,float bias) noexcept {
    return std::clamp(x+bias,-1.f,1.f)-bias;
}
}

void Mania::prepare(double sr) {
    rate_=rate(sr);step_=pole(13.,rate_);
    retrigger_.prepare(rate_);
    for(auto& s:state_)s.saturation.prepare();
    reset();
}
void Mania::reset() noexcept {
    retrigger_.reset();
    for(auto& s:state_) {
        s.feedback=0.f;
        s.saturation.reset();
        s.dryDelay.reset();
    }
    smoothed_=amount_;mixDelay_.reset(smoothed_);
}
void Mania::setAmount(float a) noexcept {amount_=norm(a);}
void Mania::process(float* const* audio,unsigned channels,std::size_t frames) noexcept {
    if(!valid(audio,channels)||!retrigger_.ready())return;
    for(std::size_t i=0;i<frames;++i) {
        smoothed_=detail::smoothToward(smoothed_,amount_,step_);
        const float m=smoothed_;
        const float mix=mixDelay_.process(m);
        float level=0.f;
        for(unsigned ch=0;ch<channels;++ch)level=std::max(level,std::abs(sane(audio[ch][i])));
        const float oscillator=retrigger_.tick(level,m);
        const float pulse=.5f+.5f*oscillator;
        const float hardPulse=pulse*pulse*(3.f-2.f*pulse);
        const float chopper=(1.f-m)*pulse+m*hardPulse;
        const float feedback=.30f+.36f*m;
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=audio[ch][i],x=sane(raw);
            const float delayedRaw=s.dryDelay.process(raw),delayed=sane(delayedRaw);
            const float exc=retrigger_.excite(ch,x,m,pulse);
            // Preserve the existing oversampled nonlinear feedback and chopper.
            const float driven=s.saturation.process(exc,[&](float up) noexcept {
                const float y=std::tanh((1.f+2.5f*m)*up+feedback*s.feedback);
                s.feedback=y;
                return y;
            });
            const float ring=driven*(.4f+.6f*oscillator);
            const float manipulated=(.45f+.55f*chopper)*driven+(.22f+.30f*m)*ring;
            const float automaticGain=1.f/(1.f+2.55f*m*m);
            const float transformed=automaticGain*manipulated;
            audio[ch][i]=mix==0.f?delayedRaw:(mix==1.f?transformed:delayed+mix*(transformed-delayed));
        }
        retrigger_.advance();
    }
}

// P026a: source-derived irregular shocks, no audible oscillator.
void Panic::prepare(double sr) {
    rate_=rate(sr);step_=pole(15.,rate_);
    for(auto& c:state_) {
        c.history.assign(static_cast<std::size_t>(std::ceil(rate_*.21))+4,0.f);
        c.saturation.prepare();
    }
    reset();
}
void Panic::reset() noexcept {
    for(auto& c:state_) {
        std::fill(c.history.begin(),c.history.end(),0.f);
        c.body=c.edge=0.f;c.saturation.reset();c.dryDelay.reset();
    }
    fast_=slow_=depth_=0.f;write_=0;age_=length_=cooldown_=kind_=0;
    rng_=0xCA125A11u;impulseCount_=0;
    smoothed_=amount_;mixDelay_.reset(smoothed_*(1.5f-.5f*smoothed_));
}
void Panic::setAmount(float a) noexcept {amount_=norm(a);}
void Panic::process(float* const* audio,unsigned channels,std::size_t frames) noexcept {
    if(!valid(audio,channels)||state_[0].history.empty())return;
    const float fa=pole(85.,rate_),sa=pole(5.,rate_);
    const float bpLow=pole(220.,rate_),bpHigh=pole(2400.,rate_);
    const auto cap=state_[0].history.size();
    const auto random=[this]() noexcept {
        rng_^=rng_<<13;rng_^=rng_>>17;rng_^=rng_<<5;
        return static_cast<float>(rng_>>8)*(1.f/16777216.f);
    };
    const auto samples=[this](float seconds) noexcept {
        return std::max(1u,static_cast<unsigned>(seconds*rate_));
    };
    for(std::size_t i=0;i<frames;++i) {
        smoothed_=detail::smoothToward(smoothed_,amount_,step_);
        const float m=smoothed_*(1.5f-.5f*smoothed_);
        const float mix=mixDelay_.process(m);
        float level=0.f;
        for(unsigned ch=0;ch<channels;++ch)level=std::max(level,std::abs(sane(audio[ch][i])));
        fast_+=fa*(level-fast_);slow_+=sa*(level-slow_);
        if(cooldown_)--cooldown_;
        const bool onset=fast_>std::max(.0015f,slow_*1.42f+.0006f);
        if(!length_ && !cooldown_ && m>.015f && fast_>.0015f &&
           (onset || (m>.55f && slow_>.002f))) {
            kind_=static_cast<unsigned>(random()*3.f);
            length_=samples(.018f+(.035f+.038f*m)*random());
            cooldown_=length_+samples((.10f+.24f*random())*(1.f-.48f*m));
            depth_=.65f+.35f*random();age_=0;++impulseCount_;
        }
        float env=0.f,lag=0.f,drop=0.f;
        if(length_) {
            const float u=static_cast<float>(age_)/static_cast<float>(length_);
            env=smooth01(u/.10f)*smooth01((1.f-u)/.24f)*depth_;
            lag=kind_==2 ? 2.f*static_cast<float>(age_) :
                static_cast<float>(rate_)*(.002f+.014f*m)*smooth01(u/.7f);
            drop=kind_==0 ? .88f : .36f;
        }
        for(unsigned ch=0;ch<channels;++ch) {
            auto& c=state_[ch];
            const float raw=audio[ch][i],x=sane(raw),dry=c.dryDelay.process(raw);
            c.history[write_]=x;
            const float d=std::clamp(lag,0.f,static_cast<float>(cap-3));
            const auto whole=static_cast<std::size_t>(d);
            const float frac=d-static_cast<float>(whole);
            const auto a=(write_+cap-whole)%cap,b=(a+cap-1)%cap;
            const float fragment=c.history[a]+frac*(c.history[b]-c.history[a]);
            c.body+=bpLow*(x-c.body);c.edge+=bpHigh*((x-c.body)-c.edge);
            const float e=m*env;
            const float exc=(1.f-e*drop)*x+e*(kind_==0?.10f:.48f)*(fragment-x)
                           +(.08f*m+.70f*e)*c.edge;
            const float wet=c.saturation.process(exc,[](float up) noexcept {return std::tanh(up);});
            audio[ch][i]=mix==0.f?dry:(mix==1.f?wet:sane(dry)+mix*(wet-sane(dry)));
        }
        if(++write_==cap)write_=0;
        if(length_ && ++age_>=length_){age_=length_=0;}
    }
}

void Psychosis::prepare(double sr) noexcept {
    rate_=rate(sr);step_=pole(16.,rate_);reset();
}
void Psychosis::reset() noexcept {
    state_={};smoothed_=amount_;mixDelay_.reset(smoothed_);
}
void Psychosis::setAmount(float a) noexcept {amount_=norm(a);}
void Psychosis::process(float* const* audio,unsigned channels,std::size_t frames) noexcept {
    if(!valid(audio,channels))return;
    const float lowPole=pole(145.,rate_),midPole=pole(1750.,rate_);
    const float focusPole=pole(620.,rate_),envPole=pole(12.,rate_);
    const float dcPole=pole(5.,rate_);
    for(std::size_t i=0;i<frames;++i) {
        smoothed_=detail::smoothToward(smoothed_,amount_,step_);
        const float m=smoothed_,mix=mixDelay_.process(m);
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=audio[ch][i],x=sane(raw),dry=s.dryDelay.process(raw);
            s.low+=lowPole*(x-s.low);s.lowMid+=midPole*(x-s.lowMid);
            // A second pole reduces ultrasonic leakage into the strong nonlinear
            // midband path. The complementary residual retains upper-band
            // content; no extra fixed latency or reduction of bass drive.
            s.lowMidFiltered+=midPole*(s.lowMid-s.lowMidFiltered);
            s.focus+=focusPole*(s.lowMidFiltered-s.focus);
            const float low=s.low,mid=s.lowMidFiltered-low,high=x-s.lowMidFiltered;
            const float emphasis=(s.lowMidFiltered-s.focus)*(.65f*m);
            s.driveEnvelope+=envPole*(std::abs(x)-s.driveEnvelope);
            const float ref=std::clamp(2.1f*s.driveEnvelope,.035f,.48f);
            const auto upL=s.up[0].process(low);
            const auto upM=s.up[1].process(mid+emphasis);
            const auto upH=s.up[2].process(high);
            std::array<float,2> shaped{};
            for(int os=0;os<2;++os) {
                const float l=upL[os]/ref,md=upM[os]/ref;
                const float bass=ref*(.58f*std::tanh(l*(1.f+2.8f*m))+
                                      .42f*asymClip(l*(1.f+3.8f*m),.12f*m));
                const float drive=md*(1.f+9.f*m*m)+l*(2.5f*m*m);
                const float clip=asymClip(drive,.23f*m),folded=fold(drive);
                const float foldMix=.43f*m*m;
                const float fracture=ref*(clip*(1.f-foldMix)+folded*foldMix);
                const float h=std::tanh(upH[os]*(1.f+.65f*m));
                const float im=ref*.26f*m*m*std::tanh(4.f*l*md);
                const float sum=.78f*bass+.67f*fracture+.72f*h+im;
                shaped[os]=.90f*std::tanh(sum/.90f);
            }
            const float wet=s.down.process(shaped[0],shaped[1]);
            s.dc+=dcPole*(wet-s.dc);
            const float y=wet-s.dc;
            audio[ch][i]=mix==0.f?dry:(mix==1.f?y:sane(dry)+mix*(y-sane(dry)));
        }
    }
}
} // namespace psychiatry
