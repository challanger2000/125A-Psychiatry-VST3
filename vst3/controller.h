#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include <random>

namespace psychiatry::vst3 {
class Controller final : public Steinberg::Vst::EditControllerEx1 {
public:
    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IEditController*>(new Controller);
    }
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;
    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) override;
    // UI-thread action. Changes all twelve real host-automatable macro IDs.
    // Never modifies the global THERAPY mix, Bypass, or the audio callback.
    void triggerLossOfControl();
    bool isGermanUi() const noexcept {return germanUi_;}
    void setGermanUi(bool german) noexcept {germanUi_=german;}
    bool isMotionEnabled() const noexcept {return motionEnabled_;}
    void setMotionEnabled(bool value) noexcept {motionEnabled_=value;}
    int zoomIndex() const noexcept {return zoomIndex_;}
    void setZoomIndex(int index) noexcept {if(index>=0&&index<6)zoomIndex_=index;}
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getParamStringByValue(
        Steinberg::Vst::ParamID id,
        Steinberg::Vst::ParamValue value,
        Steinberg::Vst::String128 string) override;
private:
    bool germanUi_=false;
    bool motionEnabled_=false; // reduced-motion mode is the default
    int zoomIndex_=0; // 50% initial size fits smaller host displays
    std::mt19937 random_{std::random_device{}()};
};
} // namespace psychiatry::vst3
