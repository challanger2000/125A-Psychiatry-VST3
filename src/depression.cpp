#include "depression.hpp"
#include "parameter_smoothing.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace psychiatry {
namespace {
float normal(float x) noexcept {
    return std::isfinite(x) ? std::clamp(x,0.f,1.f) : 0.f;
}
float finite(float x) noexcept {
    return std::isfinite(x) ? std::clamp(x,-4.f,4.f) : 0.f;
}
}
void Depression::prepare(double sampleRate) {
    rate_=std::isfinite(sampleRate) && sampleRate>=1000. && sampleRate<=2000000.
        ? sampleRate : 48000.;
    // Four non-commensurate delay times, offset per stereo channel.
    constexpr std::array<double,4> times{0.047,0.061,0.073,0.089};
    for(std::size_t ch=0;ch<2;++ch)
        for(std::size_t i=0;i<kDelayCount;++i)
            state_[ch].delay[i].assign(
                std::max<std::size_t>(2,static_cast<std::size_t>(
                    rate_*(times[i]+(ch?0.0037:0.0)))),0.f);
    step_=static_cast<float>(1.-std::exp(-1./(0.012*rate_)));
    for(auto& channel:state_) {
        channel.saturation.prepare();
        channel.drift.assign(static_cast<std::size_t>(std::ceil(rate_*.045))+4,0.f);
    }
    reset();
}
void Depression::reset() noexcept {
    for(auto& c:state_) {
        c.cursor.fill(0);
        c.damping.fill(0);
        c.darkness=0.f;
        c.dynamics=c.fastEnvelope=c.slowEnvelope=c.presence=0.f;
        c.driftPhase=0.f;c.driftWrite=0;
        std::fill(c.drift.begin(),c.drift.end(),0.f);
        c.gainDelay.reset(1.f);
        c.saturation.reset();
        c.dryDelay.reset();
        for(auto& line:c.delay) std::fill(line.begin(),line.end(),0.f);
    }
    smoothed_=target_;
    mixDelay_.reset(smoothed_/(0.50f+0.50f*smoothed_));
}
void Depression::setAmount(float a) noexcept { target_=normal(a); }

void Depression::process(float* const* channels, unsigned count, std::size_t frames,const float* curve) noexcept {
    if(!channels||!channels[0]||(count!=1&&count!=2)||
        (count==2&&!channels[1])||state_[0].delay[0].empty()) return;
    const auto pole=[this](double seconds) noexcept {
        return static_cast<float>(1.-std::exp(-1./(seconds*rate_)));
    };
    const float attack=pole(.0018),release=pole(.028),slowAttack=pole(.10),slowRelease=pole(.30);
    const float presencePole=static_cast<float>(1.-std::exp(-6.283185307179586*3200./rate_));
    const float driftStep=static_cast<float>(1./(.42*rate_));
    for(std::size_t n=0;n<frames;++n) {
        if(curve)target_=curve[n];
        smoothed_=detail::smoothToward(smoothed_,target_,step_);
        // P006 measured 20%-macro effect below the onset threshold on
        // clean bass/atmosphere. A smooth concave curve makes the first
        // third musically perceptible without shifting the 0% or 100% ends.
        const float m=smoothed_/(0.50f+0.50f*smoothed_);
        const float mix=mixDelay_.process(m);
        const float drive=1.f+4.5f*m*m;
        const float cutoff=7500.f*std::pow(0.062f,m);
        const float a=1.f-std::exp(-6.28318530718f*cutoff/static_cast<float>(rate_));
        for(unsigned ch=0;ch<count;++ch) {
            auto& c=state_[ch];
            const float original=channels[ch][n];
            const float x=finite(original);
            const float delayedRaw=c.dryDelay.process(original);
            const float delayed=finite(delayedRaw);
            // Attenuate onsets before adding any room response. Compare
            // a fast peak follower to a slower body follower; both are
            // continuous and release between phrases.
            const float level=std::abs(x);
            c.fastEnvelope+=(level>c.fastEnvelope?attack:release)*(level-c.fastEnvelope);
            c.slowEnvelope+=(level>c.slowEnvelope?slowAttack:slowRelease)*(level-c.slowEnvelope);
            const float excess=std::clamp((c.fastEnvelope-c.slowEnvelope)/
                (.065f+c.slowEnvelope),0.f,1.f);
            const float transientGain=1.f-.54f*m*excess;
            const float levelGain=1.f/(1.f+.95f*m*c.slowEnvelope);
            const float bodyGain=transientGain*levelGain;
            const float delayedGain=c.gainDelay.process(bodyGain);
            c.dynamics+=step_*4.f*(level-c.dynamics);
            // P013: 2x filtered wet-only tanh saturation. Original high
            // source frequencies generated aliases before the lowpass;
            // first create the harmonics at 2x, then decimate safely.
            const float compressed=c.saturation.process(x*bodyGain,[&](float up) noexcept {
                return std::tanh(drive*up);
            })/(1.f+0.3f*m*c.dynamics*drive);
            // A dynamically narrowing band, not a passive EQ preset.
            c.darkness+=a*(compressed-c.darkness);
            // The presence path is also processed (attenuated transients,
            // low-pass shelf), not an untouched parallel dry overlay.
            c.presence+=presencePole*(delayed*delayedGain-c.presence);
            const float presence=(1.f-.52f*m)*delayed*delayedGain+.52f*m*c.presence;
            // Two past-only interpolated readheads, half a cycle apart.
            // Each head moves away from the writer, creating a small
            // downward pitch drift; triangular crossfade hides resets.
            const auto cap=c.drift.size();
            c.drift[c.driftWrite]=c.darkness;
            const float p=c.driftPhase,q=p<.5f?p+.5f:p-.5f;
            const auto read=[&](float phase) noexcept {
                const float lag=static_cast<float>(rate_)*(.001f+.015f*m*phase);
                const auto whole=static_cast<std::size_t>(lag);
                const auto i=c.driftWrite>=whole?c.driftWrite-whole:c.driftWrite+cap-whole;
                const auto j=i?i-1:cap-1;
                return c.drift[i]+(lag-static_cast<float>(whole))*(c.drift[j]-c.drift[i]);
            };
            const float weight=1.f-std::abs(2.f*p-1.f);
            const float drifting=weight*read(p)+(1.f-weight)*read(q);
            c.driftPhase+=driftStep;if(c.driftPhase>=1.f)c.driftPhase-=1.f;
            if(++c.driftWrite==cap)c.driftWrite=0;
            const float dark=c.darkness+.12f*m*(drifting-c.darkness);
            const float body=.32f*presence+.68f*dark;
            // Plate-like diffuse tail: feedback in an energy-preserving
            // orthogonal Hadamard scattering matrix; <1 feedback gain.
            std::array<float,4> out{};
            for(std::size_t i=0;i<kDelayCount;++i) {
                const auto p=c.cursor[i];
                const float delayed=c.delay[i][p];
                // Feedback damping grows with macro intensity.
                c.damping[i]+=(0.18f+0.22f*(1.f-m))*(delayed-c.damping[i]);
                out[i]=c.damping[i];
            }
            const std::array<float,4> mixed{
                0.5f*(out[0]+out[1]+out[2]+out[3]),
                0.5f*(out[0]-out[1]+out[2]-out[3]),
                0.5f*(out[0]+out[1]-out[2]-out[3]),
                0.5f*(out[0]-out[1]-out[2]+out[3])
            };
            const float send=(m<=0.f?0.f:(0.10f+0.20f*m))*body;
            // Maximum small-signal feedback is 0.84*1.10 = 0.924 (<1).
            // This is intentionally non-self-oscillating even at full macro.
            const float feedback=0.60f+0.24f*m;
            for(std::size_t i=0;i<kDelayCount;++i) {
                // Saturation in the *return* path creates a progressively
                // crumbling hall, rather than an ordinary clean plate.
                const float put=std::tanh(send + feedback*mixed[i]*(1.f+0.10f*m));
                c.delay[i][c.cursor[i]]=put;
                if(++c.cursor[i]>=c.delay[i].size()) c.cursor[i]=0;
            }
            const float tail=0.25f*(out[0]+out[1]+out[2]+out[3]);
            const float reverbMix=m*m*(0.12f+0.30f*m);
            // Controlled musical decay and level, not louder = better.
            const float transformed=body*(1.f-.18f*m)+tail*reverbMix;
            channels[ch][n]=mix==0.f?delayedRaw:(mix==1.f?transformed:delayed+mix*(transformed-delayed));
        }
    }
}
} // namespace psychiatry
