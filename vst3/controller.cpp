#include "controller.h"
#include "ids.h"
#include "monitor_model.h"
#include "scene_sequence.h"
#include "scene_blend.h"
#include <chrono>
#include "loss_of_control.h"

#include "state_stream.h"
#include "vstgui/lib/cframe.h"
#include "pluginterfaces/base/ustring.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cresourcedescription.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/coffscreencontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include <cstring>
#include "pluginterfaces/vst/ivsteditcontroller.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdint>

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
    StateStream s(state);
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
    StateStream stream(state);
    if(!stream.writeInt32(kLanguageStateMagic)||
       !stream.writeInt32(kLanguageStateVersion)||
       !stream.writeInt32(germanUi_?1:0)||
       !stream.writeInt32(motionEnabled_?1:0)||
       !stream.writeInt32(zoomIndex_))return kResultFalse;
    return kResultOk;
}
tresult PLUGIN_API Controller::setState(IBStream* state) {
    if(!state)return kResultFalse;
    StateStream stream(state);
    int32 magic=0,version=0,lang=0,motion=0,zoom=0;
    if(stream.empty()) {
        applyUiState(false,false,0); // legacy sessions open at the safe 50% zoom
        return kResultOk;
    }
    if(!stream.readInt32(magic)||magic!=kLanguageStateMagic||
       !stream.readInt32(version)||(version<1||version>kLanguageStateVersion)||
       !stream.readInt32(lang)||(lang!=0&&lang!=1))return kResultFalse;
    if(version>=2 && (!stream.readInt32(motion)||(motion!=0&&motion!=1)))
        return kResultFalse;
    if(version>=3 && (!stream.readInt32(zoom)||zoom<0||zoom>=static_cast<int32>(kZoomFactors.size())))
        return kResultFalse;
    applyUiState(lang==1,motion!=0,zoom); // English-only editor; preserve state bytes
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
        sequence_.update(model_.peak>0.f?model_.dominant:-1,false,0.);
        currentScene_=sequence_.current();
        lastIdle_=Clock::now();
        refreshTextures();
        motion_=controller_->isMotionEnabled();
        setWantsIdle(true);
    }
    void onIdle() override {
        const auto now=Clock::now();
        const double dt=std::clamp(std::chrono::duration<double>(now-lastIdle_).count(),0.,1.);
        lastIdle_=now;
        const auto next=readModel();
        const bool motion=controller_->isMotionEnabled();
        const bool changed=!(next==model_)||motion!=motion_;
        sequence_.update(next.peak>0.f?next.dominant:-1,motion,dt);
        const int selected=sequence_.current();
        bool sceneChanged=false;
        if(selected!=currentScene_){
            // Keep the visible mixture, not the old target, when interrupted.
            // A failed offscreen allocation defers the switch rather than jumping.
            if(motion&&next.peak>0.f){
                auto from=freezeCurrentMix();
                if(from){
                    previous_=from;
                    currentScene_=selected;
                    fade_=0.;
                    sceneChanged=true;
                }
            }else{
                previous_=nullptr;
                currentScene_=selected;
                fade_=1.;
                sceneChanged=true;
            }
        }
        model_=next;motion_=motion;
        if(motion_&&model_.peak>0.f) {
            phase_=std::fmod(phase_+dt*.9,6283.185307179586);
            if(!sceneChanged)fade_=std::min(1.,fade_+dt/1.2);
            if(fade_>=1.)previous_=nullptr;
            refreshTextures();
            redrawElapsed_+=dt;
            if(changed||sceneChanged||redrawElapsed_>=1./30.){redrawElapsed_=0.;invalid();}
        } else {
            const bool reset=phase_!=0.||fade_!=1.;
            phase_=0.;fade_=1.;redrawElapsed_=0.;previous_=nullptr;
            refreshTextures();
            if(changed||sceneChanged||reset)invalid();
        }
    }
    void draw(VSTGUI::CDrawContext* dc) override {
        using namespace VSTGUI;
        const auto r=getViewSize();
        ConcatClip clip(*dc,r);
        if(clip.isEmpty()){setDirty(false);return;}
        dc->setFillColor(CColor{0,0,0,255});
        dc->drawRect(r,kDrawFilled);
        const double phase=phase_;
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
        const int current=currentScene_;
        if(motion_&&fade_<1.&&previous_){
            // Source-over weights: old*(1-f)*level + new*f*level.
            const auto alpha=sceneAlpha(level,static_cast<float>(fade_));
            paint(previous_,alpha.previous,scale,dx,dy);
            paint(scenes_[current],alpha.current,scale,dx,dy);
        }else paint(scenes_[current],level,scale,dx,dy);
        const int secondary=model_.secondary();
        if(secondary>=0&&model_.peak>0.f) {
            const int other=sequence_.forWorld(secondary);
            if(other!=current)paint(scenes_[other],.18f*model_.levels[secondary],scale,dx,dy);
        }
        paint(glass_,1.f,1.,0.,0.);
        if(edge_)edge_->draw(dc,r);
        // A missing critical texture must not silently resemble a normal standby.
        if(!scenes_[current]||!scenes_[current]->isLoaded()||!glass_->isLoaded()){
            dc->setFont(kNormalFont);dc->setFontColor(CColor{255,190,150,255});
            dc->drawString("ARTWORK MISSING",r,kCenterText,true);
        }
        setDirty(false);
    }
private:
    using Clock=std::chrono::steady_clock;
    // Freeze the opaque photo blend before the common scene opacity, motion
    // transform, aperture mask and glass. At most one persistent composite
    // plus current/secondary textures; no accumulating image stack.
    VSTGUI::SharedPointer<VSTGUI::CBitmap> freezeCurrentMix() {
        using namespace VSTGUI;
        auto current=scenes_[currentScene_];
        if(!current||!current->isLoaded())return nullptr;
        if(!previous_||fade_>=1.)return current;
        if(fade_<=0.)return previous_;
        const auto previous=previous_;
        const auto f=static_cast<float>(fade_);
        return renderBitmapOffscreen(CPoint(768,768),1.,[&](CDrawContext& dc){
            auto draw=[&](CBitmap* bitmap,float alpha){
                CGraphicsTransform tr;
                tr.scale(768./bitmap->getWidth(),768./bitmap->getHeight());
                CDrawContext::Transform transform(dc,tr);
                bitmap->draw(&dc,CRect(0,0,bitmap->getWidth(),bitmap->getHeight()),CPoint(0,0),alpha);
            };
            draw(previous,1.f);
            draw(current,f);
        });
    }
    // Decode and release only on the GUI thread; never in audio processing.
    void refreshTextures() {
        using namespace VSTGUI;
        const int secondary=model_.secondary();
        const int other=secondary>=0&&model_.peak>0.f?sequence_.forWorld(secondary):-1;
        for(std::size_t i=0;i<scenes_.size();++i) {
            const int index=static_cast<int>(i);
            const bool wanted=index==currentScene_||index==other;
            if(!wanted)scenes_[i]=nullptr;
            else if(!scenes_[i])scenes_[i]=VSTGUI::owned(new CBitmap(CResourceDescription(kSceneFiles[i])));
        }
    }
    MonitorModel readModel() const {
        std::array<double,MonitorModel::worldCount> macros{};
        for(std::size_t i=0;i<macros.size();++i)
            macros[i]=controller_->getParamNormalized(kMacroIds[i]);
        return MonitorModel::make(macros,controller_->getParamNormalized(kTherapy),
                                  controller_->getParamNormalized(kBypass));
    }
    Controller* controller_;
    std::array<VSTGUI::SharedPointer<VSTGUI::CBitmap>,kSceneFiles.size()> scenes_{};
    SceneSequence sequence_{};
    VSTGUI::SharedPointer<VSTGUI::CBitmap> glass_,edge_,previous_;
    MonitorModel model_{};
    int currentScene_=0;
    bool motion_=false;
    Clock::time_point lastIdle_{};
    double phase_=0.,fade_=1.,redrawElapsed_=0.;
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

// The pinned SDK calls resizeView from inside enableEditing/open, before
// the CFrame and native child are fully initialized. Isolate that internal
// geometry work; forward platform interfaces (e.g. Linux IRunLoop) to the
// actual host, but negotiate the native size only after construction.
class AttachGeometryFrame final : public IPlugFrame {
public:
    explicit AttachGeometryFrame(IPlugFrame* host):host_(host){}
    tresult PLUGIN_API queryInterface(const TUID iid,void** out) override {
        if(!out)return kInvalidArgument;
        *out=nullptr;
        if(FUnknownPrivate::iidEqual(iid,FUnknown::iid)||
           FUnknownPrivate::iidEqual(iid,IPlugFrame::iid)){
            *out=static_cast<IPlugFrame*>(this);addRef();return kResultOk;
        }
        return host_?host_->queryInterface(iid,out):kNoInterface;
    }
    uint32 PLUGIN_API addRef() override{return ++refs_;}
    uint32 PLUGIN_API release() override{const auto n=--refs_;if(!n)delete this;return n;}
    tresult PLUGIN_API resizeView(IPlugView*,ViewRect* size) override {
        return size&&size->getWidth()>0&&size->getHeight()>0?kResultTrue:kResultFalse;
    }
private:
    IPtr<IPlugFrame> host_;
    uint32 refs_=1;
};

class PsychiatryEditor final : public VSTGUI::VST3Editor, public VSTGUI::VST3EditorDelegate {
public:
    explicit PsychiatryEditor(Controller* parent)
        : VSTGUI::VST3Editor(parent,"view","psychiatry.uidesc"), parent_(parent) {
        setDelegate(this);
        setAllowedZoomFactors(std::vector<double>(kZoomFactors.begin(),kZoomFactors.end()));
        acceptedZoom_=parent_->zoomIndex();
        setZoomFactor(kZoomFactors[static_cast<std::size_t>(acceptedZoom_)]);
        setRect(preferredRect());
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
            menu->setValue(static_cast<float>(parent_->zoomIndex()));
            zoomMenu_=menu;
            return menu;
        }
        return nullptr;
    }
    void didOpen(VSTGUI::VST3Editor*) override {
        open_=true;
        if(!building_)parent_->editorOpened(this);
        syncMotion();
        syncZoomMenu();
    }
    void willClose(VSTGUI::VST3Editor*) override {
        open_=false;
        parent_->editorClosed(this);
    }
    ~PsychiatryEditor() override {parent_->editorClosed(this);}
    void syncMotion() {
        if(open_&&motionControl_){
            motionControl_->setValueNormalized(parent_->isMotionEnabled()?1.f:0.f);
            motionControl_->invalid();
        }
    }
    bool applyZoom(int index) {return applyGeometry(index,getContentScaleFactor());}
    bool applyGeometry(int index,double scale) {
        if(!open_||!getFrame())return false;
        VSTGUI::SharedPointer<PsychiatryEditor> keepEditor(this);
        VSTGUI::SharedPointer<VSTGUI::CFrame> keepFrame(getFrame());
        const auto generation=attachmentGeneration_;
        const auto oldZoom=zoomFactor,oldScale=contentScaleFactor;
        const auto oldRect=getRect();
        const auto targetZoom=kZoomFactors[static_cast<std::size_t>(index)];
        const double target=targetZoom*scale;
        // One operation owns both factors and the frame transform.
        // Do not invoke VST3Editor's delegate-based zoom path: it can
        // initiate another resize before the first CFrame::setZoom returns.
        zoomFactor=targetZoom;contentScaleFactor=scale;
        const bool already=std::abs(keepFrame->getTransform().m11-target)<1.e-6&&
            std::abs(keepFrame->getWidth()-minSize.x*target)<1.&&
            std::abs(keepFrame->getHeight()-minSize.y*target)<1.;
        const bool accepted=already||keepFrame->setZoom(target);
        if(generation!=attachmentGeneration_||getFrame()!=keepFrame.get())return false;
        if(!accepted){
            zoomFactor=oldZoom;contentScaleFactor=oldScale;
            setRect(oldRect);syncZoomMenu();return false;
        }
        acceptedZoom_=index;syncZoomMenu();
        return open_;
    }
    void syncZoomMenu() {
        if(open_&&zoomMenu_){zoomMenu_->setValue(static_cast<float>(acceptedZoom_));zoomMenu_->invalid();}
    }
protected:
    tresult PLUGIN_API attached(void* parent,FIDString type) override {
        if(building_||open_||getFrame()||!type||
           isPlatformTypeSupported(type)!=kResultTrue)return kResultFalse;
        VSTGUI::SharedPointer<PsychiatryEditor> keepEditor(this);
        const auto oldRect=getRect();
        attachHost_=plugFrame;
        if(!attachHost_)return kResultFalse;
        const auto generation=++attachmentGeneration_;
        negotiatingGeneration_=generation;pendingScale_=0.;
        auto internal=Steinberg::owned(new AttachGeometryFrame(attachHost_));
        building_=true;attachCanceled_=false;
        plugFrame=internal.get();
        // Reset both SDK geometry caches on *every* attachment, including
        // reuse of the same IPlugView after removal at a different scale.
        acceptedZoom_=parent_->zoomIndex();
        setZoomFactor(kZoomFactors[static_cast<std::size_t>(acceptedZoom_)]);
        nonEditRect=VSTGUI::CRect();
        sizeRequest={};
        ViewRect wanted=preferredRect();
        setRect(wanted);
        const auto result=Steinberg::Vst::VSTGUIEditor::attached(parent,type);
        building_=false;
        plugFrame=attachHost_;
        attachHost_=nullptr;
        const bool valid=result==kResultOk&&!attachCanceled_&&open_&&getFrame()&&
            getFrame()->isAttached()&&getFrame()->hasChildren()&&
            std::abs(getFrame()->getWidth()-wanted.getWidth())<1.&&
            std::abs(getFrame()->getHeight()-wanted.getHeight())<1.;
        if(!valid||!plugFrame){
            Steinberg::Vst::VSTGUIEditor::removed();setRect(oldRect);return kResultFalse;
        }
        // A host may reject or synchronously call removed/setFrame(nullptr)
        // here. The SDK construction stack has already unwound. Retain the
        // host and editor across the single external resize request.
        IPtr<IPlugFrame> host=plugFrame;
        // EditorHost (and other hosts) compares getSize() with the request
        // and returns success without resizing if they already match.
        // Publish the *previous* host rectangle until it accepts the request;
        // the internal native frame already has the desired geometry.
        setRect(oldRect);
        const auto requested=wanted;
        const auto accepted=host->resizeView(this,&wanted);
        // The callback can remove this attachment and attach the same
        // IPlugView again before returning. The older transaction has no
        // authority over the new frame, rectangle or controller registry.
        if(generation!=attachmentGeneration_)return kResultFalse;
        if(accepted!=kResultTrue||!open_||!getFrame()||plugFrame.get()!=host.get()||
           wanted.getWidth()!=requested.getWidth()||wanted.getHeight()!=requested.getHeight()||
           getFrame()->getWidth()!=requested.getWidth()||getFrame()->getHeight()!=requested.getHeight()){
            if(open_||getFrame())Steinberg::Vst::VSTGUIEditor::removed();
            setRect(oldRect);return kResultFalse;
        }
        setRect(requested);
        // The view is intentionally absent from the controller registry
        // during construction/initial negotiation. Reconcile any UI recall
        // delivered by those callbacks before publishing this attachment.
        // Bound the number of negotiations even if a host changes its
        // preference on every callback. Refusal or persistent churn fails
        // this attachment cleanly rather than exposing inconsistent state.
        for(unsigned attempt=0;acceptedZoom_!=parent_->zoomIndex()||pendingScale_>0.;++attempt){
            if(attempt==6){
                Steinberg::Vst::VSTGUIEditor::removed();setRect(oldRect);return kResultFalse;
            }
            const double scale=pendingScale_>0.?pendingScale_:getContentScaleFactor();
            pendingScale_=0.;
            const bool zoomAccepted=applyGeometry(parent_->zoomIndex(),scale);
            if(generation!=attachmentGeneration_)return kResultFalse;
            if(!zoomAccepted||!open_||!getFrame()||plugFrame.get()!=host.get()){
                if(open_||getFrame())Steinberg::Vst::VSTGUIEditor::removed();
                setRect(oldRect);return kResultFalse;
            }
        }
        negotiatingGeneration_=0;
        parent_->editorOpened(this);syncMotion();syncZoomMenu();
        return kResultOk;
    }
    tresult PLUGIN_API removed() override {
        if(building_){attachCanceled_=true;return kResultOk;}
        VSTGUI::SharedPointer<PsychiatryEditor> keepEditor(this);
        return Steinberg::Vst::VSTGUIEditor::removed();
    }
    tresult PLUGIN_API setFrame(IPlugFrame* host) override {
        if(building_){
            if(attachHost_.get()!=host)attachCanceled_=true;
            attachHost_=host;return kResultOk;
        }
        return Steinberg::Vst::VSTGUIEditor::setFrame(host);
    }
    bool beforeSizeChange(const VSTGUI::CRect& next,const VSTGUI::CRect& old) override {
        if(building_)return true;
        const auto generation=attachmentGeneration_;
        IPtr<IPlugFrame> keepHost=plugFrame;
        const bool accepted=VSTGUI::VST3Editor::beforeSizeChange(next,old);
        return accepted&&generation==attachmentGeneration_&&plugFrame.get()==keepHost.get();
    }
    void PLUGIN_API close() override {
        if(!getFrame())return;
        ++attachmentGeneration_;negotiatingGeneration_=0;pendingScale_=0.;
        VSTGUI::VST3Editor::close();
        // The pinned VSTGUI close() only clears its frame pointer when
        // refcount==1. If a resize or native event retains the frame,
        // it releases the editor's ownership but leaves a stale pointer.
        // Clear it now; the retained frame is destroyed by its last owner.
        frame=nullptr;
    }
    tresult PLUGIN_API onSize(ViewRect* rect) override {
        VSTGUI::SharedPointer<PsychiatryEditor> keepEditor(this);
        VSTGUI::SharedPointer<VSTGUI::CFrame> keepFrame(getFrame());
        return VSTGUI::VST3Editor::onSize(rect);
    }
#ifdef VST3_CONTENT_SCALE_SUPPORT
    tresult PLUGIN_API setContentScaleFactor(ScaleFactor factor) override {
        if(!std::isfinite(factor)||factor<=0.||minSize.y*.5*factor<1.||
           minSize.x*2.*factor>2147483647.)return kInvalidArgument;
        VSTGUI::SharedPointer<PsychiatryEditor> keepEditor(this);
        if(building_||(negotiatingGeneration_!=0&&negotiatingGeneration_==attachmentGeneration_)){
            pendingScale_=factor;return kResultOk;
        }
        if(!getFrame()){
            // Before setFrame(), the host sizes its future parent from
            // getSize(). Publish the scaled rectangle immediately. Once
            // a host frame exists, negotiate with it before publishing,
            // preserving the old rectangle for equality-shortcut hosts.
            if(preAttachResize_)return kResultFalse;
            const auto oldRect=getRect();
            const auto oldScale=contentScaleFactor,oldZoom=zoomFactor;
            const auto oldIndex=acceptedZoom_;
            contentScaleFactor=factor;
            acceptedZoom_=parent_->zoomIndex();
            zoomFactor=kZoomFactors[static_cast<std::size_t>(acceptedZoom_)];
            auto wanted=preferredRect();
            const auto requested=wanted;
            IPtr<IPlugFrame> host=plugFrame;
            if(!host){setRect(requested);return kResultOk;}
            const auto generation=attachmentGeneration_;
            preAttachResize_=true;
            const auto result=host->resizeView(this,&wanted);
            preAttachResize_=false;
            if(generation!=attachmentGeneration_||getFrame())return kResultFalse;
            if(result!=kResultTrue||plugFrame.get()!=host.get()||
               wanted.getWidth()!=requested.getWidth()||wanted.getHeight()!=requested.getHeight()){
                contentScaleFactor=oldScale;zoomFactor=oldZoom;acceptedZoom_=oldIndex;
                setRect(oldRect);return kResultFalse;
            }
            setRect(requested);return kResultOk;
        }
        return parent_->requestEditorScale(this,factor);

    }
#endif
    bool onCommandMenuItemSelected(VSTGUI::CCommandMenuItem* item) override {
        VSTGUI::SharedPointer<PsychiatryEditor> keepEditor(this);
        VSTGUI::SharedPointer<VSTGUI::CFrame> keepFrame(getFrame());
        if(item&&item->getCommandCategory()=="Zoom"){
            parent_->setZoomIndex(item->getTag());return true;
        }
        return VSTGUI::VST3Editor::onCommandMenuItemSelected(item);
    }
    void valueChanged(VSTGUI::CControl* control) override {
        if(control&&control->getTag()==kMotionUiTag){
            if(open_&&control->isEditing())parent_->setMotionEnabled(control->getValueNormalized()>.5f);
            return;
        }
        if(control&&control->getTag()==kZoomUiTag){
            if(auto* menu=dynamic_cast<VSTGUI::COptionMenu*>(control)){
                const int index=menu->getCurrentIndex();
                if(index>=0&&index<static_cast<int>(kZoomFactors.size()))
                    if(open_)parent_->setZoomIndex(index);
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
                motionControl_=control;
                control->setValueNormalized(parent_->isMotionEnabled()?1.f:0.f);control->invalid();
            }else if(control->getTag()==kZoomUiTag){
                // The SDK XML creator may apply default values after construction.
                control->setValue(static_cast<float>(parent_->zoomIndex()));control->invalid();
            }
        }
    }
    void onViewRemoved(VSTGUI::CFrame* frame,VSTGUI::CView* view) override {
        if(view==zoomMenu_)zoomMenu_=nullptr;
        if(view==motionControl_)motionControl_=nullptr;
        VSTGUI::VST3Editor::onViewRemoved(frame,view);
    }
private:
    ViewRect preferredRect() const {
        const auto scale=getAbsScaleFactor();
        return ViewRect(0,0,static_cast<int32>(std::floor(minSize.x*scale)),
                            static_cast<int32>(std::floor(minSize.y*scale)));
    }
    Controller* parent_;
    VSTGUI::COptionMenu* zoomMenu_=nullptr;
    VSTGUI::CControl* motionControl_=nullptr;
    int acceptedZoom_=0;
    friend class Controller;
    bool open_=false;
    bool building_=false,attachCanceled_=false,preAttachResize_=false;
    IPtr<IPlugFrame> attachHost_;
    std::uint64_t attachmentGeneration_=0;
    std::uint64_t negotiatingGeneration_=0,registrationSerial_=0;
    double pendingScale_=0.;
};

// UI thread only. Keep actual per-view geometry and menu on refusal.
// Shared saved zoom becomes the requested index when at least one attached
// view accepts it; if all refuse, retain the old preference. With no views,
// store the preference for the next attachment. No repeated resize retries.
void Controller::editorOpened(PsychiatryEditor* editor) {
    if(std::find(editors_.begin(),editors_.end(),editor)==editors_.end()){
        editor->registrationSerial_=++registrationSerial_;
        editors_.push_back(editor);
    }
}
void Controller::editorClosed(PsychiatryEditor* editor) noexcept {
    editors_.erase(std::remove(editors_.begin(),editors_.end(),editor),editors_.end());
}
void Controller::setMotionEnabled(bool value) {
    motionEnabled_=value;
    for(auto* editor:editors_)editor->syncMotion();
}
void Controller::setZoomIndex(int index) {
    if(index<0||index>=static_cast<int>(kZoomFactors.size()))return;
    pendingZoom_=index==activeZoom_?-1:index;
    drainGeometry();
}
tresult Controller::requestEditorScale(PsychiatryEditor* editor,double scale) {
    editor->pendingScale_=scale;
    const bool deferred=applyingUi_;
    const auto generation=editor->attachmentGeneration_;
    drainGeometry();
    if(deferred)return kResultOk;
    return generation==editor->attachmentGeneration_&&
        std::abs(editor->contentScaleFactor-scale)<1.e-6?kResultOk:kResultFalse;
}
void Controller::drainGeometry() {
    if(applyingUi_)return;
    applyingUi_=true;
    // Serialize controller zoom and per-view DPI requests on the UI thread.
    // Retain each snapshot across synchronous host callbacks and use
    // registration serials to distinguish reused IPlugView attachments.
    for(unsigned pass=0;pass<6;++pass){
        const int index=pendingZoom_;
        pendingZoom_=-1;
        bool work=index>=0;
        if(index>=0){
            activeZoom_=index;
            const auto cutoff=registrationSerial_;
            const auto snapshot=editors_;
            for(auto* editor:snapshot)editor->addRef();
            bool accepted=snapshot.empty();
            for(auto* editor:snapshot){
                if(std::find(editors_.begin(),editors_.end(),editor)!=editors_.end()&&
                   editor->registrationSerial_<=cutoff)
                    accepted=editor->applyZoom(index)||accepted;
            }
            if(accepted)zoomIndex_=index;
            for(auto* editor:snapshot)editor->release();
            // New/reused views created during callbacks read the prior
            // committed zoom. Reconcile only those registrations now;
            // do not retry ordinary host refusals on existing views.
            const auto newcomers=editors_;
            for(auto* editor:newcomers)editor->addRef();
            for(auto* editor:newcomers){
                if(std::find(editors_.begin(),editors_.end(),editor)!=editors_.end()&&
                   editor->registrationSerial_>cutoff)
                    editor->applyZoom(zoomIndex_);
            }
            for(auto* editor:newcomers)editor->release();
            activeZoom_=-1;
        }
        const auto scales=editors_;
        for(auto* editor:scales)editor->addRef();
        for(auto* editor:scales){
            if(std::find(editors_.begin(),editors_.end(),editor)!=editors_.end()&&editor->pendingScale_>0.){
                const double scale=editor->pendingScale_;
                editor->pendingScale_=0.;
                work=true;
                editor->applyGeometry(editor->acceptedZoom_,scale);
            }
        }
        for(auto* editor:scales)editor->release();
        if(!work&&pendingZoom_<0)break;
    }
    // A continuously contradictory host cannot trap the UI thread.
    // Keep the last accepted geometry and discard unprocessed requests.
    for(auto* editor:editors_)editor->pendingScale_=0.;
    pendingZoom_=-1;activeZoom_=-1;applyingUi_=false;
}
void Controller::applyUiState(bool german,bool motion,int zoom) {
    germanUi_=german;
    setMotionEnabled(motion);
    setZoomIndex(zoom);
}

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
