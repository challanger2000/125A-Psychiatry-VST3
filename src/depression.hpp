#pragma once
#include <array>
#include <cstddef>
#include <vector>

namespace psychiatry {
// P003 reference chain: program-driven crush -> downward filter -> dark modulated
// plate-like feedback delay network. No realtime allocations.
class Depression final {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void setAmount(float normalized) noexcept;
    void process(float* const* channels, unsigned channelCount, std::size_t frames) noexcept;

private:
    static constexpr std::size_t kDelayCount=4;
    struct ChannelState {
        std::array<std::vector<float>,kDelayCount> delay;
        std::array<std::size_t,kDelayCount> cursor{};
        std::array<float,kDelayCount> damping{};
        float darkness=0.f;
        float dynamics=0.f;
    };
    std::array<ChannelState,2> state_{};
    float target_=0.f;
    float smoothed_=0.f;
    float step_=0.001f;
    double rate_=48000.0;
};
} // namespace psychiatry
