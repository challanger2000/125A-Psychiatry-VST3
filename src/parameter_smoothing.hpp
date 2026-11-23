#pragma once
#include <cmath>

namespace psychiatry::detail {

// P017: maintain click-free exponential parameter motion, but ensure exact
// neutral/full endpoints are actually reached after automation. The old
// recurrence approached 0 forever (and could stall a few ulps below 1).
// Snap only within a negligible residual, or if float precision has stalled.
inline float smoothToward(float current, float target, float coefficient) noexcept {
    const float next = current + coefficient * (target-current);
    if ((target==0.f || target==1.f) &&
        (std::abs(target-next)<=1.e-5f || next==current))
        return target;
    return next;
}

} // namespace psychiatry::detail
