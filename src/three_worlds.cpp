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
float fold(float sample,float amount) noexcept {
    // A bounded nonlinear "fractured mirror" shaper, smoothly introduced.
    const float folded=std::sin(1.5707963268f*sample);
    return sample + amount*(folded-sample);
}
}

void Mania::prepare(double sr) noexcept {
    rate_=rate(sr);step_=pole(13.,rate_);
    for(auto& s:state_)s.saturation.prepare();
    reset();
}
void Mania::reset() noexcept {
    for(auto& s:state_) {
        s.phase=s.feedback=s.fast=s.slow=0.f;
        s.saturation.reset();
        s.dryDelay.reset();
    }
    smoothed_=amount_;mixDelay_.reset(smoothed_);
}
void Mania::setAmount(float a) noexcept {amount_=norm(a);}
void Mania::process(float* const* audio,unsigned channels,std::size_t frames) noexcept {
    if(!valid(audio,channels))return;
    const auto invRate=static_cast<float>(1./rate_);
    const auto fast= pole(92.,rate_),slow=pole(4.,rate_);
    for(std::size_t i=0;i<frames;++i) {
        smoothed_=detail::smoothToward(smoothed_,amount_,step_);
        const float m=smoothed_;
        const float mix=mixDelay_.process(m);
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=audio[ch][i];
            const float x=sane(raw);
            const float delayedRaw=s.dryDelay.process(raw);
            const float delayed=sane(delayedRaw);
            // Onsets accelerate the restless internal clock. Unlike generic
            // tremolo, envelope, feedback and ring sidebands interact.
            const float level=std::abs(x);
            s.fast+=fast*(level-s.fast);
            s.slow+=slow*(level-s.slow);
            const float urgency=std::clamp(4.f*(s.fast-s.slow),0.f,1.f);
            const float hz=2.6f + 3.0f*m + 47.f*m*m + 18.f*m*urgency;
            s.phase+=invRate*hz;
            if(s.phase>=1.f) s.phase-=1.f;
            const float oscillator=std::sin(kTau*s.phase);
            const float pulse=0.5f+0.5f*oscillator;
            const float hardPulse=pulse*pulse*(3.f-2.f*pulse);
            const float chopper=(1.f-m)*pulse+m*hardPulse;
            const float feedback=0.30f+0.36f*m;
            // P013: 2x nonlinear ring/feedback excitation with anti-alias
            // filtering before downsampling, with no bypass/Dry injection.
            const float driven=s.saturation.process(x,[&](float up) noexcept {
                const float y=std::tanh((1.f+2.5f*m)*up+feedback*s.feedback);
                s.feedback=y;
                return y;
            });
            const float ring=driven*(0.4f+0.6f*oscillator);
            const float manipulated=(0.45f+0.55f*chopper)*driven+
                 (0.22f+0.30f*m)*ring;
            // P006: isolate level from intensity. The original MANIA
            // generated +9..10 dB RMS rise and >0 dBFS peaks on normalized
            // sources at 85%. Reduce gain progressively as drive rises;
            // keep pulse/ring speed and timbral changes intact.
            const float automaticGain=1.f/(1.f+2.55f*m*m);
            const float transformed=automaticGain*manipulated;
            audio[ch][i]=mix==0.f?delayedRaw:(mix==1.f?transformed:delayed+mix*(transformed-delayed));
        }
    }
}

void Panic::prepare(double sr) noexcept {
    rate_=rate(sr);step_=pole(15.,rate_);
    for(auto& c:state_)c.saturation.prepare();
    reset();
}
void Panic::reset() noexcept {
    // Do not overwrite the prepared biquad coefficients when resetting.
    for(auto& c:state_){
        c.fast=c.slow=c.phase=c.envelope=0.f;
        c.last=c.slowHigh=0.f;
        c.cooldown=0;
        c.saturation.reset();
        c.dryDelay.reset();
    }
    state_[0].rng=0xCA125A11u;state_[1].rng=0x8BA92AE7u;
    smoothed_=amount_;mixDelay_.reset(smoothed_*(1.5f-0.5f*smoothed_));impulseCount_=0;
}
void Panic::setAmount(float a) noexcept {amount_=norm(a);}
void Panic::process(float* const* audio,unsigned channels,std::size_t frames) noexcept {
    if(!valid(audio,channels))return;
    const float fa=pole(330.,rate_),sa=pole(14.,rate_);
    const float highpass=pole(220.,rate_);
    const float decay=static_cast<float>(std::exp(-1./(rate_*0.028)));
    const auto cooldown=static_cast<unsigned>(rate_*0.030);
    const float invRate=static_cast<float>(1./rate_);
    for(std::size_t i=0;i<frames;++i) {
        smoothed_=detail::smoothToward(smoothed_,amount_,step_);
        // Smooth onset boost; P006 reported a marginal 20% PANIC response
        // on clean synth bass. Preserve both endpoints and audio smoothing.
        const float m=smoothed_*(1.5f-0.5f*smoothed_);
        const float mix=mixDelay_.process(m);
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=audio[ch][i];
            const float x=sane(raw);
            const float delayedRaw=s.dryDelay.process(raw);
            const float delayed=sane(delayedRaw);
            const float level=std::abs(x);
            s.fast+=fa*(level-s.fast);
            s.slow+=sa*(level-s.slow);
            if(s.cooldown>0)--s.cooldown;
            const float trigger=s.fast-s.slow;
            if(m>0.01f && s.cooldown==0 &&
               trigger>(0.11f-0.06f*m) && s.fast>0.06f) {
                s.envelope=std::min(1.f,0.55f+trigger);
                s.cooldown=cooldown;
                ++impulseCount_;
            }
            s.envelope*=decay;
            s.phase+=invRate*(700.f+1600.f*m+1300.f*s.envelope);
            if(s.phase>=1.f) s.phase-=1.f;
            s.slowHigh+=highpass*(x-s.slowHigh);
            const float piercing=x-s.slowHigh;
            // A bounded, input-triggered 'electric' shock. Noise is emitted
            // only during transient events; silence does not become hiss.
            s.rng^=s.rng<<13;s.rng^=s.rng>>17;s.rng^=s.rng<<5;
            const float noise=static_cast<float>(s.rng & 0xffffu)*(1.f/32768.f)-1.f;
            const float siren=std::sin(kTau*s.phase);
            const float pulse=s.envelope*(0.65f*siren+0.12f*noise);
            const float tremor=0.80f + 0.20f*std::sin(kTau*s.phase*0.037f);
            // P014: protect the fast transient/electric shaper from
            // third-harmonic foldback. Oversample ONLY the wet excitation
            // and saturator, then use a separately filtered decimator.
            // This introduces no additional original/Dry signal at 100%.
            const float excitation=x*(1.f+0.25f*m)+
                0.85f*m*pulse+0.4f*m*piercing;
            const float wet=s.saturation.process(excitation,[](float up) noexcept {
                return std::tanh(up);
            });
            // P011: energetic electric transients should not win a blind
            // level comparison simply by being ~+4.5 dB louder on voice.
            // Smooth macro-dependent wet-only trim, unity at 0%.
            const float levelTrim=1.f/(1.f+0.25f*m*m);
            const float transformed=levelTrim*wet*(1.f-0.35f*m*(1.f-tremor));
            audio[ch][i]=mix==0.f?delayedRaw:(mix==1.f?transformed:delayed+mix*(transformed-delayed));
        }
    }
}

void Psychosis::prepare(double sr) noexcept {
    rate_=rate(sr);
    step_=pole(16.,rate_);
    reset();
}
void Psychosis::reset() noexcept {
    state_={};smoothed_=amount_;mixDelay_.reset(smoothed_);
}
void Psychosis::setAmount(float a) noexcept {amount_=norm(a);}
void Psychosis::process(float* const* audio,unsigned channels,std::size_t frames) noexcept {
    if(!valid(audio,channels))return;
    const float lowPole=pole(170.,rate_);
    const float midPole=pole(2050.,rate_);
    const float envPole=pole(20.,rate_);
    for(std::size_t i=0;i<frames;++i) {
        smoothed_=detail::smoothToward(smoothed_,amount_,step_);
        const float m=smoothed_;
        const float mix=mixDelay_.process(m);
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=audio[ch][i];
            const float x=sane(raw);
            const float delayedRaw=s.dryDelay.process(raw);
            const float delayed=sane(delayedRaw);
            s.low+=lowPole*(x-s.low);
            s.lowMid+=midPole*(x-s.lowMid);
            const float low=s.low;
            const float mid=s.lowMid-low;
            const float high=x-s.lowMid;
            s.driveEnvelope+=envPole*(std::abs(x)-s.driveEnvelope);
            const float dynamic=0.2f*std::clamp(s.driveEnvelope,0.f,1.f);
            const auto upL=s.up[0].process(low);
            const auto upM=s.up[1].process(mid);
            const auto upH=s.up[2].process(high);
            std::array<float,2> shaped{};
            for(int os=0;os<2;++os) {
                const float l=std::tanh(upL[os]*(1.f+4.f*m+dynamic));
                const float mm=std::tanh(upM[os]*(1.f+12.f*m+dynamic));
                const float h=std::tanh(upH[os]*(1.f+6.f*m+dynamic));
                const float folded=fold(mm,(m>0.25f)?(m-0.25f)*0.92f:0.f);
                const float im=std::tanh((upM[os]*upH[os])*8.f*m);
                shaped[os]=0.60f*l+0.75f*folded+0.42f*h+0.28f*m*im;
            }
            const float decimated=s.down.process(shaped[0],shaped[1]);
            // Preserve the calibrated wet-only gain structure from P006.
            const float automaticGain=1.f/(1.f+1.30f*m*m);
            const float transformed=0.72f*automaticGain*decimated;
            audio[ch][i]=mix==0.f?delayedRaw:(mix==1.f?transformed:delayed+mix*(transformed-delayed));
        }
    }
}
} // namespace psychiatry
