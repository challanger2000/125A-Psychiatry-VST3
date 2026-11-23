#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "paranoia.hpp"
#include "depression.hpp"
#include "obsession.hpp"
#include "three_worlds.hpp"
#include "spectral_worlds.hpp"

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
    // P005 nine-world audio prototype; THERAPY applied once at the very end.
    // Ordering is preliminary, subject to musical audition and revision.
    psychiatry::Depression depression_{};
    psychiatry::Paranoia paranoia_{};
    psychiatry::Obsession obsession_{};
    psychiatry::Mania mania_{};
    psychiatry::Panic panic_{};
    psychiatry::Psychosis psychosis_{};
    psychiatry::Schizophrenia schizophrenia_{};
    psychiatry::Dissociation dissociation_{};
    psychiatry::Hallucination hallucination_{};
    psychiatry::Paranoia::Settings paranoiaSettings_{};
    float depressionAmount_=0.f;
    float obsessionAmount_=0.f;
    float maniaAmount_=0.f;
    float panicAmount_=0.f;
    float psychosisAmount_=0.f;
    float schizophreniaAmount_=0.f;
    float dissociationAmount_=0.f;
    float hallucinationAmount_=0.f;
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
