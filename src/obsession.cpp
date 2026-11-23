#include "obsession.hpp"
#include "parameter_smoothing.hpp"
#include <algorithm>
#include <cmath>

namespace psychiatry {
namespace {
float norm(float x) noexcept {
    return std::isfinite(x)?std::clamp(x,0.f,1.f):0.f;
}
float safe(float x) noexcept {
    return std::isfinite(x)?std::clamp(x,-4.f,4.f):0.f;
}
float quantize(float v,float steps) noexcept {
    return std::round(v*steps)/steps;
}
}
void Obsession::prepare(double sampleRate) {
    rate_=std::isfinite(sampleRate)&&sampleRate>=1000.&&sampleRate<=2000000.
         ?sampleRate:48000.;
    // Fixed-size past-audio memory independent of host block size.
    const auto length=std::max<std::size_t>(512,static_cast<std::size_t>(rate_*0.7));
    history_.assign(length,{});
    step_=static_cast<float>(1.-std::exp(-1./(0.012*rate_)));
    reset();
}
void Obsession::reset() noexcept {
    std::fill(history_.begin(),history_.end(),Sample{});
    cursor_=filled_=start_=phase_=0;
    loopLength_=std::max<std::size_t>(1,static_cast<std::size_t>(0.25*rate_));
    retriggers_=0;
    repeats_=0;
    smoothed_=target_;
    reducer_=0.f;
}
void Obsession::setAmount(float amount) noexcept {target_=norm(amount);}
void Obsession::process(float* const* channels,unsigned count,std::size_t frames) noexcept {
    if(history_.empty()||!channels||!channels[0]||(count!=1&&count!=2)||
       (count==2&&!channels[1])) return;
    const auto cap=history_.size();
    for(std::size_t n=0;n<frames;++n) {
        smoothed_=detail::smoothToward(smoothed_,target_,step_);
        const float m=smoothed_;
        const float rawL=channels[0][n], rawR=count==2?channels[1][n]:rawL;
        history_[cursor_]={safe(rawL),safe(rawR)};
        if(filled_<cap)++filled_;
        cursor_=(cursor_+1)%cap;

        // Loop sizes collapse from 275ms to 18ms. Choose only at wrap,
        // otherwise adjusting a parameter could leap to an invalid cursor.
        if(phase_>=loopLength_) {
            phase_=0;
            ++repeats_;
            ++retriggers_;
        }
        const auto repeatsPerCapture=static_cast<unsigned>(2.f+11.f*m);
        if(phase_==0 && (repeats_>=repeatsPerCapture || retriggers_==0)) {
            const auto targetLen=static_cast<std::size_t>(
                rate_*(0.275-0.257*std::sqrt(m)));
            loopLength_=std::clamp<std::size_t>(targetLen,32,cap-1);
            // Includes no future data: capture previous loop worth of audio.
            start_=(cursor_+cap-loopLength_-1)%cap;
            repeats_=0;
        }
        // Always keep history running even during zero macro.
        if(m>0.f && filled_>loopLength_+1) {
            const auto idx=(start_+phase_)%cap;
            const Sample sample=history_[idx];
            const float fadeFrames=std::min<float>(96.f,static_cast<float>(loopLength_)*0.18f);
            const float fade=std::min({1.f,
                static_cast<float>(phase_)/std::max(1.f,fadeFrames),
                static_cast<float>(loopLength_-phase_)/std::max(1.f,fadeFrames)});
            const float wet=m*(0.48f+0.52f*m)*std::max(0.f,fade);
            // Dirty decaying digital echo within the stutter; only in wet path.
            const float bits=std::max(6.f,16.f-10.f*m-1.5f*static_cast<float>(repeats_));
            const float quantSteps=std::exp2(bits);
            const float degrade=0.20f+0.70f*m;
            const float loopL=(1.f-degrade)*sample.l+
                degrade*std::tanh(1.5f*quantize(sample.l,quantSteps));
            const float loopR=(1.f-degrade)*sample.r+
                degrade*std::tanh(1.5f*quantize(sample.r,quantSteps));
            channels[0][n]=safe(rawL)+wet*(loopL-safe(rawL));
            if(count==2) channels[1][n]=safe(rawR)+wet*(loopR-safe(rawR));
        }
        ++phase_;
    }
}
} // namespace psychiatry
