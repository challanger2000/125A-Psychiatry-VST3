#include "processor.h"
#include "ids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

namespace psychiatry::vst3 {
using namespace Steinberg;

namespace {
double normalized(double v) noexcept {
    return std::isfinite(v) ? std::clamp(v, 0.0, 1.0) : 0.0;
}
struct Lane {
    Vst::IParamValueQueue* queue=nullptr;
    int32 index=0;
    int32 count=0;
    int32 nextOffset=0;
    Vst::ParamValue nextValue=0;
};
bool advance(Lane& lane, int32 blockFrames) noexcept {
    if(!lane.queue || lane.index>=lane.count) return false;
    int32 offset=0;
    Vst::ParamValue value=0;
    if(lane.queue->getPoint(lane.index,offset,value)!=kResultTrue) {
        lane.index=lane.count;
        return false;
    }
    // Host can deliver points at the block end for final state changes.
    lane.nextOffset=std::clamp(offset,0,blockFrames);
    lane.nextValue=value;
    return true;
}
} // namespace

Processor::Processor() {
    setControllerClass(kControllerCID);
    settings_.amount=0.f;
    settings_.therapy=0.f;
    settings_.trigger=psychiatry::Paranoia::Trigger::Hybrid;
    settings_.pattern=psychiatry::Paranoia::Pattern::Every;
}

tresult PLUGIN_API Processor::initialize(FUnknown* context) {
    auto result=AudioEffect::initialize(context);
    if(result!=kResultOk) return result;
    addAudioInput(STR16("Audio In"),Vst::SpeakerArr::kStereo);
    addAudioOutput(STR16("Audio Out"),Vst::SpeakerArr::kStereo);
    return kResultOk;
}

tresult PLUGIN_API Processor::setBusArrangements(Vst::SpeakerArrangement* inputs,
        int32 numIns,Vst::SpeakerArrangement* outputs,int32 numOuts) {
    if(numIns!=1 || numOuts!=1 || !inputs || !outputs ||
        inputs[0]!=outputs[0] ||
        (inputs[0]!=Vst::SpeakerArr::kMono && inputs[0]!=Vst::SpeakerArr::kStereo))
        return kResultFalse;
    auto* input=getAudioInput(0);
    auto* output=getAudioOutput(0);
    if(!input || !output) return kResultFalse;
    input->setArrangement(inputs[0]);
    output->setArrangement(outputs[0]);
    return kResultTrue;
}

tresult PLUGIN_API Processor::setupProcessing(Vst::ProcessSetup& setup) {
    if(!std::isfinite(setup.sampleRate) || setup.sampleRate<1000.0 ||
       setup.sampleRate>2000000.0 || setup.maxSamplesPerBlock<=0)
        return kResultFalse;
    const auto result=AudioEffect::setupProcessing(setup);
    if(result==kResultOk) {
        // No memory allocated by process(). setupProcessing is not an audio callback.
        paranoia_.prepare(setup.sampleRate);
        paranoia_.setSettings(settings_);
        prepared_=true;
    }
    return result;
}

tresult PLUGIN_API Processor::setActive(TBool state) {
    if(state && prepared_) paranoia_.reset();
    return AudioEffect::setActive(state);
}

tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolic) {
    // Explicitly 32-bit, until the 64-bit host path receives audio tests.
    return symbolic==Vst::kSample32 ? kResultTrue : kResultFalse;
}

void Processor::apply(Vst::ParamID id,Vst::ParamValue value) noexcept {
    const auto v=normalized(value);
    if(id==kParanoia) {
        settings_.amount=static_cast<float>(v);
        paranoia_.setSettings(settings_);
    } else if(id==kTherapy) {
        settings_.therapy=static_cast<float>(v);
        paranoia_.setSettings(settings_);
    } else if(id==kBypass) {
        bypass_=v>=0.5;
    }
}

bool Processor::processSpan(Vst::ProcessData& data,
                            int32 offset,int32 frames) noexcept {
    if(frames<=0) return true;
    if(data.numInputs<1||data.numOutputs<1||!data.inputs||!data.outputs) return false;
    auto& input=data.inputs[0];
    auto& output=data.outputs[0];
    if(!input.channelBuffers32||!output.channelBuffers32 ||
       (input.numChannels!=1 && input.numChannels!=2) ||
       input.numChannels!=output.numChannels) return false;

    std::array<float*,2> ptrs{};
    for(int32 ch=0; ch<input.numChannels; ++ch) {
        float* src=input.channelBuffers32[ch];
        float* dst=output.channelBuffers32[ch];
        if(!src||!dst) return false;
        src+=offset;
        dst+=offset;
        if(src!=dst) std::memmove(dst,src,static_cast<std::size_t>(frames)*sizeof(float));
        ptrs[static_cast<std::size_t>(ch)]=dst;
    }
    output.silenceFlags=0; // Reverb/reverse tails may survive flagged-silent input.
    if(!bypass_) paranoia_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                                   static_cast<std::size_t>(frames));
    return true;
}

tresult PLUGIN_API Processor::process(Vst::ProcessData& data) {
    if(data.numSamples<0) return kResultFalse;

    // Host automation can change at multiple sample offsets in one audio block.
    // Bounded stack-only lanes; no vector growth/heap allocation in callback.
    std::array<Lane,32> lanes{};
    int32 count=0;
    if(data.inputParameterChanges) {
        const int32 total=data.inputParameterChanges->getParameterCount();
        if(total>static_cast<int32>(lanes.size())) return kResultFalse; // fail visibly, never silently drop automation
        for(int32 k=0;k<total;++k) {
            auto* q=data.inputParameterChanges->getParameterData(k);
            if(!q||q->getPointCount()==0) continue;
            Lane lane;
            lane.queue=q;
            lane.count=q->getPointCount();
            if(!advance(lane,data.numSamples)) continue;
            lanes[static_cast<std::size_t>(count++)]=lane;
        }
    }

    // Apply all events at offset zero, then process until each next event.
    // Disabled processing still applies automation so project state stays current.
    int32 pos=0;
    for(;;) {
        bool changed=false;
        for(int32 n=0;n<count;++n) {
            Lane& lane=lanes[static_cast<std::size_t>(n)];
            while(lane.index<lane.count && lane.nextOffset<=pos) {
                apply(lane.queue->getParameterId(),lane.nextValue);
                ++lane.index;
                advance(lane,data.numSamples);
                changed=true;
            }
        }
        if(pos>=data.numSamples) break;
        int32 next=data.numSamples;
        for(int32 n=0;n<count;++n) {
            const Lane& lane=lanes[static_cast<std::size_t>(n)];
            if(lane.index<lane.count) next=std::min(next,lane.nextOffset);
        }
        if(next<=pos) {
            if(!changed) return kResultFalse;
            continue;
        }
        if(!processSpan(data,pos,next-pos)) return kResultFalse;
        pos=next;
    }
    return kResultOk;
}

tresult PLUGIN_API Processor::setState(IBStream* state) {
    if(!state) return kResultFalse;
    IBStreamer s(state,kLittleEndian);
    int32 magic=0,version=0,bypass=0;
    float amount=0.f,therapy=0.f;
    if(!s.readInt32(magic)||!s.readInt32(version)||magic!=kStateMagic ||
       version!=kStateVersion||!s.readFloat(amount)||!s.readFloat(therapy)||
       !s.readInt32(bypass)||!std::isfinite(amount)||!std::isfinite(therapy))
        return kResultFalse;
    settings_.amount=static_cast<float>(normalized(amount));
    settings_.therapy=static_cast<float>(normalized(therapy));
    bypass_=(bypass!=0);
    paranoia_.setSettings(settings_);
    return kResultOk;
}

tresult PLUGIN_API Processor::getState(IBStream* state) {
    if(!state) return kResultFalse;
    IBStreamer s(state,kLittleEndian);
    if(!s.writeInt32(kStateMagic)||!s.writeInt32(kStateVersion)||
       !s.writeFloat(settings_.amount)||!s.writeFloat(settings_.therapy)||
       !s.writeInt32(bypass_ ? 1 : 0)) return kResultFalse;
    return kResultOk;
}
} // namespace psychiatry::vst3
