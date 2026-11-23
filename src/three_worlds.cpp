#include "three_worlds.hpp"
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
    rate_=rate(sr);step_=pole(13.,rate_);reset();
}
void Mania::reset() noexcept {
    state_={};smoothed_=amount_;
}
void Mania::setAmount(float a) noexcept {amount_=norm(a);}
void Mania::process(float* const* audio,unsigned channels,std::size_t frames) noexcept {
    if(!valid(audio,channels))return;
    const auto invRate=static_cast<float>(1./rate_);
    const auto fast= pole(92.,rate_),slow=pole(4.,rate_);
    for(std::size_t i=0;i<frames;++i) {
        smoothed_+=step_*(amount_-smoothed_);
        const float m=smoothed_;
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=audio[ch][i];
            const float x=sane(raw);
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
            const float driven=std::tanh((1.f+2.5f*m)*x + feedback*s.feedback);
            s.feedback=driven;
            const float ring=driven*(0.4f+0.6f*oscillator);
            const float manipulated=(0.45f+0.55f*chopper)*driven+
                 (0.22f+0.30f*m)*ring;
            // P006: isolate level from intensity. The original MANIA
            // generated +9..10 dB RMS rise and >0 dBFS peaks on normalized
            // sources at 85%. Reduce gain progressively as drive rises;
            // keep pulse/ring speed and timbral changes intact.
            const float automaticGain=1.f/(1.f+2.55f*m*m);
            const float transformed=automaticGain*manipulated;
            if(m>0.f) audio[ch][i]=x+m*(transformed-x);
        }
    }
}

void Panic::prepare(double sr) noexcept {
    rate_=rate(sr);step_=pole(15.,rate_);reset();
}
void Panic::reset() noexcept {
    state_={};
    state_[0].rng=0xCA125A11u;state_[1].rng=0x8BA92AE7u;
    smoothed_=amount_;impulseCount_=0;
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
        smoothed_+=step_*(amount_-smoothed_);
        // Smooth onset boost; P006 reported a marginal 20% PANIC response
        // on clean synth bass. Preserve both endpoints and audio smoothing.
        const float m=smoothed_*(1.5f-0.5f*smoothed_);
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=audio[ch][i];
            const float x=sane(raw);
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
            const float wet=std::tanh(x*(1.f+0.25f*m)+
                0.85f*m*pulse+0.4f*m*piercing);
            // P011: energetic electric transients should not win a blind
            // level comparison simply by being ~+4.5 dB louder on voice.
            // Smooth macro-dependent wet-only trim, unity at 0%.
            const float levelTrim=1.f/(1.f+0.25f*m*m);
            const float transformed=levelTrim*wet*(1.f-0.35f*m*(1.f-tremor));
            if(m>0.f) audio[ch][i]=x+m*(transformed-x);
        }
    }
}

void Psychosis::prepare(double sr) noexcept {
    rate_=rate(sr);
    step_=pole(16.,rate_);
    // P012 anti-aliasing: true 2x filtered upsample/decimate. A 6th-order
    // Butterworth IIR with a cutoff at ~0.41 of the original rate is used
    // twice (before nonlinear processing AND before downsampling).
    // Coefficients are computed ONLY during prepare(), never in process().
    // The three stable SOS Q values are the sixth-order Butterworth poles.
    constexpr std::array<double,3> q{
        0.5176380902050415,0.7071067811865476,1.9318516525781366};
    constexpr double omega=6.2831853071795864769*0.205;
    const double sine=std::sin(omega),cosine=std::cos(omega);
    for(std::size_t k=0;k<3;++k) {
        const double alpha=sine/(2.*q[k]);
        const double a0=1.+alpha;
        oversampleCoeffs_[k].b0=
            static_cast<float>((1.-cosine)/(2.*a0));
        oversampleCoeffs_[k].b1=
            static_cast<float>((1.-cosine)/a0);
        oversampleCoeffs_[k].b2=oversampleCoeffs_[k].b0;
        oversampleCoeffs_[k].a1=
            static_cast<float>(-2.*cosine/a0);
        oversampleCoeffs_[k].a2=
            static_cast<float>((1.-alpha)/a0);
    }
    reset();
}
void Psychosis::reset() noexcept {
    state_={};smoothed_=amount_;
}
void Psychosis::setAmount(float a) noexcept {amount_=norm(a);}
void Psychosis::process(float* const* audio,unsigned channels,std::size_t frames) noexcept {
    if(!valid(audio,channels))return;
    const float lowPole=pole(170.,rate_);
    const float midPole=pole(2050.,rate_);
    const float envPole=pole(20.,rate_);
    // P012: prefiltered 2x nonlinear wet branch with a separate lowpass
    // decimator. Filter state is per-instance and fully preallocated.
    const auto filtering=[this](std::array<Biquad,3>& stages,float sample) noexcept {
        for(std::size_t section=0;section<stages.size();++section)
            sample=stages[section].process(sample,oversampleCoeffs_[section]);
        return sample;
    };
    for(std::size_t i=0;i<frames;++i) {
        smoothed_+=step_*(amount_-smoothed_);
        const float m=smoothed_;
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=audio[ch][i];
            const float x=sane(raw);
            s.low+=lowPole*(x-s.low);
            s.lowMid+=midPole*(x-s.lowMid);
            const float low=s.low;
            const float mid=s.lowMid-low;
            const float high=x-s.lowMid;
            s.driveEnvelope+=envPole*(std::abs(x)-s.driveEnvelope);
            const float dynamic=0.2f*std::clamp(s.driveEnvelope,0.f,1.f);
            // Two streamed oversamples per input frame: [2*x, 0] in
            // each of three prefiltered bands. An IIR reconstruction stage
            // suppresses the 33-kHz image of a 15-kHz source BEFORE the
            // nonlinear shapers see it; the output IIR suppresses harmonics
            // above the real host Nyquist BEFORE 2:1 decimation.
            // No history arrays resize or allocate within an audio callback.
            float decimated=0.f;
            for(int os=0;os<2;++os) {
                const float inLow=filtering(s.up[0],os==0?2.f*low:0.f);
                const float inMid=filtering(s.up[1],os==0?2.f*mid:0.f);
                const float inHigh=filtering(s.up[2],os==0?2.f*high:0.f);
                const float l=std::tanh(inLow*(1.f+4.f*m+dynamic));
                const float mm=std::tanh(inMid*(1.f+12.f*m+dynamic));
                const float h=std::tanh(inHigh*(1.f+6.f*m+dynamic));
                const float folded=fold(mm,(m>0.25f)?(m-0.25f)*0.92f:0.f);
                const float im=std::tanh((inMid*inHigh)*8.f*m);
                const float shaped=0.60f*l+0.75f*folded+0.42f*h+
                    0.28f*m*im;
                const float filtered=filtering(s.down,shaped);
                if(os==0)decimated=filtered;
            }
            // Preserve the calibrated wet-only gain structure from P006.
            const float automaticGain=1.f/(1.f+1.30f*m*m);
            const float transformed=0.72f*automaticGain*decimated;
            if(m>0.f) audio[ch][i]=x+m*(transformed-x);
        }
    }
}
} // namespace psychiatry
