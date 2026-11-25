#pragma once
#include <array>
#include <cstddef>
namespace psychiatry::detail {
// 49-tap symmetric halfband FIR. The center tap is exactly 1/2 and
// all other even taps vanish. Paired odd taps sum to 1/2 (DC unity).
inline constexpr std::array<float,12> kHalfbandPairs{
    -0.000115388950857f, 0.000394901084403f, -0.00101504469796f,
    0.00219631759435f, -0.00424126431553f, 0.00755374002956f,
    -0.0126986967562f, 0.0205712172795f, -0.0329042652715f,
    0.0540618395534f, -0.100080981319f, 0.31627762577f};
inline constexpr std::size_t kLinearPhaseLatency=24;
class LinearPhaseInterpolator2x {
public:
    void reset() noexcept {history_={};position_=0;}
    std::array<float,2> process(float x) noexcept {
        history_[position_]=history_[position_+24]=x;
        const auto past=[this](std::size_t k) noexcept {return history_[position_+24-k];};
        float odd=0.f;
        for(std::size_t j=0;j<12;++j)
            odd+=kHalfbandPairs[j]*(past(j)+past(23-j));
        const std::array<float,2> out{past(12),2.f*odd};
        if(++position_==24)position_=0;
        return out;
    }
private:
    std::array<float,48> history_{};
    std::size_t position_=0;
};
class LinearPhaseDecimator2x {
public:
    void reset() noexcept {even_={};odd_={};position_=0;}
    float process(float a,float b) noexcept {
        even_[position_]=a;
        const auto pastOdd=[this](std::size_t k) noexcept {return odd_[position_+24-k];};
        float sum=0.5f*even_[(position_+24-12)%24];
        for(std::size_t j=0;j<12;++j)
            sum+=kHalfbandPairs[j]*(pastOdd(j+1)+pastOdd(24-j));
        odd_[position_]=odd_[position_+24]=b;
        if(++position_==24)position_=0;
        return sum;
    }
private:
    std::array<float,24> even_{};
    std::array<float,48> odd_{};
    std::size_t position_=0;
};
class LinearPhaseOversample2x final {
public:
    void prepare() noexcept {reset();}
    void reset() noexcept {up_.reset();down_.reset();}
    template<class Shape> float process(float x,Shape&& shape) noexcept {
        const auto pair=up_.process(x);
        const float a=shape(pair[0]);
        const float b=shape(pair[1]);
        return down_.process(a,b);
    }
private:
    LinearPhaseInterpolator2x up_{};
    LinearPhaseDecimator2x down_{};
};
template<std::size_t N> class FixedDelay {
public:
    void reset(float initial=0.f) noexcept {buffer_.fill(initial);position_=0;}
    float process(float x) noexcept {
        const float out=buffer_[position_];
        buffer_[position_]=x;
        position_=(position_+1)%N;
        return out;
    }
private:
    static_assert(N>0,"positive fixed latency");
    std::array<float,N> buffer_{};
    std::size_t position_=0;
};
}
