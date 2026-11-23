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
    // UI-thread action. Changes the same nine real host-automatable macro IDs.
    // Never modifies the global THERAPY mix, Bypass, or the audio callback.
    void triggerLossOfControl();
private:
    std::mt19937 random_{std::random_device{}()};
};
} // namespace psychiatry::vst3
