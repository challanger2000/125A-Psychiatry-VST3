#pragma once
#include <array>
#include <cmath>
#include <cstddef>

namespace psychiatry::detail {
// P013: true streamed 2x nonlinear-only interpolation and decimation.
// 6th-order Butterworth reconstruction and independent anti-alias filter.
// No heap, no host/DAW interaction, no extra dry signal.
class Oversample2x final {
public:
    void prepare() noexcept {
        constexpr std::array<double,3> q{0.5176380902050415,0.7071067811865476,1.9318516525781366};
        constexpr double omega=6.2831853071795864769*0.205;
        const double cs=std::cos(omega),sn=std::sin(omega);
        for(std::size_t i=0;i<3;++i) {
            const double alpha=sn/(2.*q[i]),a0=1.+alpha;
            auto& c=coeffs_[i];
            c.b0=static_cast<float>((1.-cs)/(2.*a0));
            c.b1=2.f*c.b0;c.b2=c.b0;
            c.a1=static_cast<float>(-2.*cs/a0);
            c.a2=static_cast<float>((1.-alpha)/a0);
        }
        reset();
    }
    void reset() noexcept {up_={};down_={};}
    template<class Shaper>
    float process(float input,Shaper&& shape) noexcept {
        float out=0.f;
        for(int phase=0;phase<2;++phase) {
            float x=filtered(up_,phase==0?2.f*input:0.f);
            x=shape(x);
            const float y=filtered(down_,x);
            if(phase==0)out=y;
        }
        return out;
    }
private:
    struct Coeff {float b0=0.f,b1=0.f,b2=0.f,a1=0.f,a2=0.f;};
    struct Section {
        float z1=0.f,z2=0.f;
        float run(float x,const Coeff& c) noexcept {
            const float y=c.b0*x+z1;
            z1=c.b1*x-c.a1*y+z2;
            z2=c.b2*x-c.a2*y;
            return y;
        }
    };
    float filtered(std::array<Section,3>& stages,float x) noexcept {
        for(std::size_t i=0;i<3;++i)x=stages[i].run(x,coeffs_[i]);
        return x;
    }
    std::array<Coeff,3> coeffs_{};
    std::array<Section,3> up_{},down_{};
};
} // namespace psychiatry::detail
