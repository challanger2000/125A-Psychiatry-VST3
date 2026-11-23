#include "controller.h"
#include "ids.h"

#include "base/source/fstreamer.h"

#include <algorithm>
#include <cmath>

namespace psychiatry::vst3 {
using namespace Steinberg;

tresult PLUGIN_API Controller::initialize(FUnknown* context) {
    const auto result=EditControllerEx1::initialize(context);
    if(result!=kResultOk) return result;
    using namespace Steinberg::Vst;
    const int32 flags=ParameterInfo::kCanAutomate;
    // Six implemented sound-world macros only. Other three remain hidden.
    parameters.addParameter(STR16("DEPRESSION"), STR16("%"), 0, 0., flags, kDepression);
    parameters.addParameter(STR16("PARANOIA"), STR16("%"), 0, 0., flags, kParanoia);
    parameters.addParameter(STR16("MANIA"), STR16("%"), 0, 0., flags, kMania);
    parameters.addParameter(STR16("PANIC"), STR16("%"), 0, 0., flags, kPanic);
    parameters.addParameter(STR16("OBSESSION"), STR16("%"), 0, 0., flags, kObsession);
    parameters.addParameter(STR16("PSYCHOSIS"), STR16("%"), 0, 0., flags, kPsychosis);
    parameters.addParameter(STR16("THERAPY"), STR16("%"), 0, 0., flags, kTherapy);
    parameters.addParameter(STR16("Bypass"), nullptr, 1, 0.,
        flags | ParameterInfo::kIsBypass, kBypass);
    return kResultOk;
}

tresult PLUGIN_API Controller::setComponentState(IBStream* state) {
    if(!state) return kResultFalse;
    IBStreamer stream(state,kLittleEndian);
    int32 magic=0,version=0,bypass=0;
    float d=0.f,p=0.f,m=0.f,panic=0.f,o=0.f,psychosis=0.f,therapy=0.f;
    if(!stream.readInt32(magic)||!stream.readInt32(version)||
       magic!=kStateMagic||version<1||version>kStateVersion) return kResultFalse;
    if(version==1) {
        if(!stream.readFloat(p)||!stream.readFloat(therapy)||
           !stream.readInt32(bypass)) return kResultFalse;
    } else if(version==2) {
        if(!stream.readFloat(d)||!stream.readFloat(p)||!stream.readFloat(o)||
           !stream.readFloat(therapy)||!stream.readInt32(bypass)) return kResultFalse;
    } else {
        if(!stream.readFloat(d)||!stream.readFloat(p)||!stream.readFloat(m)||
           !stream.readFloat(panic)||!stream.readFloat(o)||!stream.readFloat(psychosis)||
           !stream.readFloat(therapy)||!stream.readInt32(bypass)) return kResultFalse;
    }
    if(!std::isfinite(d)||!std::isfinite(p)||!std::isfinite(m)||
       !std::isfinite(panic)||!std::isfinite(o)||!std::isfinite(psychosis)||
       !std::isfinite(therapy)) return kResultFalse;
    setParamNormalized(kDepression,std::clamp(static_cast<double>(d),0.,1.));
    setParamNormalized(kParanoia,std::clamp(static_cast<double>(p),0.,1.));
    setParamNormalized(kMania,std::clamp(static_cast<double>(m),0.,1.));
    setParamNormalized(kPanic,std::clamp(static_cast<double>(panic),0.,1.));
    setParamNormalized(kObsession,std::clamp(static_cast<double>(o),0.,1.));
    setParamNormalized(kPsychosis,std::clamp(static_cast<double>(psychosis),0.,1.));
    setParamNormalized(kTherapy,std::clamp(static_cast<double>(therapy),0.,1.));
    setParamNormalized(kBypass,bypass ? 1. : 0.);
    return kResultOk;
}

IPlugView* PLUGIN_API Controller::createView(FIDString) {
    // Host's generic parameter UI while the nine macros undergo sound quality review.
    return nullptr;
}
} // namespace psychiatry::vst3
