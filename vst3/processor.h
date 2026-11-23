#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "paranoia.hpp"
#include "depression.hpp"
#include "obsession.hpp"

#include <array>
#include <vector>

namespace psychiatry::vst3 {
class Processor final : public Steinberg::Vst::AudioEffect {
public:
    Processor();
    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(new Processor);
    }

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) override;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API setBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs,
            Steinberg::int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs,
            Steinberg::int32 numOuts) override;
    Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolic) override;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;

private:
    // P003 serial macro topology: oppressive timbre => paranoid reverse =>
    // repetitive fixation. Shared THERAPY mixed once at the end, not per macro.
    psychiatry::Depression depression_{};
    psychiatry::Paranoia paranoia_{};
    psychiatry::Obsession obsession_{};
    psychiatry::Paranoia::Settings paranoiaSettings_{};
    float depressionAmount_=0.f;
    float obsessionAmount_=0.f;
    float therapyAmount_=0.f;
    float wetSmoothed_=1.f;
    float wetStep_=0.001f;
    bool bypass_=false;
    bool prepared_=false;
    std::array<std::vector<float>,2> original_{};

    void apply(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) noexcept;
    bool processSpan(Steinberg::Vst::ProcessData& data,
                     Steinberg::int32 offset, Steinberg::int32 frames) noexcept;
};
} // namespace psychiatry::vst3
