#include "spectral_worlds.hpp"
#include "parameter_smoothing.hpp"
#include <algorithm>
#include <cmath>

namespace psychiatry {
namespace {
constexpr float kTau=6.2831853071795864769f;
float norm(float v) noexcept {return std::isfinite(v)?std::clamp(v,0.f,1.f):0.f;}
float sane(float v) noexcept {return std::isfinite(v)?std::clamp(v,-4.f,4.f):0.f;}
double saneRate(double sr) noexcept {
    return std::isfinite(sr)&&sr>=1000.&&sr<=2000000.?sr:48000.;
}
float coeff(double hz,double sr) noexcept {
    return static_cast<float>(1.-std::exp(-kTau*hz/sr));
}
bool valid(float* const* io,unsigned ch) noexcept {
    return io&&io[0]&&(ch==1||(ch==2&&io[1]));
}
float linearRead(const std::vector<float>& ring,std::size_t write,float delay) noexcept {
    if(ring.empty())return 0.f;
    const auto size=ring.size();
    const float d=std::clamp(delay,1.f,static_cast<float>(size-3));
    const std::size_t n=static_cast<std::size_t>(d);
    const float frac=d-static_cast<float>(n);
    const auto a=(write+size-n)%size, b=(write+size-(n+1))%size;
    return ring[a]*(1.f-frac)+ring[b]*frac;
}
} // namespace

namespace {
// Independent voice periods and interval targets. Two complementary
// Hann heads per voice, not shared modulation or a fixed slapback delay.
constexpr std::array<float,3> kVoiceSeconds{.034f,.047f,.029f};
constexpr std::array<float,3> kVoiceSemitones{7.f,-7.f,3.f};
constexpr std::array<float,3> kVoiceStart{.23f,.61f,.11f};
constexpr std::array<float,3> kVoiceLow{650.f,850.f,480.f};
constexpr std::array<float,3> kVoiceHigh{3000.f,1800.f,2200.f};
}

void Schizophrenia::prepare(double sr) {
    rate_=saneRate(sr);
    step_=coeff(18.,rate_);
    for(auto& s:state_)s.ring.assign(
        std::max<std::size_t>(64,static_cast<std::size_t>(std::ceil(.070*rate_))+4),0.f);
    reset();
}
void Schizophrenia::reset() noexcept {
    for(unsigned ch=0;ch<state_.size();++ch) {
        auto& s=state_[ch];
        std::fill(s.ring.begin(),s.ring.end(),0.f);
        s.write=0;s.voices={};
        for(std::size_t k=0;k<s.voices.size();++k) {
            const float phase=kVoiceStart[k]+(ch?.17f:0.f);
            s.voices[k].phase=phase>=1.f?phase-1.f:phase;
        }
    }
    smoothed_=amount_;
}
void Schizophrenia::setAmount(float v) noexcept {amount_=norm(v);}
void Schizophrenia::process(float* const* io,unsigned channels,std::size_t frames) noexcept {
    if(!valid(io,channels)||state_[0].ring.empty())return;
    std::array<float,3> grain{},phaseStep{},lowPole{},highPole{};
    for(std::size_t k=0;k<grain.size();++k) {
        grain[k]=std::max(4.f,static_cast<float>(rate_*kVoiceSeconds[k]));
        phaseStep[k]=1.f/grain[k];
        lowPole[k]=coeff(kVoiceLow[k],rate_);
        highPole[k]=coeff(kVoiceHigh[k],rate_);
    }
    const float baseLag=std::max(1.f,static_cast<float>(rate_*.002));
    for(std::size_t i=0;i<frames;++i) {
        smoothed_=detail::smoothToward(smoothed_,amount_,step_);
        const float m=smoothed_;
        std::array<float,3> ratio{};
        for(std::size_t k=0;k<ratio.size();++k)
            ratio[k]=std::exp2(kVoiceSemitones[k]*m/12.f);
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=io[ch][i],x=sane(raw);
            s.ring[s.write]=x;
            std::array<float,3> voice{};
            for(std::size_t k=0;k<voice.size();++k) {
                auto& v=s.voices[k];
                const float p=v.phase,q=p<.5f?p+.5f:p-.5f;
                const float excursion=std::abs(ratio[k]-1.f)*grain[k];
                const auto read=[&](float phase) noexcept {
                    const float lag=baseLag+excursion*(ratio[k]>=1.f?1.f-phase:phase);
                    return linearRead(s.ring,s.write,lag);
                };
                const float w=.5f-.5f*std::cos(kTau*p);
                const float sample=w*read(p)+(1.f-w)*read(q);
                v.phase+=phaseStep[k];if(v.phase>=1.f)v.phase-=1.f;
                v.low+=lowPole[k]*(sample-v.low);
                v.high+=highPole[k]*(sample-v.high);
                // Different spectral envelopes, not three identical copies.
                voice[k]=k==0?.65f*sample+.35f*(v.high-v.low):
                         k==1?.45f*sample+.55f*v.low:
                              .45f*sample+.55f*(v.high-v.low);
            }
            const float high=voice[0],low=voice[1],middle=voice[2];
            // Stereo voices pull in opposing directions; mono retains
            // all three. Limited pairwise product makes the timbres collide.
            const float side=channels==1?0.f:(ch==0?1.f:-1.f);
            const float conflict=.16f*std::tanh(3.f*high*low)+
                                 .08f*std::tanh(3.f*low*middle);
            const float transformed=std::tanh(.33f*x+
                (.52f+.13f*side*m)*high+(.56f-.14f*side*m)*low+
                (.36f+.06f*side*m)*middle+m*conflict);
            // Immediate transformed body retains the proven full-wet
            // startup contract, without a dry bypass during ring fill.
            const float wet=transformed*(1.f+.10f*m*m);
            if(m>0.f)io[ch][i]=m==1.f?wet:x+m*(wet-x);
            if(++s.write>=s.ring.size())s.write=0;
        }
    }
}

void Dissociation::prepare(double sr) {
    rate_=saneRate(sr);
    step_=coeff(14.,rate_);
    for(auto& s:state_) {
        const auto size=std::max<std::size_t>(64,static_cast<std::size_t>(rate_*0.112));
        s.frozen.assign(size,0.f);s.recording.assign(size,0.f);
    }
    reset();
}
void Dissociation::reset() noexcept {
    for(auto& s:state_) {
        std::fill(s.frozen.begin(),s.frozen.end(),0.f);
        std::fill(s.recording.begin(),s.recording.end(),0.f);
        s.capture=s.play=s.untilCapture=0;
        s.blur=s.low=s.high=0.f;
        s.ready=false;s.recordingReady=false;
    }
    smoothed_=amount_;
}
void Dissociation::setAmount(float v) noexcept {amount_=norm(v);}
void Dissociation::process(float* const* io,unsigned channels,std::size_t frames) noexcept {
    if(!valid(io,channels)||state_[0].frozen.empty())return;
    const float a=coeff(280.,rate_),b=coeff(1650.,rate_);
    for(std::size_t i=0;i<frames;++i) {
        smoothed_=detail::smoothToward(smoothed_,amount_,step_);
        const float m=smoothed_;
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=io[ch][i],x=sane(raw);
            const auto size=s.frozen.size();
            // Capture into separate preallocated storage. Publish only at
            // a replay boundary. vector::swap is constant-time.
            if(s.recordingReady && (!s.ready || s.play==0)) {
                s.frozen.swap(s.recording);
                s.recordingReady=false;s.ready=true;s.capture=0;s.play=0;
                s.untilCapture=static_cast<std::size_t>(
                    rate_*(0.35-0.20*m)) + (ch?size/5:0);
            }
            if(!s.recordingReady) {
                if(s.untilCapture==0 || !s.ready) {
                    s.recording[s.capture++]=x;
                    if(s.capture==size)s.recordingReady=true;
                } else --s.untilCapture;
            }
            float backwards=0.f;
            float window=0.f;
            if(s.ready) {
                // Play backwards, but fade 2ms at grain boundaries.
                const auto idx=size-1-s.play;
                backwards=s.frozen[idx];
                const float ramp=std::max(1.f,static_cast<float>(rate_*0.003));
                window=std::min({1.f,static_cast<float>(s.play)/ramp,
                    static_cast<float>(size-s.play)/ramp});
                if(++s.play>=size)s.play=0;
            }
            s.low+=a*(backwards-s.low);
            s.high+=b*(backwards-s.high);
            const float spectralBlur=0.58f*s.low+0.32f*(s.high-s.low);
            s.blur+=(0.012f+0.014f*(1.f-m))*(spectralBlur-s.blur);
            const float displaced=0.48f*window*backwards+
                0.50f*s.blur+0.16f*(s.high-s.low);
            // P011: recovered/reversed grains are attenuated by windowing,
            // smoothing, and destructive source-correlation. Restore useful
            // full-wet output energy WITHOUT reintroducing the dry input:
            // makeup multiplies only the transformed time-displaced branch.
            // Unity at 0%; smooth +~5.1 dB at 100%. Not a limiter/AGC.
            const float makeup=1.f+0.80f*m*m;
            if(m>0.f) io[ch][i]=m==1.f?makeup*displaced:x+m*(makeup*displaced-x);
        }
    }
}

void Hallucination::prepare(double sr) {
    rate_=saneRate(sr);
    step_=coeff(15.,rate_);
    for(std::size_t ch=0;ch<2;++ch)
        state_[ch].echo.assign(std::max<std::size_t>(64,
            static_cast<std::size_t>(rate_*(ch?0.139:0.113))),0.f);
    reset();
}
void Hallucination::reset() noexcept {
    for(auto& s:state_) {
        s.analytic.fill(0.f);
        std::fill(s.echo.begin(),s.echo.end(),0.f);
        s.analyticWrite=s.echoWrite=0;
        s.phase=s.formantLow=s.formantHigh=0.f;
    }
    smoothed_=amount_;
}
void Hallucination::setAmount(float v) noexcept {amount_=norm(v);}
void Hallucination::process(float* const* io,unsigned channels,std::size_t frames) noexcept {
    if(!valid(io,channels)||state_[0].echo.empty())return;
    const float a=coeff(550.,rate_),b=coeff(1600.,rate_);
    const float invRate=static_cast<float>(1./rate_);
    for(std::size_t i=0;i<frames;++i) {
        smoothed_=detail::smoothToward(smoothed_,amount_,step_);
        const float m=smoothed_;
        for(unsigned ch=0;ch<channels;++ch) {
            auto& s=state_[ch];
            const float raw=io[ch][i],x=sane(raw);
            const auto write=s.analyticWrite;
            s.analytic[write]=x;
            // P006: compute the eight fixed Hilbert taps once at compile time,
            // rather than 8 cos() calls PER SAMPLE and per channel on the
            // realtime audio thread. Same Hamming-windowed 17-tap FIR response.
            // Reference: 2 / (pi*k) * (0.54 + 0.46*cos(pi*k/8)).
            constexpr std::array<int,8> offsets{-7,-5,-3,-1,1,3,5,7};
            constexpr std::array<float,8> taps{
                -0.010460155335f,-0.046341542172f,
                -0.151947214432f,-0.614328266812f,
                 0.614328266812f, 0.151947214432f,
                 0.046341542172f, 0.010460155335f
            };
            const auto center=(write+17-8)%17;
            const float real=s.analytic[center];
            float quadrature=0.f;
            for(std::size_t tap=0;tap<taps.size();++tap) {
                const int k=offsets[tap];
                const auto index=(center+17-static_cast<std::size_t>(k+17)%17)%17;
                quadrature+=taps[tap]*s.analytic[index];
            }
            const float shifted=real*std::cos(kTau*s.phase)-
                                quadrature*std::sin(kTau*s.phase);
            s.phase+=invRate*(3.f+91.f*m+(ch?1.13f:0.f));
            if(s.phase>=1.f)s.phase-=1.f;
            if(++s.analyticWrite>=17)s.analyticWrite=0;
            s.formantLow+=a*(shifted-s.formantLow);
            s.formantHigh+=b*(shifted-s.formantHigh);
            const float otherVoice=s.formantHigh-s.formantLow;
            const float echo=s.echo[s.echoWrite];
            s.echo[s.echoWrite]=std::tanh(0.28f*x+0.62f*echo+
                                       0.26f*otherVoice);
            if(++s.echoWrite>=s.echo.size())s.echoWrite=0;
            const float transformed=std::tanh(0.57f*shifted+
                (0.24f+0.20f*m)*otherVoice+0.28f*echo);
            // P011: analytic phase/echo paths lost ~5-7.5 dB on P010
            // source proxies. Apply gradual makeup ONLY to the effected
            // signal. At 100%, the output remains pure wet (no dry blend).
            const float makeup=1.f+0.65f*m*m;
            if(m>0.f)io[ch][i]=x+m*(makeup*transformed-x);
        }
    }
}
} // namespace psychiatry
