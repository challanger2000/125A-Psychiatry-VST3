#include "reactive_acoustic.hpp"
#include "acoustic_bank.hpp"
#include <algorithm>
#include <cmath>

namespace psychiatry {
namespace {
constexpr std::size_t count=sizeof(acoustic_bank::paranoia)/sizeof(acoustic_bank::paranoia[0]);
static_assert(count==6,"PARANOIA acoustic bank manifest");
float sample(const acoustic_bank::Clip& c,std::size_t i) noexcept {
    return static_cast<float>(c.pcm[i])*(1.f/32768.f);
}
}
void ReactiveAcoustic::prepare(double sr) noexcept {
    rate_=std::isfinite(sr)&&sr>=1000.&&sr<=2000000.?sr:48000.;
    reset();
}
void ReactiveAcoustic::reset() noexcept {
    rng_=0x125A6B71u;events_=0;playing_=false;position_=0.;
    begin_=end_=0;clip_=0;increment_=1.;pan_=0.f;cooldown_=0;
}
float ReactiveAcoustic::random() noexcept {
    rng_^=rng_<<13;rng_^=rng_>>17;rng_^=rng_<<5;
    return static_cast<float>(rng_>>8)*(1.f/16777216.f);
}
void ReactiveAcoustic::begin() noexcept {
    // Whispers receive half of the selection probability; no consecutive
    // reuse of the same source when another source is available.
    const float select=random();
    auto chosen=select<.5f?static_cast<std::size_t>(random()*2.f):
        2+static_cast<std::size_t>(random()*static_cast<float>(count-2));
    if(events_ && chosen==clip_)chosen=(chosen+1)%count;
    clip_=chosen;
    const auto& c=acoustic_bank::paranoia[clip_];
    const auto desired=static_cast<std::size_t>((.48+.95*random())*acoustic_bank::sampleRate);
    const auto length=std::min(c.frames,desired);
    begin_=static_cast<std::size_t>(random()*static_cast<double>(c.frames-length));
    end_=begin_+length;position_=static_cast<double>(begin_);
    increment_=(.82+.23*random())*acoustic_bank::sampleRate/rate_;
    pan_=(2.f*random()-1.f)*.85f;
    playing_=true;++events_;
}
ReactiveAcoustic::Stereo ReactiveAcoustic::tick(float envelope,float amount) noexcept {
    // 24 kHz assets have a 12 kHz band limit; no playback below 32 kHz,
    // where the randomized resampling ratio might exceed Nyquist.
    if(rate_<32000. || amount<=0.f) {
        playing_=false;cooldown_=0;return {};
    }
    if(cooldown_)--cooldown_;
    if(!playing_ && !cooldown_ && envelope>.004f && amount>.12f)begin();
    if(!playing_)return {};
    const auto& c=acoustic_bank::paranoia[clip_];
    const auto i=static_cast<std::size_t>(position_);
    const auto next=std::min(i+1,end_-1);
    const float frac=static_cast<float>(position_-static_cast<double>(i));
    const float x=sample(c,i)+frac*(sample(c,next)-sample(c,i));
    const float fade=std::min({1.f,static_cast<float>((position_-begin_)/360.),
                                    static_cast<float>((end_-position_)/360.)});
    // The output remains proportional to the input envelope, including
    // during the end of a recorded fragment. No free-running ambience.
    const float v=x*fade*amount*std::min(.32f,1.7f*std::max(0.f,envelope));
    position_+=increment_;
    if(position_>=static_cast<double>(end_)) {
        playing_=false;
        cooldown_=static_cast<unsigned>(rate_*(.38+(1.30-.75*amount)*random()));
    }
    // Constant-power pan is unnecessary here: sum stays bounded and mono
    // downmix is independent of the per-event pan position.
    return {v*(1.f-.65f*pan_),v*(1.f+.65f*pan_)};
}
}
