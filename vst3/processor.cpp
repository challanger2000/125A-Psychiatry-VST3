#include "processor.h"
#include "parameter_smoothing.hpp"
#include "ids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>

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
        tailSamples_=static_cast<uint32>(std::ceil(30.*setup.sampleRate))+kLatency;
        depression_.prepare(setup.sampleRate);
        paranoia_.prepare(setup.sampleRate);
        obsession_.prepare(setup.sampleRate);
        mania_.prepare(setup.sampleRate);
        panic_.prepare(setup.sampleRate);
        psychosis_.prepare(setup.sampleRate);
        schizophrenia_.prepare(setup.sampleRate);
        dissociation_.prepare(setup.sampleRate);
        hallucination_.prepare(setup.sampleRate);
        delirium_.prepare(setup.sampleRate);catatonia_.prepare(setup.sampleRate);amnesia_.prepare(setup.sampleRate);
        depression_.setAmount(depressionAmount_);
        paranoia_.setSettings(paranoiaSettings_);
        obsession_.setAmount(obsessionAmount_);
        mania_.setAmount(maniaAmount_);
        panic_.setAmount(panicAmount_);
        psychosis_.setAmount(psychosisAmount_);
        schizophrenia_.setAmount(schizophreniaAmount_);
        dissociation_.setAmount(dissociationAmount_);
        hallucination_.setAmount(hallucinationAmount_);
        delirium_.setAmount(deliriumAmount_);catatonia_.setAmount(catatoniaAmount_);amnesia_.setAmount(amnesiaAmount_);
        for(auto& channel:original_)
            channel.assign(static_cast<std::size_t>(setup.maxSamplesPerBlock),0.f);
        wetStep_=static_cast<float>(1.-std::exp(-1./(0.012*setup.sampleRate)));
        resetTimeline();
        wetSmoothed_=1.f-therapyAmount_;
        prepared_=true;
    }
    return result;
}

tresult PLUGIN_API Processor::setActive(TBool state) {
    if(state && prepared_) {
        resetTimeline();
        depression_.reset();
        paranoia_.reset();
        obsession_.reset();
        mania_.reset();
        panic_.reset();
        psychosis_.reset();
        schizophrenia_.reset();
        dissociation_.reset();
        hallucination_.reset();
        delirium_.reset();catatonia_.reset();amnesia_.reset();
        wetSmoothed_=1.f-therapyAmount_;
    }
    return AudioEffect::setActive(state);
}

tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolic) {
    return symbolic==Vst::kSample32 ? kResultTrue : kResultFalse;
}

// Parameter queues follow the same fixed input-time latency as audio.
// A host event at sample t enters each stage when that stage receives
// sample t; the four stage mix envelopes have their own 24-sample delay.
void Processor::resetTimeline() noexcept {
    clock_=0;pending_={};
    for(auto& d:dryDelay_)d.reset();
    applyDSP(0,depressionAmount_);applyDSP(1,paranoiaSettings_.amount);
    applyDSP(2,schizophreniaAmount_);applyDSP(3,maniaAmount_);
    applyDSP(4,panicAmount_);applyDSP(5,dissociationAmount_);
    applyDSP(6,obsessionAmount_);applyDSP(7,hallucinationAmount_);
    applyDSP(8,psychosisAmount_);applyDSP(9,therapyAmount_);applyDSP(10,bypass_?1.f:0.f);
    applyDSP(11,deliriumAmount_);applyDSP(12,catatoniaAmount_);applyDSP(13,amnesiaAmount_);
}
void Processor::applyDSP(unsigned lane,float v) noexcept {
    switch(lane) {
        case 0: depression_.setAmount(v);break;
        case 1: {auto settings=paranoiaSettings_;settings.amount=v;paranoia_.setSettings(settings);break;}
        case 2: schizophrenia_.setAmount(v);break;
        case 3: mania_.setAmount(v);break;
        case 4: panic_.setAmount(v);break;
        case 5: dissociation_.setAmount(v);break;
        case 6: obsession_.setAmount(v);break;
        case 7: hallucination_.setAmount(v);break;
        case 8: psychosis_.setAmount(v);break;
        case 9: therapyDSP_=v;break;
        case 10: bypassDSP_=v>=0.5f;break;
        case 11: delirium_.setAmount(v);break;
        case 12: catatonia_.setAmount(v);break;
        case 13: amnesia_.setAmount(v);break;
    }
}
void Processor::apply(Vst::ParamID id,Vst::ParamValue value) noexcept {
    const float v=static_cast<float>(normalized(value));
    unsigned lane=0;
    switch(id) {
        case kDepression: depressionAmount_=v;lane=0;break;
        case kParanoia: paranoiaSettings_.amount=v;lane=1;break;
        case kSchizophrenia: schizophreniaAmount_=v;lane=2;break;
        case kMania: maniaAmount_=v;lane=3;break;
        case kPanic: panicAmount_=v;lane=4;break;
        case kDissociation: dissociationAmount_=v;lane=5;break;
        case kObsession: obsessionAmount_=v;lane=6;break;
        case kHallucination: hallucinationAmount_=v;lane=7;break;
        case kPsychosis: psychosisAmount_=v;lane=8;break;
        case kTherapy: therapyAmount_=v;lane=9;break;
        case kBypass: bypass_=v>=0.5f;lane=10;break;
        case kDelirium: deliriumAmount_=v;lane=11;break;
        case kCatatonia: catatoniaAmount_=v;lane=12;break;
        case kAmnesia: amnesiaAmount_=v;lane=13;break;
        default: return;
    }
    constexpr std::uint64_t D=psychiatry::detail::kLinearPhaseLatency;
    constexpr std::array<std::uint64_t,14> offsets{0,3*D,D,D,2*D,3*D,3*D,3*D,3*D,4*D,4*D,4*D,4*D,4*D};
    auto& q=pending_[lane];
    const auto due=clock_+offsets[lane];
    if(q.count && q.items[(q.head+q.count-1)%q.items.size()].due==due) {
        q.items[(q.head+q.count-1)%q.items.size()].value=v;
    } else {
        // At most one change per sample per lane, within kLatency samples.
        // Capacity kLatency+1 includes the event due at the current sample.
        if(q.count<q.items.size()) {
            q.items[(q.head+q.count)%q.items.size()]={due,v};
            ++q.count;
        }
    }
}
void Processor::dispatchPending() noexcept {
    for(unsigned lane=0;lane<pending_.size();++lane) {
        auto& q=pending_[lane];
        while(q.count && q.items[q.head].due<=clock_) {
            applyDSP(lane,q.items[q.head].value);
            q.head=(q.head+1)%q.items.size();--q.count;
        }
    }
}
std::uint64_t Processor::nextDue() const noexcept {
    auto due=std::numeric_limits<std::uint64_t>::max();
    for(const auto& q:pending_)if(q.count)due=std::min(due,q.items[q.head].due);
    return due;
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
            for(int32 i=0;i<size;++i)original_[c][static_cast<std::size_t>(i)]=dryDelay_[c].process(from[i]);
            if(from!=to) std::memmove(to,from,static_cast<std::size_t>(size)*sizeof(float));
            ptrs[c]=to;
        }
        { // Keep wet and dry timelines continuous even during native bypass.
            // Fixed order yields designed interaction; individual macro
            // processors each return exact dry at zero amount.
            depression_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                                static_cast<std::size_t>(size));
            schizophrenia_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                                    static_cast<std::size_t>(size));
            mania_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                           static_cast<std::size_t>(size));
            panic_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                           static_cast<std::size_t>(size));
            paranoia_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                              static_cast<std::size_t>(size));
            dissociation_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                                  static_cast<std::size_t>(size));
            obsession_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                               static_cast<std::size_t>(size));
            hallucination_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                                   static_cast<std::size_t>(size));
            psychosis_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),
                               static_cast<std::size_t>(size));
            delirium_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),static_cast<std::size_t>(size));
            catatonia_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),static_cast<std::size_t>(size));
            amnesia_.process(ptrs.data(),static_cast<unsigned>(input.numChannels),static_cast<std::size_t>(size));
            for(int32 i=0;i<size;++i) {
                wetSmoothed_=psychiatry::detail::smoothToward(wetSmoothed_,1.f-therapyDSP_,wetStep_);
                const float w=std::clamp(wetSmoothed_,0.f,1.f);
                for(int32 ch=0;ch<input.numChannels;++ch) {
                    const auto c=static_cast<std::size_t>(ch);
                    const float dry=original_[c][static_cast<std::size_t>(i)];
                    const float wet=ptrs[c][i];
                    // Exact endpoints need no subtract/add round trip.
                    if(bypassDSP_||w==0.f)ptrs[c][i]=dry;
                    else if(w<1.f)ptrs[c][i]=dry+w*(wet-dry);
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
        dispatchPending();
        if(pos>=data.numSamples) break;
        int32 next=data.numSamples;
        const auto due=nextDue();
        if(due!=std::numeric_limits<std::uint64_t>::max()&&due>clock_&&due-clock_<static_cast<std::uint64_t>(next-pos))
            next=pos+static_cast<int32>(due-clock_);
        for(int32 n=0;n<count;++n) {
            const Lane& lane=lanes[static_cast<std::size_t>(n)];
            if(lane.index<lane.count) next=std::min(next,lane.nextOffset);
        }
        if(next<=pos) {
            if(!changed) return kResultFalse;
            continue;
        }
        if(!processSpan(data,pos,next-pos)) return kResultFalse;
        clock_+=static_cast<std::uint64_t>(next-pos);
        pos=next;
    }
    return kResultOk;
}

tresult PLUGIN_API Processor::setState(IBStream* state) {
    if(!state) return kResultFalse;
    IBStreamer s(state,kLittleEndian);
    int32 magic=0,version=0,bypass=0;
    float d=0.f,p=0.f,schizo=0.f,m=0.f,panic=0.f,
          diss=0.f,o=0.f,hall=0.f,psychosis=0.f,therapy=0.f;
    float delirium=0.f,catatonia=0.f,amnesia=0.f;
    if(!s.readInt32(magic)||!s.readInt32(version)||
       magic!=kStateMagic || version<1 || version>kStateVersion)
        return kResultFalse;
    if(version==1) {
        // P002: PARANOIA / THERAPY / Bypass
        if(!s.readFloat(p)||!s.readFloat(therapy)||!s.readInt32(bypass))
            return kResultFalse;
    } else if(version==2) {
        // P003: DEPRESSION / PARANOIA / OBSESSION / THERAPY / Bypass
        if(!s.readFloat(d)||!s.readFloat(p)||!s.readFloat(o)||
           !s.readFloat(therapy)||!s.readInt32(bypass))return kResultFalse;
    } else if(version==3) {
        // P004: DEPRESSION / PARANOIA / MANIA / PANIC / OBSESSION /
        // PSYCHOSIS / THERAPY / Bypass
        if(!s.readFloat(d)||!s.readFloat(p)||!s.readFloat(m)||
           !s.readFloat(panic)||!s.readFloat(o)||!s.readFloat(psychosis)||
           !s.readFloat(therapy)||!s.readInt32(bypass))return kResultFalse;
    } else {
        // P005: all nine macro values in stable ParamID order, THERAPY, Bypass.
        if(!s.readFloat(d)||!s.readFloat(p)||!s.readFloat(schizo)||
           !s.readFloat(m)||!s.readFloat(panic)||!s.readFloat(diss)||
           !s.readFloat(o)||!s.readFloat(hall)||!s.readFloat(psychosis)||
           !s.readFloat(therapy)||!s.readInt32(bypass))return kResultFalse;
    }
    if(version>=5&&(!s.readFloat(delirium)||!s.readFloat(catatonia)||!s.readFloat(amnesia)))return kResultFalse;
    const float vals[]{d,p,schizo,m,panic,diss,o,hall,psychosis,therapy,delirium,catatonia,amnesia};
    for(const auto value:vals)
        if(!std::isfinite(value))return kResultFalse;
    depressionAmount_=static_cast<float>(normalized(d));
    paranoiaSettings_.amount=static_cast<float>(normalized(p));
    schizophreniaAmount_=static_cast<float>(normalized(schizo));
    maniaAmount_=static_cast<float>(normalized(m));
    panicAmount_=static_cast<float>(normalized(panic));
    dissociationAmount_=static_cast<float>(normalized(diss));
    obsessionAmount_=static_cast<float>(normalized(o));
    hallucinationAmount_=static_cast<float>(normalized(hall));
    psychosisAmount_=static_cast<float>(normalized(psychosis));
    deliriumAmount_=static_cast<float>(normalized(delirium));
    catatoniaAmount_=static_cast<float>(normalized(catatonia));
    amnesiaAmount_=static_cast<float>(normalized(amnesia));
    therapyAmount_=static_cast<float>(normalized(therapy));
    bypass_=(bypass!=0);
    depression_.setAmount(depressionAmount_);
    paranoia_.setSettings(paranoiaSettings_);
    schizophrenia_.setAmount(schizophreniaAmount_);
    mania_.setAmount(maniaAmount_);
    panic_.setAmount(panicAmount_);
    dissociation_.setAmount(dissociationAmount_);
    obsession_.setAmount(obsessionAmount_);
    hallucination_.setAmount(hallucinationAmount_);
    psychosis_.setAmount(psychosisAmount_);
    delirium_.setAmount(deliriumAmount_);catatonia_.setAmount(catatoniaAmount_);amnesia_.setAmount(amnesiaAmount_);
    pending_={};
    therapyDSP_=therapyAmount_;bypassDSP_=bypass_;
    wetSmoothed_=1.f-therapyAmount_;
    return kResultOk;
}

tresult PLUGIN_API Processor::getState(IBStream* state) {
    if(!state)return kResultFalse;
    IBStreamer s(state,kLittleEndian);
    if(!s.writeInt32(kStateMagic)||!s.writeInt32(kStateVersion)||
       !s.writeFloat(depressionAmount_)||
       !s.writeFloat(paranoiaSettings_.amount)||
       !s.writeFloat(schizophreniaAmount_)||
       !s.writeFloat(maniaAmount_)||
       !s.writeFloat(panicAmount_)||
       !s.writeFloat(dissociationAmount_)||
       !s.writeFloat(obsessionAmount_)||
       !s.writeFloat(hallucinationAmount_)||
       !s.writeFloat(psychosisAmount_)||
       !s.writeFloat(therapyAmount_)||
       !s.writeInt32(bypass_?1:0)||!s.writeFloat(deliriumAmount_)||
       !s.writeFloat(catatoniaAmount_)||!s.writeFloat(amnesiaAmount_))return kResultFalse;
    return kResultOk;
}
} // namespace psychiatry::vst3
