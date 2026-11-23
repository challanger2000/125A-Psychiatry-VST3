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
    lane.nextOffset=std::clamp(offset,0,blockFrames);
    lane.nextValue=value;
    return true;
}
} // namespace

Processor::Processor() {
    setControllerClass(kControllerCID);
    paranoiaSettings_.amount=0.f;
    // Internal PARANOIA mixer is kept fully wet. THERAPY is a single final mix.
    paranoiaSettings_.therapy=0.f;
    paranoiaSettings_.trigger=psychiatry::Paranoia::Trigger::Hybrid;
    paranoiaSettings_.pattern=psychiatry::Paranoia::Pattern::Every;
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
        depression_.prepare(setup.sampleRate);
        paranoia_.prepare(setup.sampleRate);
        obsession_.prepare(setup.sampleRate);
        mania_.prepare(setup.sampleRate);
        panic_.prepare(setup.sampleRate);
        psychosis_.prepare(setup.sampleRate);
        depression_.setAmount(depressionAmount_);
        paranoia_.setSettings(paranoiaSettings_);
        obsession_.setAmount(obsessionAmount_);
        mania_.setAmount(maniaAmount_);
        panic_.setAmount(panicAmount_);
        psychosis_.setAmount(psychosisAmount_);
        for(auto& channel:original_)
            channel.assign(static_cast<std::size_t>(setup.maxSamplesPerBlock),0.f);
        wetStep_=static_cast<float>(1.-std::exp(-1./(0.012*setup.sampleRate)));
        wetSmoothed_=1.f-therapyAmount_;
        prepared_=true;
    }
    return result;
}

tresult PLUGIN_API Processor::setActive(TBool state) {
    if(state && prepared_) {
        depression_.reset();
        paranoia_.reset();
        obsession_.reset();
        mania_.reset();
        panic_.reset();
        psychosis_.reset();
        wetSmoothed_=1.f-therapyAmount_;
    }
    return AudioEffect::setActive(state);
}

tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolic) {
    return symbolic==Vst::kSample32 ? kResultTrue : kResultFalse;
}

void Processor::apply(Vst::ParamID id,Vst::ParamValue value) noexcept {
    const float v=static_cast<float>(normalized(value));
    if(id==kDepression) {
        depressionAmount_=v;
        depression_.setAmount(v);
    } else if(id==kParanoia) {
        paranoiaSettings_.amount=v;
        paranoia_.setSettings(paranoiaSettings_);
    } else if(id==kObsession) {
        obsessionAmount_=v;
        obsession_.setAmount(v);
    } else if(id==kMania) {
        maniaAmount_=v;
        mania_.setAmount(v);
    } else if(id==kPanic) {
        panicAmount_=v;
        panic_.setAmount(v);
    } else if(id==kPsychosis) {
        psychosisAmount_=v;
        psychosis_.setAmount(v);
    } else if(id==kTherapy) {
        therapyAmount_=v;
    } else if(id==kBypass) {
        bypass_=v>=0.5f;
    }
}

bool Processor::processSpan(Vst::ProcessData& data,
                            int32 offset,int32 frames) noexcept {
    if(frames<=0) return true;
    if(data.numInputs<1||data.numOutputs<1||!data.inputs||!data.outputs) return false;
    auto& input=data.inputs[0];
    auto& output=data.outputs[0];
    if(!input.channelBuffers32||!output.channelBuffers32||
       (input.numChannels!=1 && input.numChannels!=2)||
       input.numChannels!=output.numChannels) return false;

    std::array<float*,2> src{},dst{};
    for(int32 ch=0;ch<input.numChannels;++ch) {
        src[static_cast<std::size_t>(ch)]=input.channelBuffers32[ch];
        dst[static_cast<std::size_t>(ch)]=output.channelBuffers32[ch];
        if(!src[static_cast<std::size_t>(ch)]||!dst[static_cast<std::size_t>(ch)])
            return false;
    }
    if(original_[0].empty()) return false;

    // Chunk in setupProcessing-sized slices. All scratch storage is reserved
    // before audio starts; processSpan and DSP effects never resize or lock.
    for(int32 pos=0;pos<frames;) {
        const auto size=static_cast<int32>(std::min<std::size_t>(
            static_cast<std::size_t>(frames-pos),original_[0].size()));
        std::array<float*,2> ptrs{};
        for(int32 ch=0;ch<input.numChannels;++ch) {
            const auto c=static_cast<std::size_t>(ch);
            const auto* from=src[c]+offset+pos;
            auto* to=dst[c]+offset+pos;
            std::memcpy(original_[c].data(),from,static_cast<std::size_t>(size)*sizeof(float));
            if(from!=to) std::memmove(to,from,static_cast<std::size_t>(size)*sizeof(float));
            ptrs[c]=to;
        }
        if(!bypass_) {
            // Fixed order yields designed interaction; individual macro
            // processors each return exact dry at zero amount.
            depression_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                                static_cast<std::size_t>(size));
            mania_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                           static_cast<std::size_t>(size));
            panic_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                           static_cast<std::size_t>(size));
            paranoia_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                              static_cast<std::size_t>(size));
            obsession_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                               static_cast<std::size_t>(size));
            psychosis_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                               static_cast<std::size_t>(size));
            for(int32 i=0;i<size;++i) {
                wetSmoothed_+=wetStep_*(1.f-therapyAmount_-wetSmoothed_);
                const float w=std::clamp(wetSmoothed_,0.f,1.f);
                for(int32 ch=0;ch<input.numChannels;++ch) {
                    const auto c=static_cast<std::size_t>(ch);
                    const float dry=original_[c][static_cast<std::size_t>(i)];
                    const float wet=ptrs[c][i];
                    ptrs[c][i]=dry+w*(wet-dry);
                }
            }
        }
        pos+=size;
    }
    output.silenceFlags=0; // Old buffers can yield audible reverb after silent input.
    return true;
}

tresult PLUGIN_API Processor::process(Vst::ProcessData& data) {
    if(data.numSamples<0) return kResultFalse;
    std::array<Lane,32> lanes{};
    int32 count=0;
    if(data.inputParameterChanges) {
        const int32 total=data.inputParameterChanges->getParameterCount();
        if(total>static_cast<int32>(lanes.size())) return kResultFalse;
        for(int32 k=0;k<total;++k) {
            auto* q=data.inputParameterChanges->getParameterData(k);
            if(!q||q->getPointCount()==0) continue;
            Lane lane;lane.queue=q;lane.count=q->getPointCount();
            if(!advance(lane,data.numSamples)) continue;
            lanes[static_cast<std::size_t>(count++)]=lane;
        }
    }
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
    float d=0.f,p=0.f,m=0.f,panic=0.f,o=0.f,psychosis=0.f,therapy=0.f;
    if(!s.readInt32(magic)||!s.readInt32(version)||
       magic!=kStateMagic || version<1 || version>kStateVersion)
        return kResultFalse;
    if(version==1) {
        // P002 state v1: PARANOIA, THERAPY, Bypass.
        if(!s.readFloat(p)||!s.readFloat(therapy)||!s.readInt32(bypass))
            return kResultFalse;
    } else if(version==2) {
        // P003 state v2: DEPRESSION, PARANOIA, OBSESSION, THERAPY, Bypass.
        if(!s.readFloat(d)||!s.readFloat(p)||!s.readFloat(o)||
           !s.readFloat(therapy)||!s.readInt32(bypass)) return kResultFalse;
    } else {
        // P004 state v3: 6 macros in stable ParamID order, THERAPY, Bypass.
        if(!s.readFloat(d)||!s.readFloat(p)||!s.readFloat(m)||
           !s.readFloat(panic)||!s.readFloat(o)||!s.readFloat(psychosis)||
           !s.readFloat(therapy)||!s.readInt32(bypass)) return kResultFalse;
    }
    if(!std::isfinite(d)||!std::isfinite(p)||!std::isfinite(m)||
       !std::isfinite(panic)||!std::isfinite(o)||!std::isfinite(psychosis)||
       !std::isfinite(therapy)) return kResultFalse;
    depressionAmount_=static_cast<float>(normalized(d));
    paranoiaSettings_.amount=static_cast<float>(normalized(p));
    maniaAmount_=static_cast<float>(normalized(m));
    panicAmount_=static_cast<float>(normalized(panic));
    obsessionAmount_=static_cast<float>(normalized(o));
    psychosisAmount_=static_cast<float>(normalized(psychosis));
    therapyAmount_=static_cast<float>(normalized(therapy));
    bypass_=(bypass!=0);
    depression_.setAmount(depressionAmount_);
    paranoia_.setSettings(paranoiaSettings_);
    mania_.setAmount(maniaAmount_);
    panic_.setAmount(panicAmount_);
    obsession_.setAmount(obsessionAmount_);
    psychosis_.setAmount(psychosisAmount_);
    wetSmoothed_=1.f-therapyAmount_;
    return kResultOk;
}

tresult PLUGIN_API Processor::getState(IBStream* state) {
    if(!state) return kResultFalse;
    IBStreamer s(state,kLittleEndian);
    if(!s.writeInt32(kStateMagic)||!s.writeInt32(kStateVersion)||
       !s.writeFloat(depressionAmount_)||
       !s.writeFloat(paranoiaSettings_.amount)||
       !s.writeFloat(maniaAmount_)||
       !s.writeFloat(panicAmount_)||
       !s.writeFloat(obsessionAmount_)||
       !s.writeFloat(psychosisAmount_)||
       !s.writeFloat(therapyAmount_)||
       !s.writeInt32(bypass_ ? 1 : 0)) return kResultFalse;
    return kResultOk;
}
} // namespace psychiatry::vst3
