#pragma once
#include <array>
#include <cstddef>
#include <vector>
#include <cstdint>

namespace psychiatry {
// P003 micro-loop apparatus. Sample history is always captured, repetition
// rate and replay degradation are coupled by a single macro.
class Obsession final {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void setAmount(float amount) noexcept;
    void process(float* const* channels,unsigned channelCount,std::size_t frames) noexcept;
    std::uint64_t retriggers() const noexcept { return retriggers_; }
private:
    struct Sample {float l=0.f,r=0.f;};
    std::vector<Sample> history_;
    std::size_t cursor_=0, filled_=0, start_=0, phase_=0, loopLength_=1;
    unsigned repeats_=0;
    std::uint64_t retriggers_=0;
    double rate_=48000.;
    float target_=0.f, smoothed_=0.f, step_=0.001f, reducer_=0.f;
};
} // namespace psychiatry
