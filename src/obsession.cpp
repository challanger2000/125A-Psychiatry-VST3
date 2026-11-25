#include "obsession.hpp"
#include "parameter_smoothing.hpp"
#include <algorithm>
#include <cmath>
namespace psychiatry {
namespace {
float norm(float x) noexcept {return std::isfinite(x)?std::clamp(x,0.f,1.f):0.f;}
float safe(float x) noexcept {return std::isfinite(x)?std::clamp(x,-4.f,4.f):0.f;}
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
    retriggers_=0;smoothed_=target_;colour_={};
}
void Obsession::setAmount(float amount) noexcept {target_=norm(amount);}
void Obsession::process(float* const* channels,unsigned count,std::size_t frames,const float* curve) noexcept {
    if(history_.empty()||!channels||!channels[0]||(count!=1&&count!=2)||(count==2&&!channels[1]))return;
    const auto cap=history_.size();
    for(std::size_t n=0;n<frames;++n) {
        if(curve)target_=curve[n];
        smoothed_=detail::smoothToward(smoothed_,target_,step_);
        const float m=smoothed_;
        const float rawL=channels[0][n],rawR=count==2?channels[1][n]:rawL;
        history_[cursor_]={safe(rawL),safe(rawR)};
        if(filled_<cap)++filled_;
        if(++cursor_==cap)cursor_=0;
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
                start_=cursor_>=loopLength_?cursor_-loopLength_:cursor_+cap-loopLength_;
                phase_=0;repeats_=0;captured_=true;colour_={};
            }
        }
        if(m>0.f) {
            const float wet=m*(0.48f+0.52f*m);
            float loopL=0.f,loopR=0.f;
            if(captured_) {
                const auto read=start_+phase_;
                const Sample sample=history_[read>=cap?read-cap:read];
                const float fadeFrames=std::max(1.f,std::min<float>(
                    static_cast<float>(rate_*.002),static_cast<float>(loopLength_)*.18f));
                const float fade=std::min({1.f,static_cast<float>(phase_)/fadeFrames,
                    static_cast<float>(loopLength_-phase_-1)/fadeFrames});
                // Repetition-dependent analogue-style coloration of the
                // immutable grain. No bit reduction, quantization or rate
                // reduction. The same source is recognizable as its upper
                // spectrum narrows and the drive progressively increases.
                const float progress=static_cast<float>(repeats_)/
                    static_cast<float>(std::max(1u,repeatsPerCapture_));
                const float wear=m*(.15f+.85f*progress);
                const float cutoff=6200.f*std::pow(.115f,wear);
                const float alpha=static_cast<float>(1.-std::exp(-6.283185307179586*cutoff/rate_));
                colour_.l+=alpha*(sample.l-colour_.l);
                colour_.r+=alpha*(sample.r-colour_.r);
                const float blend=.80f*wear;
                const float drive=1.f+.50f*m+.60f*wear;
                // P026f: retain the nonlinear transfer and spectral wear, but
                // trim wet-only replay gain at high settings. The fixed
                // 0.82 factor at full amount is -1.72 dB relative to P026e.
                const float gain=(1.f-.06f*m*m)*(1.f-.18f*m*m)*fade;
                const auto shape=[&](float x,float filtered) noexcept {
                    const float coloured=x+blend*(filtered-x);
                    return gain*std::tanh(drive*coloured);
                };
                loopL=shape(sample.l,colour_.l);
                loopR=shape(sample.r,colour_.r);
            }
            channels[0][n]=wet==1.f?loopL:safe(rawL)+wet*(loopL-safe(rawL));
            if(count==2)channels[1][n]=wet==1.f?loopR:safe(rawR)+wet*(loopR-safe(rawR));
        }
        if(captured_)++phase_;
    }
}
} // namespace psychiatry
