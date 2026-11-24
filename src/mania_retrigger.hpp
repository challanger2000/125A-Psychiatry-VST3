#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace psychiatry::detail {
// Two repeats of one recent, band-limited fragment; common stereo clock.
class ManiaRetrigger final {
public:
    void prepare(double sr) {
        rate_=sr;
        for(auto& c:channels_) c.history.assign(static_cast<std::size_t>(std::ceil(sr*.09))+4,0.f);
        const auto pole=[sr](double hz){return static_cast<float>(1.-std::exp(-6.283185307179586*hz/sr));};
        fastPole_=pole(92.);slowPole_=pole(4.);
        lowPole_=pole(std::min(1800.,.20*sr));
        gLo_=static_cast<float>(std::tan(3.141592653589793*std::min(330.,.12*sr)/sr));
        gHi_=static_cast<float>(std::tan(3.141592653589793*std::min(2800.,.35*sr)/sr));
        reset();
    }
    bool ready() const noexcept {return !channels_[0].history.empty();}
    void reset() noexcept {
        for(auto& c:channels_) {std::fill(c.history.begin(),c.history.end(),0.f);c.low1=c.low2=c.ic1=c.ic2=0.f;}
        phase_=0.;fast_=slow_=0.f;write_=start_=grain_=age_=0;events_=0;
    }
    std::uint64_t events() const noexcept {return events_;}
    float tick(float level,float m) noexcept {
        fast_+=fastPole_*(level-fast_);slow_+=slowPole_*(level-slow_);
        const float urgency=std::clamp(4.f*(fast_-slow_),0.f,1.f);
        const float hz=2.6f+21.4f*m*m+6.f*m*(1.f-m)*urgency;
        phase_+=hz/rate_;
        if(phase_>=1.) {
            phase_-=1.;
            if(m>.01f && fast_>.001f && age_>=2*grain_) {
                grain_=std::max<std::size_t>(4,static_cast<std::size_t>(rate_*std::min(.014+.012*(1.-m),.42/hz)));
                const auto cap=channels_[0].history.size();
                start_=(write_+cap-grain_)%cap;age_=0;++events_;
            }
        }
        return static_cast<float>(std::sin(6.283185307179586*phase_));
    }
    float excite(unsigned ch,float x,float m,float pulse) noexcept {
        auto& c=channels_[ch];const auto cap=c.history.size();
        c.low1+=lowPole_*(x-c.low1);c.low2+=lowPole_*(c.low1-c.low2);
        float replay=0.f;
        if(age_<2*grain_) {
            const auto pos=age_%grain_;
            const float u=static_cast<float>(pos)/static_cast<float>(grain_-1);
            const auto taper=[](float z){z=std::clamp(z,0.f,1.f);return z*z*(3.f-2.f*z);};
            const float window=taper(u/.12f)*taper((1.f-u)/.12f);
            replay=.65f*m*m*window*(c.history[(start_+pos)%cap]-c.low2);
        }
        c.history[write_]=c.low2;
        // TPT state-variable bandpass: positive damping and bounded conductance.
        const float g=gLo_+(gHi_-gLo_)*m*pulse,k=1.f/(.8f+1.2f*m);
        const float a=1.f/(1.f+g*(g+k));
        const float v1=a*(c.ic1+g*(c.low2-c.ic2)),v2=c.ic2+g*v1;
        c.ic1=2.f*v1-c.ic1;c.ic2=2.f*v2-c.ic2;
        return x+replay+.24f*m*k*v1;
    }
    void advance() noexcept {
        if(++write_==channels_[0].history.size())write_=0;
        if(age_<2*grain_)++age_;
    }
private:
    struct Channel {std::vector<float> history;float low1=0.f,low2=0.f,ic1=0.f,ic2=0.f;};
    std::array<Channel,2> channels_{};
    double rate_=48000.,phase_=0.;
    float fast_=0.f,slow_=0.f,fastPole_=0.f,slowPole_=0.f,lowPole_=0.f,gLo_=0.f,gHi_=0.f;
    std::size_t write_=0,start_=0,grain_=0,age_=0;
    std::uint64_t events_=0;
};
}
