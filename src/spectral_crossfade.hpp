#pragma once

namespace psychiatry::detail {
// Partial power compensation for decorrelated dry/shifted spectral voices.
// Linear coefficients at 50% can lose 3 dB even at equal branch RMS.
// This conservative polynomial restores only 1.94 dB at midpoint, not
// the full equal-power 3 dB; no envelope tracking, automatic normalization
// or gain in the delay/feedback path. Both endpoints are exact.
inline float spectralCrossfade(float dry,float wet,float m) noexcept {
    if(m==0.f)return dry;
    if(m==1.f)return wet;
    return (dry+m*(wet-dry))*(1.f+m*(1.f-m));
}
}
