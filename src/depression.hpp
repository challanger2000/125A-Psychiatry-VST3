#pragma once
#include <array>
#include <cstddef>
#include <vector>
#include "linear_phase_2x.hpp"

namespace psychiatry {
// P026g: transient attenuation, slow leveling, dark body and pitch drift,
// subordinate diffuse tail. All history allocated during prepare.
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
        float dynamics=0.f,fastEnvelope=0.f,slowEnvelope=0.f;
        float presence=0.f,driftPhase=0.f;
        std::vector<float> drift;
        std::size_t driftWrite=0;
        detail::FixedDelay<detail::kLinearPhaseLatency> gainDelay{};
        detail::LinearPhaseOversample2x saturation{};
        detail::FixedDelay<detail::kLinearPhaseLatency> dryDelay{};
    };
    std::array<ChannelState,2> state_{};
    detail::FixedDelay<detail::kLinearPhaseLatency> mixDelay_{};
    float target_=0.f;
    float smoothed_=0.f;
    float step_=0.001f;
    double rate_=48000.0;
};
} // namespace psychiatry
