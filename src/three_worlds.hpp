#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

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
    };
    std::array<Channel,2> state_{};
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
    };
    std::array<Channel,2> state_{};
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
    // P012: allocation-free 2x over/undersampling around ALL nonlinear
    // branches. Three cascaded Butterworth biquads per direction, per band.
    // These have no hidden dry mix and are reset in setActive().
    struct Coeff {
        float b0=0.f,b1=0.f,b2=0.f,a1=0.f,a2=0.f;
    };
    struct Biquad {
        float z1=0.f,z2=0.f;
        float process(float x,const Coeff& c) noexcept {
            const float y=c.b0*x+z1;
            z1=c.b1*x-c.a1*y+z2;
            z2=c.b2*x-c.a2*y;
            return y;
        }
    };
    struct Channel {
        float low=0.f,lowMid=0.f,driveEnvelope=0.f;
        // Upsampler for low/mid/high. Decimator is shared after the
        // combined nonlinear wet branches for each individual channel.
        std::array<std::array<Biquad,3>,3> up{};
        std::array<Biquad,3> down{};
    };
    std::array<Channel,2> state_{};
    std::array<Coeff,3> oversampleCoeffs_{};
    float amount_=0.f,smoothed_=0.f,step_=0.001f;
    double rate_=48000.0;
};
} // namespace psychiatry
