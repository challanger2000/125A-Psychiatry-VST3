#include "obsession.hpp"
#include "parameter_smoothing.hpp"
#include <algorithm>
#include <cmath>
namespace psychiatry {
namespace {
float norm(float x) noexcept {return std::isfinite(x)?std::clamp(x,0.f,1.f):0.f;}
float safe(float x) noexcept {return std::isfinite(x)?std::clamp(x,-4.f,4.f):0.f;}
float quantize(float v,float steps) noexcept {return std::round(v*steps)/steps;}
}
void Obsession::prepare(double sampleRate) {
    rate_=std::isfinite(sampleRate)&&sampleRate>=1000.&&sampleRate<=2000000.?sampleRate:48000.;
    // Hold captured audio for the whole repeat group. Its duration is <1 s
    // at a fixed macro position; additionally cap repeats against capacity.
    history_.assign(std::max<std::size_t>(512,static_cast<std::size_t>(rate_*1.5)),{});
    step_=static_cast<float>(1.-std::exp(-1./(0.012*rate_)));
    reset();
}
void Obsession::reset() noexcept {
    std::fill(history_.begin(),history_.end(),Sample{});
    cursor_=filled_=start_=phase_=0;
    loopLength_=1;repeats_=0;repeatsPerCapture_=2;captured_=false;
    retriggers_=0;smoothed_=target_;
}
void Obsession::setAmount(float amount) noexcept {target_=norm(amount);}
void Obsession::process(float* const* channels,unsigned count,std::size_t frames) noexcept {
    if(history_.empty()||!channels||!channels[0]||(count!=1&&count!=2)||(count==2&&!channels[1]))return;
    const auto cap=history_.size();
    for(std::size_t n=0;n<frames;++n) {
        smoothed_=detail::smoothToward(smoothed_,target_,step_);
        const float m=smoothed_;
        const float rawL=channels[0][n],rawR=count==2?channels[1][n]:rawL;
        history_[cursor_]={safe(rawL),safe(rawR)};
        if(filled_<cap)++filled_;
        cursor_=(cursor_+1)%cap;
        if(captured_&&phase_>=loopLength_) {phase_=0;++repeats_;++retriggers_;}
        if(!captured_||(phase_==0&&repeats_>=repeatsPerCapture_)) {
            const auto targetLen=std::clamp<std::size_t>(
                static_cast<std::size_t>(rate_*(0.275-0.257*std::sqrt(m))),32,cap-1);
            if(filled_>=targetLen) {
                loopLength_=targetLen;
                repeatsPerCapture_=static_cast<unsigned>(std::min<std::size_t>(
                    static_cast<unsigned>(2.f+11.f*m),(cap-loopLength_-1)/loopLength_));
                // Capture a complete preceding grain. Freeze length and repeat
                // count together so later automation cannot invalidate history.
                start_=(cursor_+cap-loopLength_)%cap;
                phase_=0;repeats_=0;captured_=true;
            }
        }
        if(m>0.f) {
            const float wet=m*(0.48f+0.52f*m);
            float loopL=0.f,loopR=0.f;
            if(captured_) {
                const Sample sample=history_[(start_+phase_)%cap];
                const float fadeFrames=std::max(1.f,std::min<float>(
                    static_cast<float>(rate_*.002),static_cast<float>(loopLength_)*.18f));
                const float fade=std::min({1.f,static_cast<float>(phase_)/fadeFrames,
                    static_cast<float>(loopLength_-phase_-1)/fadeFrames});
                const float bits=std::max(6.f,16.f-10.f*m-1.5f*static_cast<float>(repeats_));
                const float quantSteps=std::exp2(bits),degrade=.20f+.70f*m;
                // Fade the LOOP, not the wet mix. Full-wet edges cannot leak
                // the live input. 0.54 dB wet-only trim preserves headroom.
                const float gain=(1.f-.06f*m*m)*fade;
                loopL=gain*((1.f-degrade)*sample.l+degrade*std::tanh(1.5f*quantize(sample.l,quantSteps)));
                loopR=gain*((1.f-degrade)*sample.r+degrade*std::tanh(1.5f*quantize(sample.r,quantSteps)));
            }
            channels[0][n]=wet==1.f?loopL:safe(rawL)+wet*(loopL-safe(rawL));
            if(count==2)channels[1][n]=wet==1.f?loopR:safe(rawR)+wet*(loopR-safe(rawR));
        }
        if(captured_)++phase_;
    }
}
} // namespace psychiatry
