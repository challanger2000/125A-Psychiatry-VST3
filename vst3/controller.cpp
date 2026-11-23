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
    // Only controls with real DSP are advertised. Remaining six are reserved.
    parameters.addParameter(STR16("DEPRESSION"), STR16("%"), 0, 0., flags, kDepression);
    parameters.addParameter(STR16("PARANOIA"), STR16("%"), 0, 0., flags, kParanoia);
    parameters.addParameter(STR16("OBSESSION"), STR16("%"), 0, 0., flags, kObsession);
    parameters.addParameter(STR16("THERAPY"), STR16("%"), 0, 0., flags, kTherapy);
    parameters.addParameter(STR16("Bypass"), nullptr, 1, 0.,
        flags | ParameterInfo::kIsBypass, kBypass);
    return kResultOk;
}

tresult PLUGIN_API Controller::setComponentState(IBStream* state) {
    if(!state) return kResultFalse;
    IBStreamer stream(state,kLittleEndian);
    int32 magic=0,version=0,bypass=0;
    float depression=0.f,paranoia=0.f,obsession=0.f,therapy=0.f;
    if(!stream.readInt32(magic)||!stream.readInt32(version)||
       magic!=kStateMagic||(version!=1 && version!=kStateVersion))
        return kResultFalse;
    if(version==1) {
        // P002 save format: paranoia, therapy, bypass.
        if(!stream.readFloat(paranoia)||!stream.readFloat(therapy)||
           !stream.readInt32(bypass)) return kResultFalse;
    } else {
        if(!stream.readFloat(depression)||!stream.readFloat(paranoia)||
           !stream.readFloat(obsession)||!stream.readFloat(therapy)||
           !stream.readInt32(bypass)) return kResultFalse;
    }
    if(!std::isfinite(depression)||!std::isfinite(paranoia)||
       !std::isfinite(obsession)||!std::isfinite(therapy))
        return kResultFalse;
    setParamNormalized(kDepression,std::clamp(static_cast<double>(depression),0.,1.));
    setParamNormalized(kParanoia,std::clamp(static_cast<double>(paranoia),0.,1.));
    setParamNormalized(kObsession,std::clamp(static_cast<double>(obsession),0.,1.));
    setParamNormalized(kTherapy,std::clamp(static_cast<double>(therapy),0.,1.));
    setParamNormalized(kBypass,bypass ? 1. : 0.);
    return kResultOk;
}

IPlugView* PLUGIN_API Controller::createView(FIDString) {
    // Host's generic parameter UI while the nine macros undergo sound quality review.
    return nullptr;
}
} // namespace psychiatry::vst3
