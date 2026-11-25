#include "controller.h"
#include "ids.h"
#include "monitor_model.h"
#include "loss_of_control.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cresourcedescription.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include <cstring>
#include "pluginterfaces/vst/ivsteditcontroller.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace psychiatry::vst3 {
using namespace Steinberg;
namespace {
constexpr Steinberg::int32 kLossOfControlUiTag=9000;
constexpr Steinberg::int32 kZoomUiTag=9003;
constexpr std::array<double,6> kZoomFactors{.5,.75,1.,1.25,1.5,2.};
constexpr Steinberg::int32 kMotionUiTag=9002;
constexpr Steinberg::int32 kLanguageStateMagic=0x50535955; // 'PSYU'
constexpr Steinberg::int32 kLanguageStateVersion=3;
// Exactly the twelve sound-macro IDs; no global or UI-only controls.
constexpr std::array<Steinberg::Vst::ParamID,12> kMacroIds{
    kDepression,kParanoia,kSchizophrenia,kMania,kPanic,
    kDissociation,kObsession,kHallucination,kPsychosis,
    kDelirium,kCatatonia,kAmnesia};
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
    parameters.addParameter(STR16("DELIRIUM"), STR16("%"), 0, 0., flags, kDelirium);
    parameters.addParameter(STR16("CATATONIA"), STR16("%"), 0, 0., flags, kCatatonia);
    parameters.addParameter(STR16("AMNESIA"), STR16("%"), 0, 0., flags, kAmnesia);
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
    float delirium=0.f,catatonia=0.f,amnesia=0.f;
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
    if(version>=5&&(!s.readFloat(delirium)||!s.readFloat(catatonia)||!s.readFloat(amnesia)))return kResultFalse;
    const float values[]{d,p,schizo,m,panic,diss,o,hall,psychosis,therapy,delirium,catatonia,amnesia};
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
    setParamNormalized(kDelirium,std::clamp(static_cast<double>(delirium),0.,1.));
    setParamNormalized(kCatatonia,std::clamp(static_cast<double>(catatonia),0.,1.));
    setParamNormalized(kAmnesia,std::clamp(static_cast<double>(amnesia),0.,1.));
    setParamNormalized(kTherapy,std::clamp(static_cast<double>(therapy),0.,1.));
    setParamNormalized(kBypass,bypass?1.:0.);
    return kResultOk;
}

// Controller-only UI state; independent from processor project state v4.
// Empty state means an older version which saved no UI preferences.
tresult PLUGIN_API Controller::getState(IBStream* state) {
    if(!state)return kResultFalse;
    IBStreamer stream(state,kLittleEndian);
    if(!stream.writeInt32(kLanguageStateMagic)||
       !stream.writeInt32(kLanguageStateVersion)||
       !stream.writeInt32(germanUi_?1:0)||
       !stream.writeInt32(motionEnabled_?1:0)||
       !stream.writeInt32(zoomIndex_))return kResultFalse;
    return kResultOk;
}
tresult PLUGIN_API Controller::setState(IBStream* state) {
    if(!state)return kResultFalse;
    IBStreamer stream(state,kLittleEndian);
    int32 magic=0,version=0,lang=0,motion=0,zoom=0;
    if(!stream.readInt32(magic)) {
        germanUi_=false;
        motionEnabled_=false;
        zoomIndex_=0; // legacy sessions open at the safe 50% zoom
        return kResultOk;
    }
    if(magic!=kLanguageStateMagic||
       !stream.readInt32(version)||(version<1||version>kLanguageStateVersion)||
       !stream.readInt32(lang)||(lang!=0&&lang!=1))return kResultFalse;
    if(version>=2 && (!stream.readInt32(motion)||(motion!=0&&motion!=1)))
        return kResultFalse;
    if(version>=3 && (!stream.readInt32(zoom)||zoom<0||zoom>=static_cast<int32>(kZoomFactors.size())))
        return kResultFalse;
    germanUi_=(lang==1); // preserve historical controller-state bytes, English-only editor
    motionEnabled_=(motion!=0);
    zoomIndex_=zoom;
    return kResultOk;
}

tresult PLUGIN_API Controller::getParamStringByValue(
    Steinberg::Vst::ParamID id,
    Steinberg::Vst::ParamValue normalized,
    Steinberg::Vst::String128 text) {
    if(((id>=kDepression && id<=kAmnesia) || id==kTherapy)
       && text && std::isfinite(normalized)) {
        const int percent=static_cast<int>(std::lround(
            100.0*std::clamp(normalized,0.0,1.0)));
        char buffer[20]{};
        std::snprintf(buffer,sizeof(buffer),"%d %%",percent);
        Steinberg::UString(text,128).fromAscii(buffer);
        return kResultOk;
    }
    return EditControllerEx1::getParamStringByValue(id,normalized,text);
}

void Controller::triggerLossOfControl() {
    // UI thread only. All 12 IDs get updated, including explicit zeros.
    // Group the edits when the host supports IComponentHandler2.
    const auto values=randomLossOfControl(random_);
    static_assert(kMacroIds.size()==std::tuple_size<decltype(values)>::value);
    startGroupEdit();
    for(std::size_t i=0;i<kMacroIds.size();++i) {
        const auto id=kMacroIds[i];
        const double value=values[i];
        beginEdit(id);
        setParamNormalized(id,value);
        performEdit(id,value);
        endEdit(id);
    }
    finishGroupEdit();
}

// P028d: owner-approved raster faceplate. The standard native CAnimKnob
// handles the multi-frame atlases and host gestures; only the observation
// window and the two-state momentary button have custom drawing.
class ObservationWindow final : public VSTGUI::CView {
public:
    explicit ObservationWindow(Controller* controller)
        : VSTGUI::CView(VSTGUI::CRect(0,0,376,376)), controller_(controller) {
        using namespace VSTGUI;
        static constexpr const char* names[]{
            "scene-corridor-bw.png","scene-fictional-horror-portrait-bw.png",
            "scene-treatment-room-bw.png","scene-corridor-color.png",
            "scene-treatment-room-color.png"};
        for(std::size_t i=0;i<scenes_.size();++i)
            scenes_[i]=VSTGUI::owned(new CBitmap(CResourceDescription(names[i])));
        glass_=VSTGUI::owned(new CBitmap(CResourceDescription("porthole-glass.png")));
        // Copy only the outside of the fixed aperture from the exact faceplate.
        // This masks the moving photograph, NOT the fixed glass or metal ring.
        auto plate=VSTGUI::owned(new CBitmap(CResourceDescription("faceplate.png")));
        edge_=VSTGUI::owned(new CBitmap(376,376));
        bool maskOk=false;
        {
            auto source=VSTGUI::owned(CBitmapPixelAccess::create(plate,false));
            auto target=VSTGUI::owned(CBitmapPixelAccess::create(edge_,false));
            if(source&&target&&plate->getWidth()==1536&&plate->getHeight()==1024) {
                for(uint32_t y=0;y<376;++y)for(uint32_t x=0;x<376;++x){
                    CColor color{0,0,0,0};
                    source->setPosition(x+580,y+315);
                    source->getColor(color);
                    const double radius=std::hypot(double(x)-187.5,double(y)-187.5);
                    color.alpha=static_cast<uint8_t>(std::lround(
                        255.*std::clamp(radius-187.,0.,1.)));
                    target->setPosition(x,y);target->setColor(color);
                }
                maskOk=true;
            }
        }
        if(!maskOk)edge_=nullptr;
        model_=readModel();
        previousScene_=scene(model_);
        motion_=controller_->isMotionEnabled();
        setWantsIdle(true);
    }
    void onIdle() override {
        const auto next=readModel();
        const bool motion=controller_->isMotionEnabled();
        const bool changed=!(next==model_)||motion!=motion_;
        if(scene(next)!=scene(model_)){
            previousScene_=scene(model_);
            fade_=0;
        }
        model_=next;motion_=motion;
        if(motion_&&model_.peak>0.f) {
            tick_=(tick_+1)%1200u;
            fade_=std::min(18u,fade_+1);
            if(changed||tick_%3==0)invalid();
        } else {
            const bool reset=tick_!=0||fade_!=18;
            tick_=0;fade_=18;
            if(changed||reset)invalid();
        }
    }
    void draw(VSTGUI::CDrawContext* dc) override {
        using namespace VSTGUI;
        const auto r=getViewSize();
        ConcatClip clip(*dc,r);
        if(clip.isEmpty()){setDirty(false);return;}
        dc->setFillColor(CColor{0,0,0,255});
        dc->drawRect(r,kDrawFilled);
        const double phase=static_cast<double>(tick_)*.023;
        const bool move=motion_&&model_.peak>0.f&&edge_;
        const double scale=move?1.035+.015*std::sin(phase):1.;
        const double dx=move?2.2*std::sin(phase*.73):0.;
        const double dy=move?1.7*std::sin(phase*.91):0.;
        auto paint=[&](CBitmap* bmp,float opacity,double z,double ox,double oy){
            if(!bmp||!bmp->isLoaded())return;
            const double w=r.getWidth()*z,h=r.getHeight()*z;
            CGraphicsTransform tr;
            tr.scale(w/bmp->getWidth(),h/bmp->getHeight())
              .translate(r.left+(r.getWidth()-w)*.5+ox,r.top+(r.getHeight()-h)*.5+oy);
            CDrawContext::Transform transform(*dc,tr);
            bmp->draw(dc,CRect(0,0,bmp->getWidth(),bmp->getHeight()),CPoint(0,0),opacity);
        };
        const float level=model_.peak>0.f?.50f+.50f*model_.peak:.30f;
        const int current=scene(model_);
        if(motion_&&fade_<18&&previousScene_!=current){
            paint(scenes_[previousScene_],level,scale,dx,dy);
            paint(scenes_[current],level*float(fade_)/18.f,scale,dx,dy);
        }else paint(scenes_[current],level,scale,dx,dy);
        const int secondary=model_.secondary();
        if(secondary>=0&&model_.peak>0.f) {
            const int other=worldScene(secondary);
            if(other!=current)paint(scenes_[other],.18f*model_.levels[secondary],scale,dx,dy);
        }
        paint(glass_,1.f,1.,0.,0.);
        if(edge_)edge_->draw(dc,r);
        // A missing critical texture must not silently resemble a normal standby.
        if(!scenes_[current]->isLoaded()||!glass_->isLoaded()){
            dc->setFont(kNormalFont);dc->setFontColor(CColor{255,190,150,255});
            dc->drawString("ARTWORK MISSING",r,kCenterText,true);
        }
        setDirty(false);
    }
private:
    // Five owner-reviewed candidate photographs: provisional reuse for twelve
    // worlds, not the final distinct image library.
    static int worldScene(int world) noexcept {
        constexpr std::array<int,12> map{0,1,2,3,4,0,2,1,4,3,2,0};
        return map[static_cast<std::size_t>(std::clamp(world,0,11))];
    }
    static int scene(const MonitorModel& m) noexcept {return m.peak>0.f?worldScene(m.dominant):0;}
    MonitorModel readModel() const {
        std::array<double,MonitorModel::worldCount> macros{};
        for(std::size_t i=0;i<macros.size();++i)
            macros[i]=controller_->getParamNormalized(kMacroIds[i]);
        return MonitorModel::make(macros,controller_->getParamNormalized(kTherapy),
                                  controller_->getParamNormalized(kBypass));
    }
    Controller* controller_;
    std::array<VSTGUI::SharedPointer<VSTGUI::CBitmap>,5> scenes_{};
    VSTGUI::SharedPointer<VSTGUI::CBitmap> glass_,edge_;
    MonitorModel model_{};
    int previousScene_=0;
    bool motion_=false;
    unsigned tick_=0,fade_=18;
};

class RasterLossButton final : public VSTGUI::CTextButton {
public:
    RasterLossButton():CTextButton(VSTGUI::CRect(0,0,110,106),nullptr,-1,nullptr,kKickStyle) {
        using namespace VSTGUI;
        released_=VSTGUI::owned(new CBitmap(CResourceDescription("loss-released.png")));
        pressed_=VSTGUI::owned(new CBitmap(CResourceDescription("loss-pressed.png")));
    }
    void draw(VSTGUI::CDrawContext* dc) override {
        auto image=getValueNormalized()>.5f?pressed_:released_;
        if(image&&image->isLoaded())image->draw(dc,getViewSize());
        else CTextButton::draw(dc);
        setDirty(false);
    }
private:
    VSTGUI::SharedPointer<VSTGUI::CBitmap> released_,pressed_;
};

class PsychiatryEditor final : public VSTGUI::VST3Editor, public VSTGUI::VST3EditorDelegate {
public:
    explicit PsychiatryEditor(Controller* parent)
        : VSTGUI::VST3Editor(parent,"view","psychiatry.uidesc"), parent_(parent) {
        setDelegate(this);
        setAllowedZoomFactors(std::vector<double>(kZoomFactors.begin(),kZoomFactors.end()));
        setZoomFactor(kZoomFactors[static_cast<std::size_t>(parent_->zoomIndex())]);
    }
    VSTGUI::CView* createCustomView(VSTGUI::UTF8StringPtr name,
            const VSTGUI::UIAttributes&,
            const VSTGUI::IUIDescription*,
            VSTGUI::VST3Editor*) override {
        if(!name)return nullptr;
        if(std::strcmp(name,"PSYCHIATRY_OBSERVATION")==0)return new ObservationWindow(parent_);
        if(std::strcmp(name,"PSYCHIATRY_LOSS")==0)return new RasterLossButton;
        if(std::strcmp(name,"PSYCHIATRY_ZOOM")==0){
            auto* menu=new VSTGUI::COptionMenu(VSTGUI::CRect(0,0,124,36),nullptr,-1);
            for(double factor:kZoomFactors){
                char label[16]{};
                std::snprintf(label,sizeof(label),"%d%%",static_cast<int>(factor*100));
                menu->addEntry(label);
            }
            menu->setCurrent(parent_->zoomIndex());
            zoomMenu_=menu;
            return menu;
        }
        return nullptr;
    }
    void onZoomChanged(VSTGUI::VST3Editor*,double factor) override {
        for(std::size_t i=0;i<kZoomFactors.size();++i)if(factor==kZoomFactors[i]){
            parent_->setZoomIndex(static_cast<int>(i));
            if(zoomMenu_){zoomMenu_->setCurrent(static_cast<int>(i));zoomMenu_->invalid();}
        }
    }
protected:
    void valueChanged(VSTGUI::CControl* control) override {
        if(control&&control->getTag()==kMotionUiTag){
            parent_->setMotionEnabled(control->getValueNormalized()>.5f);return;
        }
        if(control&&control->getTag()==kZoomUiTag){
            if(auto* menu=dynamic_cast<VSTGUI::COptionMenu*>(control)){
                const int index=menu->getCurrentIndex();
                if(index>=0&&index<static_cast<int>(kZoomFactors.size()))
                    setZoomFactor(kZoomFactors[static_cast<std::size_t>(index)]);
            }
            return;
        }
        if(control&&control->getTag()==kLossOfControlUiTag){
            if(control->getValueNormalized()>.5f)parent_->triggerLossOfControl();
            return;
        }
        VSTGUI::VST3Editor::valueChanged(control);
    }
    void onViewAdded(VSTGUI::CFrame* frame,VSTGUI::CView* view) override {
        VSTGUI::VST3Editor::onViewAdded(frame,view);
        if(auto* control=dynamic_cast<VSTGUI::CControl*>(view)){
            if(control->getTag()==kMotionUiTag){
                control->setValueNormalized(parent_->isMotionEnabled()?1.f:0.f);control->invalid();
            }
        }
    }
    void onViewRemoved(VSTGUI::CFrame* frame,VSTGUI::CView* view) override {
        if(view==zoomMenu_)zoomMenu_=nullptr;
        VSTGUI::VST3Editor::onViewRemoved(frame,view);
    }
private:
    Controller* parent_;
    VSTGUI::COptionMenu* zoomMenu_=nullptr;
};

IPlugView* PLUGIN_API Controller::createView(FIDString name) {
    // P028d raster editor candidate; preserve the existing VST3 host adapter.
    // VSTGUI's VST3Editor performs normal beginEdit/performEdit/endEdit
    // and host-to-control parameter synchronization automatically.
    // No image decoding/GUI rendering is done in the audio callback.
    if(FIDStringsEqual(name,Steinberg::Vst::ViewType::kEditor))
        return new PsychiatryEditor(this);
    return nullptr;
}
} // namespace psychiatry::vst3
