#include "paranoia.hpp"
#include "parameter_smoothing.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace psychiatry {
namespace {
float clamp01(float x) noexcept {
    return std::isfinite(x) ? std::clamp(x, 0.0f, 1.0f) : 0.0f;
}
float safeSample(float x) noexcept {
    if (!std::isfinite(x) || std::abs(x) < 1e-25f) return 0.0f;
    return std::clamp(x, -16.0f, 16.0f);
}
std::size_t wrapBack(std::size_t idx, std::size_t back, std::size_t size) noexcept {
    return (idx + size - (back % size)) % size;
}
} // namespace

void Paranoia::prepare(double sampleRate) {
    if (!std::isfinite(sampleRate) || sampleRate < 1000.0 || sampleRate > 2000000.0)
        sampleRate = 48000.0;
    sampleRate_ = sampleRate;
    const auto capacity = static_cast<std::size_t>(sampleRate_ * 2.1) + 16;
    lookback_.assign(capacity, {});
    feedback_.assign(capacity, {});
    acoustic_.prepare(sampleRate_);
    reset();
}

void Paranoia::reset() noexcept {
    std::fill(lookback_.begin(), lookback_.end(), Stereo{});
    std::fill(feedback_.begin(), feedback_.end(), Stereo{});
    cursor_ = filled_ = remaining_ = reverseCursor_ = reverseLength_ = 0;
    elapsed_ = 0;
    previousEnvelope_ = envelope_ = 0.0f;
    smoothedAmount_ = clamp01(settings_.amount);
    smoothedTherapy_ = clamp01(settings_.therapy);
    alternate_ = false;
    stats_ = {};
    acoustic_.reset();
    random_ = seed_; // Reproduce event timing after resetting playback.
    scheduleNext();
}

void Paranoia::setSettings(Settings p) noexcept {
    p.amount = clamp01(p.amount);
    p.therapy = clamp01(p.therapy);
    p.bpm = std::isfinite(p.bpm) ? std::clamp(p.bpm, 20.0f, 300.0f) : 120.0f;
    p.intervalMs = std::isfinite(p.intervalMs) ? std::clamp(p.intervalMs, 20.0f, 2000.0f) : 240.0f;
    p.division = std::clamp(p.division, 1u, 16u);
    settings_ = p;
    // The period is recomputed at the next scheduled event to prevent jitter.
}

void Paranoia::setSeed(std::uint64_t seed) noexcept {
    seed_ = seed == 0 ? 0xACCE55123456789ULL : seed;
    random_ = seed_;
}

std::uint32_t Paranoia::nextRandom() noexcept {
    // xorshift64*: deterministic across Windows/Linux, no allocation.
    random_ ^= random_ >> 12;
    random_ ^= random_ << 25;
    random_ ^= random_ >> 27;
    return static_cast<std::uint32_t>((random_ * 2685821657736338717ULL) >> 32);
}

void Paranoia::scheduleNext() noexcept {
    const double seconds = settings_.clock == Clock::BeatDivision
        ? 60.0 / (settings_.bpm * settings_.division)
        : settings_.intervalMs * 0.001;
    // A slight deterministic jitter at higher intensity; still tempo-adherent in BeatDivision mode.
    const float m = clamp01(settings_.amount);
    const double jitter = settings_.clock == Clock::BeatDivision
        ? 1.0
        : 1.0 + (static_cast<double>(nextRandom()) / 4294967295.0 - 0.5) * (0.40 * m);
    interval_ = std::max<std::size_t>(1, static_cast<std::size_t>(sampleRate_ * seconds * jitter));
    elapsed_ = 0;
}

void Paranoia::triggerReverse(float intensity) noexcept {
    ++stats_.triggers;
    bool fire = true;
    switch (settings_.pattern) {
        case Pattern::Every: break;
        case Pattern::Alternating: alternate_ = !alternate_; fire = alternate_; break;
        case Pattern::Random: fire = (nextRandom() & 0xffffu) < 0xa000u; break;
    }
    if (!fire || lookback_.empty() || filled_ < 64) return;
    const auto cap = lookback_.size();
    const auto proposed = static_cast<std::size_t>(sampleRate_ * (0.035 + 0.165 * intensity));
    reverseLength_ = std::min({std::max<std::size_t>(proposed, 64), filled_ - 1, cap - 1});
    remaining_ = reverseLength_;
    reverseCursor_ = wrapBack(cursor_, 1, cap); // last completed write
    ++stats_.reverseEvents;
}

void Paranoia::process(float* const* channels, unsigned channelCount,
                       std::size_t frames) noexcept {
    if (lookback_.empty() || !channels || !channels[0] ||
        (channelCount != 1 && channelCount != 2) ||
        (channelCount == 2 && !channels[1])) return;
    const std::size_t size = lookback_.size();
    const float coefficient = static_cast<float>(1.0 - std::exp(-1.0 / (0.015 * sampleRate_)));
    const float envCoef = static_cast<float>(1.0 - std::exp(-1.0 / (0.004 * sampleRate_)));
    for (std::size_t i = 0; i < frames; ++i) {
        const float inL = safeSample(channels[0][i]);
        const float inR = channelCount == 2 ? safeSample(channels[1][i]) : inL;
        smoothedAmount_ = detail::smoothToward(smoothedAmount_, settings_.amount, coefficient);
        smoothedTherapy_ = detail::smoothToward(smoothedTherapy_, settings_.therapy, coefficient);
        const float m = smoothedAmount_;

        // Keep independent input history, preserving the previous audio needed by reverse events.
        lookback_[cursor_] = {inL, inR};
        filled_ = std::min(filled_ + 1, size);
        const float level = std::max(std::abs(inL), std::abs(inR));
        envelope_ += envCoef * (level - envelope_);
        const bool transient = envelope_ > (0.07f + 0.20f * (1.0f - m)) &&
            envelope_ - previousEnvelope_ > (0.00015f + 0.00035f * (1.0f - m));
        previousEnvelope_ = envelope_;
        const auto ghost=acoustic_.tick(envelope_,m);
        const float ghostL=channelCount==1?.5f*(ghost.l+ghost.r):ghost.l;
        const float ghostR=channelCount==1?ghostL:ghost.r;

        if (++elapsed_ >= interval_) {
            if (settings_.trigger != Trigger::Transient && m > 0.0001f) triggerReverse(m);
            scheduleNext();
        } else if (transient && settings_.trigger != Trigger::Timed &&
                   m > 0.0001f && elapsed_ > static_cast<std::size_t>(sampleRate_ * 0.035)) {
            triggerReverse(m);
            scheduleNext();
        }

        // Feedback comb: nonlinearity *inside* the feedback path, not a final generic overdrive.
        const auto delayL = static_cast<std::size_t>(sampleRate_ * (0.013 + 0.026 * m));
        const auto delayR = static_cast<std::size_t>(sampleRate_ * (0.021 + 0.031 * m));
        const float fbL = feedback_[wrapBack(cursor_, delayL, size)].l;
        const float fbR = feedback_[wrapBack(cursor_, delayR, size)].r;
        const float fbGain = 0.12f + 0.55f * m;
        feedback_[cursor_] = {
            std::tanh(0.62f * inL + 0.42f * ghostL + fbGain * fbL),
            std::tanh(0.62f * inR + 0.42f * ghostR + fbGain * fbR)
        };

        Stereo reverse{};
        if (remaining_ > 0) {
            const auto consumed = reverseLength_ - remaining_;
            const auto fade = std::max<std::size_t>(1, static_cast<std::size_t>(sampleRate_ * 0.004));
            const float envelope = std::min({1.0f,
                static_cast<float>(consumed) / static_cast<float>(fade),
                static_cast<float>(remaining_) / static_cast<float>(fade)});
            reverse = lookback_[reverseCursor_];
            reverse.l *= envelope;
            reverse.r *= envelope;
            reverseCursor_ = wrapBack(reverseCursor_, 1, size);
            --remaining_;
            ++stats_.reversedSamples;
        }

        // Parallel spatially divergent echo and bounded, stereo-dependent reverse ghosts.
        const float wetL = std::tanh((0.85f - 0.21f * m) * inL +
            (0.18f + 0.18f * m) * fbR + (0.35f + 0.32f * m) * reverse.l + ghostL);
        const float wetR = std::tanh((0.85f - 0.21f * m) * inR +
            (0.18f + 0.18f * m) * fbL + (0.35f + 0.32f * m) * reverse.r + ghostR);
        const float affectedL = inL + m * (wetL - inL);
        const float affectedR = inR + m * (wetR - inR);
        const float wetAmount = 1.0f - smoothedTherapy_;
        // Internal history sanitation must not alter inactive output.
        if (m > 0.f && wetAmount > 0.f) {
            channels[0][i] = inL + wetAmount * (affectedL - inL);
            if (channelCount == 2)
                channels[1][i] = inR + wetAmount * (affectedR - inR);
        }
        cursor_ = (cursor_ + 1) % size;
    }
}
} // namespace psychiatry
