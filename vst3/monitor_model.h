#pragma once
#include <algorithm>
#include <array>
#include <cmath>
namespace psychiatry::vst3 {
struct MonitorModel {
    std::array<float,9> levels{};
    float wet=1.f, peak=0.f, total=0.f;
    int dominant=0;
    bool bypass=false;
    static float norm(double value) noexcept {
        return std::isfinite(value)?static_cast<float>(std::clamp(value,0.,1.)):0.f;
    }
    static MonitorModel make(const std::array<double,9>& macros,double therapy,double bypassValue) noexcept {
        MonitorModel model;
        model.bypass=norm(bypassValue)>=.5f;
        model.wet=model.bypass?0.f:1.f-norm(therapy);
        for(unsigned i=0;i<9;++i) {
            model.levels[i]=norm(macros[i])*model.wet;
            model.total+=model.levels[i];
            if(model.levels[i]>model.peak){model.peak=model.levels[i];model.dominant=static_cast<int>(i);}
        }
        return model;
    }
    bool operator==(const MonitorModel& rhs) const noexcept {
        return levels==rhs.levels&&wet==rhs.wet&&bypass==rhs.bypass;
    }
};
}
