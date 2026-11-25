#include "processor.h"
#include "parameter_smoothing.hpp"
#include "ids.h"

#include "state_stream.h"
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
constexpr std::array<Vst::ParamID,14> kLaneIds{
    kDepression,kParanoia,kSchizophrenia,kMania,kPanic,kDissociation,kObsession,
    kHallucination,kPsychosis,kTherapy,kBypass,kDelirium,kCatatonia,kAmnesia};
constexpr unsigned D=psychiatry::detail::kLinearPhaseLatency;
constexpr std::array<unsigned,14> kOffsets{0,3*D,D,D,2*D,3*D,3*D,3*D,3*D,4*D,4*D,4*D,4*D,4*D};
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
        consumeRecall();
        bypassRampFrames_=static_cast<std::uint32_t>(std::max(1.,std::ceil(.005*setup.sampleRate)));
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
        for(auto& lane:lanes_){
            lane.points.resize(static_cast<std::size_t>(setup.maxSamplesPerBlock));
            lane.curve.resize(static_cast<std::size_t>(setup.maxSamplesPerBlock));
        }
        wetStep_=static_cast<float>(1.-std::exp(-1./(0.012*setup.sampleRate)));
        resetTimeline();
        wetSmoothed_=1.f-therapyAmount_;
        prepared_=true;
    }
    return result;
}

tresult PLUGIN_API Processor::setActive(TBool state) {
    if(state && prepared_) {
        consumeRecall();
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

tresult PLUGIN_API Processor::setProcessing(TBool) {
    // VST3 processing transitions must not allocate, lock or reset DSP history.
    return kResultOk;
}

tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolic) {
    return symbolic==Vst::kSample32 ? kResultTrue : kResultFalse;
}

// Parameter queues follow the same fixed input-time latency as audio.
// A host event at sample t enters each stage when that stage receives
// sample t; the four stage mix envelopes have their own 24-sample delay.
void Processor::resetTimeline() noexcept {
    resetControls();
    for(auto& d:dryDelay_)d.reset();
    applyDSP(0,depressionAmount_);applyDSP(1,paranoiaSettings_.amount);
    applyDSP(2,schizophreniaAmount_);applyDSP(3,maniaAmount_);
    applyDSP(4,panicAmount_);applyDSP(5,dissociationAmount_);
    applyDSP(6,obsessionAmount_);applyDSP(7,hallucinationAmount_);
    applyDSP(8,psychosisAmount_);applyDSP(9,therapyAmount_);applyDSP(10,bypass_?1.f:0.f);
    applyDSP(11,deliriumAmount_);applyDSP(12,catatoniaAmount_);applyDSP(13,amnesiaAmount_);
    bypassMix_=bypass_?1.f:0.f;bypassRemaining_=0;
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
        case 10: {const bool next=v>=.5f;if(next!=bypassDSP_){bypassDSP_=next;bypassRemaining_=bypassRampFrames_;}break;}
        case 11: delirium_.setAmount(v);break;
        case 12: catatonia_.setAmount(v);break;
        case 13: amnesia_.setAmount(v);break;
    }
}
void Processor::storeInput(unsigned lane,float v) noexcept {
    switch(lane){
        case 0:depressionAmount_=v;break;case 1:paranoiaSettings_.amount=v;break;
        case 2:schizophreniaAmount_=v;break;case 3:maniaAmount_=v;break;
        case 4:panicAmount_=v;break;case 5:dissociationAmount_=v;break;
        case 6:obsessionAmount_=v;break;case 7:hallucinationAmount_=v;break;
        case 8:psychosisAmount_=v;break;case 9:therapyAmount_=v;break;
        case 10:bypass_=v>=.5f;break;case 11:deliriumAmount_=v;break;
        case 12:catatoniaAmount_=v;break;case 13:amnesiaAmount_=v;break;
    }
}
void Processor::resetControls() noexcept {
    const auto values=captureState().values;
    for(unsigned i=0;i<lanes_.size();++i){
        auto& lane=lanes_[i];lane.input=values[i];lane.delay.fill(values[i]);lane.cursor=0;
    }
}
void Processor::renderCurves(int32 start,int32 frames) noexcept {
    // Delays are independent per lane. This preserves each stage's exact
    // input-time automation with a single block traversal per DSP stage.
    for(unsigned i=0;i<lanes_.size();++i){
        auto& lane=lanes_[i];const auto delay=kOffsets[i];
        for(int32 n=0;n<frames;++n){
            const int32 at=start+n;
            if(lane.index<lane.count){
                const auto& point=lane.points[lane.index];
                if(at==point.offset){
                    lane.input=static_cast<float>(point.value);
                    lane.previousOffset=point.offset;lane.previousValue=point.value;++lane.index;
                }else if(i!=10&&lane.previousValue!=point.value){
                    const double t=double(at-lane.previousOffset)/double(point.offset-lane.previousOffset);
                    lane.input=static_cast<float>(lane.previousValue+t*(point.value-lane.previousValue));
                }
            }
            if(delay){
                lane.curve[n]=lane.delay[lane.cursor];
                lane.delay[lane.cursor]=lane.input;
                if(++lane.cursor==delay)lane.cursor=0;
            }else lane.curve[n]=lane.input;
        }
        storeInput(i,lane.input);
    }
}
bool Processor::validateBuffers(const Vst::ProcessData& data) noexcept {
    if(!data.numSamples)return true;
    const auto* out=getAudioOutput(0);const auto* in=getAudioInput(0);
    if(!out||!in||!prepared_)return false;
    if(!out->isActive())return true;
    if(data.numOutputs<1||!data.outputs)return false;
    const auto& output=data.outputs[0];
    if(!output.channelBuffers32||(output.numChannels!=1&&output.numChannels!=2))return false;
    if(in->isActive()&&(data.numInputs<1||!data.inputs||!data.inputs[0].channelBuffers32||
        data.inputs[0].numChannels!=output.numChannels))return false;
    for(int32 ch=0;ch<output.numChannels;++ch)
        if(!output.channelBuffers32[ch]||(in->isActive()&&!data.inputs[0].channelBuffers32[ch]))return false;
    return true;
}
void Processor::processSpan(Vst::ProcessData& data,
                            int32 offset,int32 frames,uint64& silent) noexcept {
    if(!getAudioOutput(0)->isActive())return;
    auto& output=data.outputs[0];
    const bool inputActive=getAudioInput(0)->isActive();
    std::array<float*,2> src{},dst{};
    for(int32 ch=0;ch<output.numChannels;++ch){
        if(inputActive)src[ch]=data.inputs[0].channelBuffers32[ch];
        dst[ch]=output.channelBuffers32[ch];
    }
    // Chunk in setupProcessing-sized slices. All scratch storage is reserved
    // before audio starts; processSpan and DSP effects never resize or lock.
    {
        constexpr int32 pos=0;
        const auto size=frames;
        std::array<float*,2> ptrs{};
        for(int32 ch=0;ch<output.numChannels;++ch) {
            const auto c=static_cast<std::size_t>(ch);
            const auto* from=inputActive?src[c]+offset+pos:nullptr;
            auto* to=dst[c]+offset+pos;
            for(int32 i=0;i<size;++i){
                const float raw=from?from[i]:0.f;
                const float sample=std::isfinite(raw)?raw:0.f;
                original_[c][static_cast<std::size_t>(i)]=dryDelay_[c].process(sample);
                to[i]=sample;
            }
            ptrs[c]=to;
        }
        { // Keep wet and dry timelines continuous even during native bypass.
            // Fixed order yields designed interaction; individual macro
            // processors each return exact dry at zero amount.
            depression_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),
                                static_cast<std::size_t>(size),lanes_[0].curve.data());
            schizophrenia_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),
                                    static_cast<std::size_t>(size),lanes_[2].curve.data());
            mania_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),
                           static_cast<std::size_t>(size),lanes_[3].curve.data());
            panic_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),
                           static_cast<std::size_t>(size),lanes_[4].curve.data());
            paranoia_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),
                              static_cast<std::size_t>(size),lanes_[1].curve.data());
            dissociation_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),
                                  static_cast<std::size_t>(size),lanes_[5].curve.data());
            obsession_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),
                               static_cast<std::size_t>(size),lanes_[6].curve.data());
            hallucination_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),
                                   static_cast<std::size_t>(size),lanes_[7].curve.data());
            psychosis_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),
                               static_cast<std::size_t>(size),lanes_[8].curve.data());
            delirium_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),static_cast<std::size_t>(size),lanes_[11].curve.data());
            catatonia_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),static_cast<std::size_t>(size),lanes_[12].curve.data());
            amnesia_.process(ptrs.data(),static_cast<unsigned>(output.numChannels),static_cast<std::size_t>(size),lanes_[13].curve.data());
            for(int32 i=0;i<size;++i) {
                therapyDSP_=lanes_[9].curve[i];
                applyDSP(10,lanes_[10].curve[i]);
                wetSmoothed_=psychiatry::detail::smoothToward(wetSmoothed_,1.f-therapyDSP_,wetStep_);
                const float target=bypassDSP_?1.f:0.f;
                if(bypassRemaining_){
                    if(bypassRemaining_==1)bypassMix_=target;
                    else bypassMix_+=(target-bypassMix_)/static_cast<float>(bypassRemaining_);
                    --bypassRemaining_;
                }
                const float w=std::clamp(wetSmoothed_,0.f,1.f)*(1.f-bypassMix_);
                for(int32 ch=0;ch<output.numChannels;++ch) {
                    const auto c=static_cast<std::size_t>(ch);
                    const float dry=original_[c][static_cast<std::size_t>(i)];
                    const float wet=ptrs[c][i];
                    // Exact endpoints need no subtract/add round trip.
                    if(w==0.f)ptrs[c][i]=dry;
                    else if(w<1.f)ptrs[c][i]=dry+w*(wet-dry);
                    if(ptrs[c][i]!=0.f)silent&=~(uint64{1}<<c);
                }
            }
        }
    }
}

tresult PLUGIN_API Processor::process(Vst::ProcessData& data) {
    if(data.numSamples<0||data.symbolicSampleSize!=Vst::kSample32||!validateBuffers(data))return kResultFalse;
    // Copy and validate every point before touching recall, DSP, clock or host buffers.
    // All scratch storage is prepared off audio. No queue is consulted twice.
    for(auto& lane:lanes_)lane.count=lane.index=0;
    if(data.inputParameterChanges){
        const int32 total=data.inputParameterChanges->getParameterCount();
        if(total<0||total>32)return kResultFalse;
        std::array<bool,14> seen{};
        for(int32 j=0;j<total;++j){
            auto* q=data.inputParameterChanges->getParameterData(j);
            if(!q)continue;
            const auto id=q->getParameterId();
            const auto it=std::find(kLaneIds.begin(),kLaneIds.end(),id);
            if(it==kLaneIds.end())continue;
            const auto i=static_cast<unsigned>(it-kLaneIds.begin());
            auto& lane=lanes_[i];const auto n=q->getPointCount();
            if(n<0||n>std::max(1,data.numSamples)||seen[i])return kResultFalse;
            seen[i]=true;
            if(!n)continue;
            if(static_cast<std::size_t>(n)>lane.points.size())return kResultFalse;
            int32 previous=-1;
            for(int32 k=0;k<n;++k){
                int32 at=0;double value=0.;
                if(q->getPoint(k,at,value)!=kResultOk||at<=previous||at<0||
                    at>std::max(0,data.numSamples-1))return kResultFalse;
                lane.points[k]={at,normalized(value)};previous=at;
            }
            lane.count=static_cast<std::size_t>(n);
        }
    }
    consumeRecall(data.numSamples>0&&prepared_);
    const auto values=captureState().values;
    for(unsigned i=0;i<lanes_.size();++i){
        auto& lane=lanes_[i];lane.input=values[i];lane.previousOffset=-1;lane.previousValue=values[i];
        if(!data.numSamples&&lane.count){
            lane.input=static_cast<float>(lane.points[0].value);
            storeInput(i,lane.input);
            // Zero-offset stage has no audio delay. Other targets enter at
            // their own latency offset when the next audio sample arrives.
            if(kOffsets[i]==0&&prepared_)applyDSP(i,lane.input);
        }
    }
    uint64 silent=~uint64{0};
    for(int32 pos=0;pos<data.numSamples;){
        const auto n=static_cast<int32>(std::min<std::size_t>(
            static_cast<std::size_t>(data.numSamples-pos),original_[0].size()));
        renderCurves(pos,n);
        processSpan(data,pos,n,silent);
        pos+=n;
    }
    if(data.numSamples>0&&getAudioOutput(0)->isActive()){
        auto& out=data.outputs[0];out.silenceFlags=silent&((uint64{1}<<out.numChannels)-1);
    }
    publishState();return kResultOk;
}
tresult PLUGIN_API Processor::setState(IBStream* state) {
    if(!state) return kResultFalse;
    StateStream s(state);
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
    StateFrame frame;
    const float wire[]{d,p,schizo,m,panic,diss,o,hall,psychosis,therapy,bypass?1.f:0.f,delirium,catatonia,amnesia};
    for(std::size_t i=0;i<frame.values.size();++i)frame.values[i]=static_cast<float>(normalized(wire[i]));
    {
        std::lock_guard<std::mutex> lock(controlStateMutex_);
        frame.generation=latestRecall_.generation+1;
        latestRecall_=frame;
        recallState_.publish(frame);
    }
    return kResultOk;
}

StateFrame Processor::captureState() const noexcept {
    return {{depressionAmount_,paranoiaSettings_.amount,schizophreniaAmount_,maniaAmount_,
        panicAmount_,dissociationAmount_,obsessionAmount_,hallucinationAmount_,psychosisAmount_,
        therapyAmount_,bypass_?1.f:0.f,deliriumAmount_,catatoniaAmount_,amnesiaAmount_},appliedGeneration_};
}
void Processor::publishState() noexcept {
    const auto frame=captureState();
    if(frame.generation!=lastPublished_.generation||frame.values!=lastPublished_.values){
        audioState_.publish(frame);lastPublished_=frame;
    }
}
void Processor::consumeRecall(bool live) noexcept {
    StateFrame frame;
    if(!recallState_.tryRead(frame)||frame.generation<=appliedGeneration_)return;
    const auto& v=frame.values;
    depressionAmount_=v[0];paranoiaSettings_.amount=v[1];schizophreniaAmount_=v[2];
    maniaAmount_=v[3];panicAmount_=v[4];dissociationAmount_=v[5];
    obsessionAmount_=v[6];hallucinationAmount_=v[7];psychosisAmount_=v[8];
    therapyAmount_=v[9];bypass_=v[10]>=.5f;deliriumAmount_=v[11];catatoniaAmount_=v[12];amnesiaAmount_=v[13];
    appliedGeneration_=frame.generation;
    resetControls();
    if(prepared_)for(unsigned i=0;i<v.size();++i)applyDSP(i,v[i]);
    therapyDSP_=therapyAmount_;
    if(!live){bypassDSP_=bypass_;bypassMix_=bypass_?1.f:0.f;bypassRemaining_=0;}
    wetSmoothed_=1.f-therapyAmount_;
    publishState();
}
tresult PLUGIN_API Processor::getState(IBStream* state) {
    if(!state)return kResultFalse;
    StateFrame frame;
    {
        std::lock_guard<std::mutex> lock(controlStateMutex_);
        while(!audioState_.tryRead(frame)){} // non-audio host thread only
        if(latestRecall_.generation>frame.generation)frame=latestRecall_;
    }
    // No mutex or mutable audio state is retained across host stream calls.
    StateStream s(state);
    if(!s.writeInt32(kStateMagic)||!s.writeInt32(kStateVersion))return kResultFalse;
    for(unsigned i=0;i<10;++i)if(!s.writeFloat(frame.values[i]))return kResultFalse;
    if(!s.writeInt32(frame.values[10]>=.5f?1:0))return kResultFalse;
    for(unsigned i=11;i<14;++i)if(!s.writeFloat(frame.values[i]))return kResultFalse;
    return kResultOk;
}
} // namespace psychiatry::vst3
