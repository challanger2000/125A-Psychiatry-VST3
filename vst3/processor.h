#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "paranoia.hpp"
#include "depression.hpp"
#include "obsession.hpp"
#include "three_worlds.hpp"
#include "spectral_worlds.hpp"
#include "creative_worlds.hpp"

#include <array>
#include <vector>
#include <mutex>
#include "state_snapshot.h"

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
    Steinberg::tresult PLUGIN_API setProcessing(Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API setBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs,
            Steinberg::int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs,
            Steinberg::int32 numOuts) override;
    static constexpr Steinberg::uint32 kLatency=4*psychiatry::detail::kLinearPhaseLatency;
    Steinberg::uint32 PLUGIN_API getLatencySamples() override { return kLatency; }
    // Fixed conservative tail allowance: macros can be automated during playback,
    // so a static upper bound avoids a UI-thread restart for each edit.
    Steinberg::uint32 PLUGIN_API getTailSamples() override { return tailSamples_; }
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
    psychiatry::Delirium delirium_{};
    psychiatry::Catatonia catatonia_{};
    psychiatry::Amnesia amnesia_{};
    psychiatry::Paranoia::Settings paranoiaSettings_{};
    float depressionAmount_=0.f;
    float obsessionAmount_=0.f;
    float maniaAmount_=0.f;
    float panicAmount_=0.f;
    float psychosisAmount_=0.f;
    float schizophreniaAmount_=0.f;
    float dissociationAmount_=0.f;
    float hallucinationAmount_=0.f;
    float deliriumAmount_=0.f,catatoniaAmount_=0.f,amnesiaAmount_=0.f;
    float therapyAmount_=0.f;
    float wetSmoothed_=1.f;
    float wetStep_=0.001f;
    bool bypass_=false;
    bool bypassDSP_=false;
    float bypassMix_=0.f;
    std::uint32_t bypassRampFrames_=1,bypassRemaining_=0;
    StateMailbox audioState_,recallState_;
    std::mutex controlStateMutex_; // host state methods only; never process()
    StateFrame latestRecall_{},lastPublished_{};
    std::uint64_t appliedGeneration_=0;
    StateFrame captureState() const noexcept;
    void publishState() noexcept;
    void consumeRecall(bool live=false) noexcept;
    float therapyDSP_=0.f;
    bool prepared_=false;
    bool processing_=false; // serialized host processing lifecycle
    Steinberg::uint32 tailSamples_=30*48000+kLatency;
    std::array<std::vector<float>,2> original_{};
    std::array<psychiatry::detail::FixedDelay<kLatency>,2> dryDelay_{};
    struct Point {Steinberg::int32 offset=0;double value=0.;};
    struct Lane {
        std::vector<Point> points;
        std::vector<float> curve;
        std::array<float,kLatency> delay{};
        unsigned cursor=0;
        std::size_t count=0,index=0;
        Steinberg::int32 previousOffset=-1;
        double previousValue=0.;
        float input=0.f;
    };
    std::array<Lane,14> lanes_{};
    void resetControls() noexcept;
    void renderCurves(Steinberg::int32 start,Steinberg::int32 frames) noexcept;
    void storeInput(unsigned lane,float v) noexcept;
    void applyDSP(unsigned lane,float value) noexcept;
    void resetTimeline() noexcept;

    bool validateBuffers(const Steinberg::Vst::ProcessData& data) noexcept;
    void processSpan(Steinberg::Vst::ProcessData& data,
                     Steinberg::int32 offset, Steinberg::int32 frames,
                     Steinberg::uint64& silent) noexcept;
};
} // namespace psychiatry::vst3
