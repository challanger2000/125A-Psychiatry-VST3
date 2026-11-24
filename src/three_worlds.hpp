#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "linear_phase_2x.hpp"
#include "mania_retrigger.hpp"

namespace psychiatry {

// Three independent P004 compound FX prototypes. A 0% macro must return the
// exact incoming sample, and every processor is allocation-free in process().
// Each class has its own state and no shared/global RNG or audio context.

class Mania final {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void setAmount(float amount) noexcept;
    void process(float* const* audio,unsigned channels,std::size_t frames) noexcept;
    std::uint64_t retriggers() const noexcept {return retrigger_.events();}
private:
    struct Channel {
        float feedback=0.f;
        detail::LinearPhaseOversample2x saturation{};
        detail::FixedDelay<detail::kLinearPhaseLatency> dryDelay{};
    };
    detail::ManiaRetrigger retrigger_{};
    std::array<Channel,2> state_{};
    detail::FixedDelay<detail::kLinearPhaseLatency> mixDelay_{};
    float amount_=0.f,smoothed_=0.f,step_=0.001f;
    double rate_=48000.0;
};

class Panic final {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void setAmount(float amount) noexcept;
    void process(float* const* audio,unsigned channels,std::size_t frames) noexcept;
    std::uint64_t impulses() const noexcept {return impulseCount_;}
private:
    struct Channel {
        std::vector<float> history;
        float body=0.f,edge=0.f;
        detail::LinearPhaseOversample2x saturation{};
        detail::FixedDelay<detail::kLinearPhaseLatency> dryDelay{};
    };
    std::array<Channel,2> state_{};
    detail::FixedDelay<detail::kLinearPhaseLatency> mixDelay_{};
    float amount_=0.f,smoothed_=0.f,step_=0.001f;
    double rate_=48000.0;
    float fast_=0.f,slow_=0.f,depth_=0.f;
    std::size_t write_=0;
    unsigned age_=0,length_=0,cooldown_=0,kind_=0;
    std::uint32_t rng_=0xCA125A11u;
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
        float low=0.f,lowMid=0.f,lowMidFiltered=0.f,driveEnvelope=0.f,dc=0.f,focus=0.f;
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
