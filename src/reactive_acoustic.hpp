#pragma once
#include <cstddef>
#include <cstdint>

namespace psychiatry {
// Sparse, signal-reactive source player. No heap, I/O or codec at runtime.
// Assets generated at build time, immutable and shared; playback state private.
class ReactiveAcoustic final {
public:
    struct Stereo {float l=0.f,r=0.f;};
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    Stereo tick(float envelope,float amount) noexcept;
    std::uint64_t events() const noexcept { return events_; }
private:
    float random() noexcept;
    void begin() noexcept;
    double rate_=48000.,position_=0.,increment_=1.;
    std::size_t clip_=0,begin_=0,end_=0;
    unsigned cooldown_=0;
    float pan_=0.f;
    bool playing_=false;
    std::uint32_t rng_=0x125A6B71u;
    std::uint64_t events_=0;
};
}
