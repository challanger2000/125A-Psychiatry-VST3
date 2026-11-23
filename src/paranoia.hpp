#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace psychiatry {

// An audio-only P001 proof of concept, NOT a VST3 or a finalized macro.
// All buffers are allocated during prepare(), never by process().
class Paranoia final {
public:
    enum class Trigger { Timed, Transient, Hybrid };
    enum class Pattern { Every, Alternating, Random };
    enum class Clock { Milliseconds, BeatDivision };

    struct Settings {
        float amount = 0.0f;       // 0..1
        float therapy = 0.0f;      // 0 = processed, 1 = dry
        Trigger trigger = Trigger::Hybrid;
        Pattern pattern = Pattern::Random;
        Clock clock = Clock::Milliseconds;
        float intervalMs = 240.0f; // 20..2000; used for timed/hybrid
        unsigned division = 4;     // triggers per quarter-note beat, 1..16
        float bpm = 120.0f;        // 20..300, host integration later
    };
    struct Stats {
        std::uint64_t triggers = 0;
        std::uint64_t reverseEvents = 0;
        std::uint64_t reversedSamples = 0;
    };

    void prepare(double sampleRate);
    void reset() noexcept;
    void setSettings(Settings settings) noexcept;
    void setSeed(std::uint64_t seed) noexcept;
    // In-place; 1 or 2 planar float channels; arbitrary number of frames.
    void process(float* const* channels, unsigned channelCount,
                 std::size_t frames) noexcept;
    Stats stats() const noexcept { return stats_; }
    unsigned latencySamples() const noexcept { return 0; } // reverses past audio, not future audio

private:
    struct Stereo { float l = 0.0f, r = 0.0f; };
    std::vector<Stereo> lookback_;
    std::vector<Stereo> feedback_;
    std::size_t cursor_ = 0;
    std::size_t filled_ = 0;
    std::size_t remaining_ = 0;
    std::size_t reverseCursor_ = 0;
    std::size_t reverseLength_ = 0;
    std::size_t elapsed_ = 0;
    std::size_t interval_ = 1;
    double sampleRate_ = 48000.0;
    float smoothedAmount_ = 0.0f;
    float smoothedTherapy_ = 0.0f;
    float previousEnvelope_ = 0.0f;
    float envelope_ = 0.0f;
    std::uint64_t random_ = 0xACCE55123456789ULL;
    bool alternate_ = false;
    Settings settings_{};
    Stats stats_{};

    std::uint32_t nextRandom() noexcept;
    void scheduleNext() noexcept;
    void triggerReverse(float intensity) noexcept;
};
} // namespace psychiatry
