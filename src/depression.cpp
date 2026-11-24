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
    for(auto& channel:state_)channel.saturation.prepare();
    reset();
}
void Depression::reset() noexcept {
    for(auto& c:state_) {
        c.cursor.fill(0);
        c.damping.fill(0);
        c.darkness=0.f;
        c.dynamics=0.f;
        c.saturation.reset();
        c.dryDelay.reset();
        for(auto& line:c.delay) std::fill(line.begin(),line.end(),0.f);
    }
    smoothed_=target_;
    mixDelay_.reset(smoothed_/(0.50f+0.50f*smoothed_));
}
void Depression::setAmount(float a) noexcept { target_=normal(a); }

void Depression::process(float* const* channels, unsigned count, std::size_t frames) noexcept {
    if(!channels||!channels[0]||(count!=1&&count!=2)||
        (count==2&&!channels[1])||state_[0].delay[0].empty()) return;
    for(std::size_t n=0;n<frames;++n) {
        smoothed_=detail::smoothToward(smoothed_,target_,step_);
        // P006 measured 20%-macro effect below the onset threshold on
        // clean bass/atmosphere. A smooth concave curve makes the first
        // third musically perceptible without shifting the 0% or 100% ends.
        const float m=smoothed_/(0.50f+0.50f*smoothed_);
        const float mix=mixDelay_.process(m);
        for(unsigned ch=0;ch<count;++ch) {
            auto& c=state_[ch];
            const float original=channels[ch][n];
            const float x=finite(original);
            const float delayedRaw=c.dryDelay.process(original);
            const float delayed=finite(delayedRaw);
            // Program-dependent compression with saturator; gradual onset.
            c.dynamics+=step_*4.f*(std::abs(x)-c.dynamics);
            const float drive=1.f+4.5f*m*m;
            // P013: 2x filtered wet-only tanh saturation. Original high
            // source frequencies generated aliases before the lowpass;
            // first create the harmonics at 2x, then decimate safely.
            const float compressed=c.saturation.process(x,[&](float up) noexcept {
                return std::tanh(drive*up);
            })/(1.f+0.3f*m*c.dynamics*drive);
            // A dynamically narrowing band, not a passive EQ preset.
            const float cutoff=7500.f*std::pow(0.062f,m);
            const float a=1.f-std::exp(-6.28318530718f*cutoff/static_cast<float>(rate_));
            c.darkness+=a*(compressed-c.darkness);
            const float body=0.36f*delayed+0.64f*c.darkness;
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
            const float send=(m<=0.f?0.f:(0.12f+0.28f*m))*body;
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
            const float reverbMix=m*m*(0.17f+0.43f*m);
            // Controlled musical decay and level, not louder = better.
            const float transformed=body*(1.f-0.3f*m)+tail*reverbMix;
            channels[ch][n]=mix==0.f?delayedRaw:(mix==1.f?transformed:delayed+mix*(transformed-delayed));
        }
    }
}
} // namespace psychiatry
