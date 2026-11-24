#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace psychiatry::detail {
// Causal two-head granular overlap/add. A launched grain is immutable:
// each source interval lies entirely before launch, and its oldest frame
// remains less than 2*grain + maxOffset frames behind the write cursor.
class DissociationGrains final {
public:
    void prepare(double sr) {
        rate_=sr;
        hop_=std::max<std::size_t>(2,static_cast<std::size_t>(std::round(.048*sr)));
        grain_=2*hop_;
        const auto cap=2*grain_+static_cast<std::size_t>(std::ceil(.090*sr))+8;
        for(std::size_t ch=0;ch<state_.size();++ch) {
            auto& c=state_[ch];c.history.assign(cap,0.f);
            c.diffusion[0].buffer.assign(std::max<std::size_t>(1,static_cast<std::size_t>(sr*(ch?.0131:.0097))),0.f);
            c.diffusion[1].buffer.assign(std::max<std::size_t>(1,static_cast<std::size_t>(sr*(ch?.0217:.0173))),0.f);
        }
        reset();
    }
    bool ready() const noexcept {return !state_[0].history.empty();}
    void reset() noexcept {
        for(auto& c:state_) {
            std::fill(c.history.begin(),c.history.end(),0.f);
            c.heads={};
            for(auto& d:c.diffusion){std::fill(d.buffer.begin(),d.buffer.end(),0.f);d.position=0;}
        }
        write_=position_=current_=0;sequence_=0;
    }
    void beginFrame(float m,unsigned channels) noexcept {
        if(position_!=0)return;
        constexpr std::array<double,8> offsets{.0,.037,.013,.061,.023,.048,.006,.072};
        for(unsigned ch=0;ch<channels;++ch) {
            auto& c=state_[ch];auto& h=c.heads[current_];
            const auto index=static_cast<std::size_t>((sequence_+(ch?3u:0u))%offsets.size());
            const auto offset=static_cast<std::size_t>(rate_*m*(offsets[index]+(ch?.009:0.)));
            const auto cap=c.history.size();
            h.start=(write_+cap-grain_-offset)%cap;
            // Direction is latched at launch, never changed mid-grain.
            h.reverse=m>.20f && (index%4u!=0u || m<.55f);
            h.active=true;
        }
        ++sequence_;
    }
    float weight() const noexcept {
        const double u=static_cast<double>(position_)/static_cast<double>(hop_);
        return static_cast<float>(.5-.5*std::cos(3.14159265358979323846*u));
    }
    float processChannel(unsigned ch,float x,float m,float w) noexcept {
        auto& c=state_[ch];
        const auto read=[&](std::size_t lane,std::size_t age) noexcept {
            const auto& h=c.heads[lane];
            if(!h.active)return 0.f;
            const auto index=h.reverse?grain_-1-age:age;
            return c.history[(h.start+index)%c.history.size()];
        };
        const float grainSignal=w*read(current_,position_)+(1.f-w)*read(1-current_,hop_+position_);
        c.history[write_]=x;
        // Two short serial allpasses add diffusion, not a dominant reverb.
        // Stable feedback coefficients have |g| < 1.
        const float diffuse=c.diffusion[1].process(c.diffusion[0].process(grainSignal,.48f),.41f);
        const float blend=.08f+.12f*m;
        return (1.f+.55f*m*m)*((1.f-blend)*grainSignal+blend*diffuse);
    }
    void endFrame() noexcept {
        if(++write_==state_[0].history.size())write_=0;
        if(++position_==hop_){position_=0;current_=1-current_;}
    }
private:
    struct Head {std::size_t start=0;bool reverse=false,active=false;};
    struct Allpass {
        std::vector<float> buffer;std::size_t position=0;
        float process(float x,float g) noexcept {
            const float y=buffer[position]-g*x;
            buffer[position]=x+g*y;
            if(++position==buffer.size())position=0;
            return y;
        }
    };
    struct Channel {
        std::vector<float> history;
        std::array<Head,2> heads{};
        std::array<Allpass,2> diffusion{};
    };
    std::array<Channel,2> state_{};
    double rate_=48000.;
    std::size_t hop_=0,grain_=0,write_=0,position_=0,current_=0;
    std::uint64_t sequence_=0;
};
}
