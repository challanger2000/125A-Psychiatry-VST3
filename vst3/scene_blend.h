#pragma once
#include <algorithm>
namespace psychiatry::vst3 {
struct SceneAlpha {float previous,current;};
inline SceneAlpha sceneAlpha(float level,float fade) noexcept {
    const float a=level*fade;
    const float b=a<1.f?level*(1.f-fade)/(1.f-a):0.f;
    return {std::clamp(b,0.f,1.f),a};
}
}
