#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <random>

namespace psychiatry::vst3 {
// A random preset replaces ALL twelve values. Select a uniformly shuffled
// subset of 3–6 active worlds; every world is eligible on every press.
// No audio-dependent normalization, no global parameter changes.
inline std::array<double,12> randomLossOfControl(std::mt19937& rng) {
    std::array<std::size_t,12> indices{0,1,2,3,4,5,6,7,8,9,10,11};
    std::shuffle(indices.begin(),indices.end(),rng);
    const auto count=std::uniform_int_distribution<int>(3,6)(rng);
    std::uniform_real_distribution<double> amount(.12,.78);
    std::array<double,12> result{};
    for(int i=0;i<count;++i)result[indices[static_cast<std::size_t>(i)]]=amount(rng);
    return result;
}
}
