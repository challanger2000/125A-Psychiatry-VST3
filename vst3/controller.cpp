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
    // P002: only PARANOIA is functional. Do not advertise the other eight macros yet.
    parameters.addParameter(STR16("PARANOIA"), STR16("%"), 0, 0., flags, kParanoia);
    parameters.addParameter(STR16("THERAPY"), STR16("%"), 0, 0., flags, kTherapy);
    parameters.addParameter(STR16("Bypass"), nullptr, 1, 0.,
        flags | ParameterInfo::kIsBypass, kBypass);
    return kResultOk;
}

tresult PLUGIN_API Controller::setComponentState(IBStream* state) {
    if(!state) return kResultFalse;
    IBStreamer stream(state,kLittleEndian);
    int32 magic=0,version=0,bypass=0;
    float paranoia=0.f,therapy=0.f;
    if(!stream.readInt32(magic)||!stream.readInt32(version)||
       magic!=kStateMagic||version!=kStateVersion||
       !stream.readFloat(paranoia)||!stream.readFloat(therapy)||
       !stream.readInt32(bypass)) return kResultFalse;
    if(!std::isfinite(paranoia)||!std::isfinite(therapy)) return kResultFalse;
    setParamNormalized(kParanoia,std::clamp(static_cast<double>(paranoia),0.,1.));
    setParamNormalized(kTherapy,std::clamp(static_cast<double>(therapy),0.,1.));
    setParamNormalized(kBypass,bypass ? 1. : 0.);
    return kResultOk;
}

IPlugView* PLUGIN_API Controller::createView(FIDString) {
    // Generic host parameter UI. The final themed GUI comes only after DSP acceptance.
    return nullptr;
}
} // namespace psychiatry::vst3
