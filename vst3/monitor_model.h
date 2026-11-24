#pragma once
#include <algorithm>
#include <array>
#include <cmath>
namespace psychiatry::vst3 {
struct MonitorModel {
    static constexpr std::size_t worldCount=12;
    std::array<float,worldCount> levels{};
    float wet=1.f, peak=0.f, total=0.f;
    int dominant=0;
    bool bypass=false;
    static float norm(double value) noexcept {
        return std::isfinite(value)?static_cast<float>(std::clamp(value,0.,1.)):0.f;
    }
    static MonitorModel make(const std::array<double,worldCount>& macros,double therapy,double bypassValue) noexcept {
        MonitorModel model;
        model.bypass=norm(bypassValue)>=.5f;
        model.wet=model.bypass?0.f:1.f-norm(therapy);
        for(std::size_t i=0;i<model.levels.size();++i) {
            model.levels[i]=norm(macros[i])*model.wet;
            model.total+=model.levels[i];
            if(model.levels[i]>model.peak){model.peak=model.levels[i];model.dominant=static_cast<int>(i);}
        }
        return model;
    }
    // Index of the next strongest active world. Equal strengths use the
    // same stable lower-ID tie rule as the dominant world.
    int secondary() const noexcept {
        int result=-1;float value=0.f;
        for(std::size_t i=0;i<levels.size();++i)
            if(static_cast<int>(i)!=dominant&&levels[i]>value){
                result=static_cast<int>(i);value=levels[i];
            }
        return result;
    }
    bool operator==(const MonitorModel& rhs) const noexcept {
        return levels==rhs.levels&&wet==rhs.wet&&bypass==rhs.bypass;
    }
};
}
