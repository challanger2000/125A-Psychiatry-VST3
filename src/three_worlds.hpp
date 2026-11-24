#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "linear_phase_2x.hpp"

namespace psychiatry {

// Three independent P004 compound FX prototypes. A 0% macro must return the
// exact incoming sample, and every processor is allocation-free in process().
// Each class has its own state and no shared/global RNG or audio context.

class Mania final {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setAmount(float amount) noexcept;
    void process(float* const* audio,unsigned channels,std::size_t frames) noexcept;
private:
    struct Channel {
        float phase=0.f,feedback=0.f,fast=0.f,slow=0.f;
        detail::LinearPhaseOversample2x saturation{};
        detail::FixedDelay<detail::kLinearPhaseLatency> dryDelay{};
    };
    std::array<Channel,2> state_{};
    detail::FixedDelay<detail::kLinearPhaseLatency> mixDelay_{};
    float amount_=0.f,smoothed_=0.f,step_=0.001f;
    double rate_=48000.0;
};

class Panic final {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setAmount(float amount) noexcept;
    void process(float* const* audio,unsigned channels,std::size_t frames) noexcept;
    std::uint64_t impulses() const noexcept {return impulseCount_;}
private:
    struct Channel {
        float fast=0.f,slow=0.f,phase=0.f,envelope=0.f;
        float last=0.f,slowHigh=0.f;
        std::uint32_t rng=0xB54CA515u;
        unsigned cooldown=0;
        detail::LinearPhaseOversample2x saturation{};
        detail::FixedDelay<detail::kLinearPhaseLatency> dryDelay{};
    };
    std::array<Channel,2> state_{};
    detail::FixedDelay<detail::kLinearPhaseLatency> mixDelay_{};
    float amount_=0.f,smoothed_=0.f,step_=0.001f;
    double rate_=48000.0;
    std::uint64_t impulseCount_=0;
};

class Psychosis final {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setAmount(float amount) noexcept;
    void process(float* const* audio,unsigned channels,std::size_t frames) noexcept;
private:
    struct Channel {
        float low=0.f,lowMid=0.f,driveEnvelope=0.f;
        std::array<detail::LinearPhaseInterpolator2x,3> up{};
        detail::LinearPhaseDecimator2x down{};
        detail::FixedDelay<detail::kLinearPhaseLatency> dryDelay{};
    };
    std::array<Channel,2> state_{};
    detail::FixedDelay<detail::kLinearPhaseLatency> mixDelay_{};
    float amount_=0.f,smoothed_=0.f,step_=0.001f;
    double rate_=48000.0;
};
} // namespace psychiatry
