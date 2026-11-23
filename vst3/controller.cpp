#include "controller.h"
#include "ids.h"

#include "base/source/fstreamer.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace psychiatry::vst3 {
using namespace Steinberg;
namespace {
constexpr Steinberg::int32 kLossOfControlUiTag=9000;
// Exactly the nine sound-macro IDs: order is independent of DSP serial routing.
constexpr std::array<Steinberg::Vst::ParamID,9> kMacroIds{
    kDepression,kParanoia,kSchizophrenia,kMania,kPanic,
    kDissociation,kObsession,kHallucination,kPsychosis};
}

tresult PLUGIN_API Controller::initialize(FUnknown* context) {
    const auto result=EditControllerEx1::initialize(context);
    if(result!=kResultOk) return result;
    using namespace Steinberg::Vst;
    const int32 flags=ParameterInfo::kCanAutomate;
    // P005 all nine macro effect worlds have callable DSP implementations.
    parameters.addParameter(STR16("DEPRESSION"), STR16("%"), 0, 0., flags, kDepression);
    parameters.addParameter(STR16("PARANOIA"), STR16("%"), 0, 0., flags, kParanoia);
    parameters.addParameter(STR16("SCHIZOPHRENIA"), STR16("%"), 0, 0., flags, kSchizophrenia);
    parameters.addParameter(STR16("MANIA"), STR16("%"), 0, 0., flags, kMania);
    parameters.addParameter(STR16("PANIC"), STR16("%"), 0, 0., flags, kPanic);
    parameters.addParameter(STR16("DISSOCIATION"), STR16("%"), 0, 0., flags, kDissociation);
    parameters.addParameter(STR16("OBSESSION"), STR16("%"), 0, 0., flags, kObsession);
    parameters.addParameter(STR16("HALLUCINATION"), STR16("%"), 0, 0., flags, kHallucination);
    parameters.addParameter(STR16("PSYCHOSIS"), STR16("%"), 0, 0., flags, kPsychosis);
    parameters.addParameter(STR16("THERAPY"), STR16("%"), 0, 0., flags, kTherapy);
    parameters.addParameter(STR16("Bypass"), nullptr, 1, 0.,
        flags | ParameterInfo::kIsBypass, kBypass);
    return kResultOk;
}

tresult PLUGIN_API Controller::setComponentState(IBStream* state) {
    if(!state)return kResultFalse;
    IBStreamer s(state,kLittleEndian);
    int32 magic=0,version=0,bypass=0;
    float d=0.f,p=0.f,schizo=0.f,m=0.f,panic=0.f,
          diss=0.f,o=0.f,hall=0.f,psychosis=0.f,therapy=0.f;
    if(!s.readInt32(magic)||!s.readInt32(version)||
       magic!=kStateMagic||version<1||version>kStateVersion)return kResultFalse;
    if(version==1) {
        if(!s.readFloat(p)||!s.readFloat(therapy)||!s.readInt32(bypass))
            return kResultFalse;
    } else if(version==2) {
        if(!s.readFloat(d)||!s.readFloat(p)||!s.readFloat(o)||
           !s.readFloat(therapy)||!s.readInt32(bypass))return kResultFalse;
    } else if(version==3) {
        if(!s.readFloat(d)||!s.readFloat(p)||!s.readFloat(m)||
           !s.readFloat(panic)||!s.readFloat(o)||!s.readFloat(psychosis)||
           !s.readFloat(therapy)||!s.readInt32(bypass))return kResultFalse;
    } else {
        if(!s.readFloat(d)||!s.readFloat(p)||!s.readFloat(schizo)||
           !s.readFloat(m)||!s.readFloat(panic)||!s.readFloat(diss)||
           !s.readFloat(o)||!s.readFloat(hall)||!s.readFloat(psychosis)||
           !s.readFloat(therapy)||!s.readInt32(bypass))return kResultFalse;
    }
    const float values[]{d,p,schizo,m,panic,diss,o,hall,psychosis,therapy};
    for(const auto v:values)if(!std::isfinite(v))return kResultFalse;
    setParamNormalized(kDepression,std::clamp(static_cast<double>(d),0.,1.));
    setParamNormalized(kParanoia,std::clamp(static_cast<double>(p),0.,1.));
    setParamNormalized(kSchizophrenia,std::clamp(static_cast<double>(schizo),0.,1.));
    setParamNormalized(kMania,std::clamp(static_cast<double>(m),0.,1.));
    setParamNormalized(kPanic,std::clamp(static_cast<double>(panic),0.,1.));
    setParamNormalized(kDissociation,std::clamp(static_cast<double>(diss),0.,1.));
    setParamNormalized(kObsession,std::clamp(static_cast<double>(o),0.,1.));
    setParamNormalized(kHallucination,std::clamp(static_cast<double>(hall),0.,1.));
    setParamNormalized(kPsychosis,std::clamp(static_cast<double>(psychosis),0.,1.));
    setParamNormalized(kTherapy,std::clamp(static_cast<double>(therapy),0.,1.));
    setParamNormalized(kBypass,bypass?1.:0.);
    return kResultOk;
}

void Controller::triggerLossOfControl() {
    // Run exclusively on VSTGUI's UI callback thread. Randomize useful ranges
    // instead of maximum values for all nine effects (avoids overloading the
    // compound serial chain on the first randomization).
    std::uniform_real_distribution<double> amount(0.12,0.78);
    std::uniform_real_distribution<double> chance(0.0,1.0);
    for(const auto id:kMacroIds) {
        const double value=chance(random_)<0.18 ? 0.0 : amount(random_);
        beginEdit(id);
        setParamNormalized(id,value);
        performEdit(id,value);
        endEdit(id);
    }
}

// This is a host-embedded editor, not a stand-alone UI.
// Its private UI-only tag (9000) does not enter the VST3 parameter list.
class PsychiatryEditor final : public VSTGUI::VST3Editor {
public:
    explicit PsychiatryEditor(Controller* parent)
        : VSTGUI::VST3Editor(parent,"view","psychiatry.uidesc"), parent_(parent) {}
protected:
    void valueChanged(VSTGUI::CControl* control) override {
        if(control && control->getTag()==kLossOfControlUiTag) {
            // VSTGUI CTextButton with kick-style sends pressed and released
            // values. Trigger once on press only, not twice per click.
            if(control->getValueNormalized()>0.5f)
                parent_->triggerLossOfControl();
            return;
        }
        VSTGUI::VST3Editor::valueChanged(control);
    }
private:
    Controller* parent_;
};

IPlugView* PLUGIN_API Controller::createView(FIDString name) {
    // Minimal 125A front panel for the FIRST DAW acceptance test.
    // VSTGUI's VST3Editor performs normal beginEdit/performEdit/endEdit
    // and host-to-control parameter synchronization automatically.
    // No image decoding/GUI rendering is done in the audio callback.
    if(FIDStringsEqual(name,Steinberg::Vst::ViewType::kEditor))
        return new PsychiatryEditor(this);
    return nullptr;
}
} // namespace psychiatry::vst3
