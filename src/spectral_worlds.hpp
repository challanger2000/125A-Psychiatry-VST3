#pragma once
#include <array>
#include <cstddef>
#include <vector>

namespace psychiatry {
// P005 prototypes. Signal-derived FX only. Individual state and memory per instance.
// "Schizophrenia" is a fictional sound-world title, not a clinical simulation.

class Schizophrenia final {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void setAmount(float amount) noexcept;
    void process(float* const* audio,unsigned channels,std::size_t frames) noexcept;
private:
    struct Voice {
        float phase=0.f,low=0.f,high=0.f;
    };
    struct Channel {
        std::vector<float> ring;
        std::size_t write=0;
        std::array<Voice,3> voices{};
    };
    std::array<Channel,2> state_{};
    double rate_=48000.;
    float amount_=0.f,smoothed_=0.f,step_=0.001f;
};

class Dissociation final {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void setAmount(float amount) noexcept;
    void process(float* const* audio,unsigned channels,std::size_t frames) noexcept;
private:
    struct Channel {
        std::vector<float> frozen, recording;
        std::size_t capture=0,play=0,untilCapture=0;
        float blur=0.f,low=0.f,high=0.f;
        bool ready=false, recordingReady=false;
    };
    std::array<Channel,2> state_{};
    double rate_=48000.;
    float amount_=0.f,smoothed_=0.f,step_=0.001f;
};

class Hallucination final {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void setAmount(float amount) noexcept;
    void process(float* const* audio,unsigned channels,std::size_t frames) noexcept;
private:
    struct Channel {
        std::array<float,17> analytic{};
        std::size_t analyticWrite=0;
        std::vector<float> echo;
        std::size_t echoWrite=0;
        float phase=0.f,formantLow=0.f,formantHigh=0.f;
    };
    std::array<Channel,2> state_{};
    double rate_=48000.;
    float amount_=0.f,smoothed_=0.f,step_=0.001f;
};
} // namespace psychiatry
