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
#include <cstring>
#include "pluginterfaces/vst/ivsteditcontroller.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdint>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace psychiatry::vst3 {
using namespace Steinberg;
namespace {
constexpr Steinberg::int32 kLossOfControlUiTag=9000;
constexpr double kFixedEditorScale=.625; // 960 x 640 at host DPI 1
constexpr Steinberg::int32 kMotionUiTag=9002;
constexpr Steinberg::int32 kLanguageStateMagic=0x50535955; // 'PSYU'
constexpr Steinberg::int32 kLanguageStateVersion=4;
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

// Controller-only UI state, separate from all audio parameters.
// No zoom field or development-version migration.
tresult PLUGIN_API Controller::getState(IBStream* state) {
    if(!state)return kResultFalse;
    StateStream stream(state);
    if(!stream.writeInt32(kLanguageStateMagic)||
       !stream.writeInt32(kLanguageStateVersion)||
       !stream.writeInt32(germanUi_?1:0)||
       !stream.writeInt32(motionEnabled_?1:0))return kResultFalse;
    return kResultOk;
}
tresult PLUGIN_API Controller::setState(IBStream* state) {
    if(!state)return kResultFalse;
    StateStream stream(state);
    int32 magic=0,version=0,lang=0,motion=0;
    if(stream.empty()) {
        applyUiState(false,false);
        return kResultOk;
    }
    if(!stream.readInt32(magic)||magic!=kLanguageStateMagic||
       !stream.readInt32(version)||version!=kLanguageStateVersion||
       !stream.readInt32(lang)||(lang!=0&&lang!=1)||
       !stream.readInt32(motion)||(motion!=0&&motion!=1))return kResultFalse;
    applyUiState(lang==1,motion!=0);
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

// Fixed editor geometry: no user zoom, no project-driven resizing, no
// cross-view resize registry. Only host-supplied DPI may change the size.
class PsychiatryEditor final : public VSTGUI::VST3Editor, public VSTGUI::VST3EditorDelegate {
public:
    explicit PsychiatryEditor(Controller* parent)
        : VSTGUI::VST3Editor(parent,"view","psychiatry.uidesc"), parent_(parent) {
        setDelegate(this);
        setAllowedZoomFactors({});
        setZoomFactor(kFixedEditorScale);
        setRect(pixelRect());
    }
    VSTGUI::CView* createCustomView(VSTGUI::UTF8StringPtr name,
            const VSTGUI::UIAttributes&,const VSTGUI::IUIDescription*,VSTGUI::VST3Editor*) override {
        if(!name)return nullptr;
        if(std::strcmp(name,"PSYCHIATRY_OBSERVATION")==0)return new ObservationWindow(parent_);
        if(std::strcmp(name,"PSYCHIATRY_LOSS")==0)return new RasterLossButton;
        return nullptr;
    }
    void didOpen(VSTGUI::VST3Editor*) override {open_=true;syncMotion();}
    void willClose(VSTGUI::VST3Editor*) override {open_=false;parent_->editorClosed(this);}
    ~PsychiatryEditor() override {parent_->editorClosed(this);}
    void syncMotion() {
        if(open_&&motionControl_){
            motionControl_->setValueNormalized(parent_->isMotionEnabled()?1.f:0.f);
            motionControl_->invalid();
        }
    }
protected:
    tresult PLUGIN_API canResize() override {return kResultFalse;}
    tresult PLUGIN_API checkSizeConstraint(ViewRect* size) override {
        if(!size)return kInvalidArgument;
        const auto wanted=pixelRect();
        size->right=size->left+wanted.getWidth();
        size->bottom=size->top+wanted.getHeight();
        return kResultTrue;
    }
    tresult PLUGIN_API getSize(ViewRect* size) override {
        if(!size)return kInvalidArgument;
        // The canonical detached rectangle depends only on the fixed
        // logical size and current per-view DPI. Do not rely on SDK caches.
        if(!getFrame()&&!building_&&!dpiBusy_&&!attaching_){
            *size=pixelRect();setRect(*size);return kResultOk;
        }
        // Keep the last advertised size during an outstanding host resize.
        return Steinberg::Vst::VSTGUIEditor::getSize(size);
    }
    tresult PLUGIN_API attached(void* parent,FIDString type) override {
        if(building_||open_||getFrame()||!type||
           isPlatformTypeSupported(type)!=kResultTrue||!plugFrame)return kResultFalse;
        VSTGUI::SharedPointer<PsychiatryEditor> keepEditor(this);
        const auto generation=++generation_;
        attaching_=true;
        IPtr<IPlugFrame> host=plugFrame;
        attachHost_=host;
        auto proxy=Steinberg::owned(new AttachGeometryFrame(host));
        building_=true;attachCanceled_=false;
        plugFrame=proxy.get();
        nonEditRect=VSTGUI::CRect();sizeRequest={};
        const auto wanted=pixelRect();
        setRect(wanted);
        const auto result=Steinberg::Vst::VSTGUIEditor::attached(parent,type);
        building_=false;
        plugFrame=attachHost_;attachHost_=nullptr;
        if(result!=kResultOk||attachCanceled_||!open_||!getFrame()||
           !getFrame()->isAttached()||!getFrame()->hasChildren()||
           plugFrame.get()!=host.get()||!frameMatches(wanted)){
            if(open_||getFrame())Steinberg::Vst::VSTGUIEditor::removed();
            attaching_=false;setRect(pixelRect());return kResultFalse;
        }
        // Win32 hosts can allocate the parent from getSize(), then report
        // DPI before setFrame()/attached(). The last advertised rectangle
        // is not evidence of the actual client area. Publish measured host
        // geometry during negotiation so equality-shortcut hosts resize.
        ViewRect actualParent{};
        const bool measured=readParentRect(parent,actualParent);
        if(!measured||!containsDimensions(actualParent,wanted)){
            setRect(measured?actualParent:hostRect_);
            auto request=wanted;
            const auto accepted=host->resizeView(this,&request);
            if(generation!=generation_)return kResultFalse;
            if(accepted!=kResultTrue||!open_||!getFrame()||
               plugFrame.get()!=host.get()||!equalDimensions(request,wanted)||!frameMatches(wanted)||
               !parentCanContain(parent,wanted)){
                if(open_||getFrame())Steinberg::Vst::VSTGUIEditor::removed();
                attaching_=false;setRect(pixelRect());return kResultFalse;
            }
        }
        setRect(wanted);hostRect_=wanted;
        attaching_=false;parent_->editorOpened(this);syncMotion();
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
        if(host&&!getFrame()&&plugFrame.get()!=host)hostRect_=getRect();
        return Steinberg::Vst::VSTGUIEditor::setFrame(host);
    }
    void PLUGIN_API close() override {
        if(!getFrame())return;
        ++generation_;attaching_=false;
        VSTGUI::VST3Editor::close();
        // Pinned SDK retains its pointer when a callback owns a CFrame ref.
        // Never let an older frame pointer survive a completed removal.
        frame=nullptr;
        setRect(pixelRect());
    }
    bool beforeSizeChange(const VSTGUI::CRect& next,const VSTGUI::CRect&) override {
        if(building_)return true;
        if(!dpiBusy_||!plugFrame)return false;
        const auto wanted=pixelRect();
        if(std::abs(next.getWidth()-wanted.getWidth())>=1.||
           std::abs(next.getHeight()-wanted.getHeight())>=1.)return false;
        const auto generation=generation_;
        IPtr<IPlugFrame> host=plugFrame;
        auto request=wanted;
        const auto result=host->resizeView(this,&request);
        return generation==generation_&&plugFrame.get()==host.get()&&
            result==kResultTrue&&equalDimensions(request,wanted);
    }
    tresult PLUGIN_API onSize(ViewRect* size) override {
        if(!size)return kInvalidArgument;
        // Fixed size: acknowledge only the canonical pixel dimensions.
        // Never feed rounded pixels or stale host confirmations into the
        // fractional CFrame transform (the source of accumulated drift).
        if(!equalDimensions(*size,pixelRect()))return kResultFalse;
        setRect(pixelRect());hostRect_=getRect();return kResultOk;
    }
#ifdef VST3_CONTENT_SCALE_SUPPORT
    tresult PLUGIN_API setContentScaleFactor(ScaleFactor factor) override {
        if(!std::isfinite(factor)||factor<=0.||minSize.y*kFixedEditorScale*factor<1.||
           minSize.x*kFixedEditorScale*factor>2147483647.)return kInvalidArgument;
        if(factor==contentScaleFactor)return kResultOk;
        // Explicitly refuse a second DPI mutation during an outstanding
        // transaction. There is no deferred or cross-view zoom work.
        if(building_||attaching_||dpiBusy_)return kResultFalse;
        VSTGUI::SharedPointer<PsychiatryEditor> keepEditor(this);
        VSTGUI::SharedPointer<VSTGUI::CFrame> keepFrame(getFrame());
        const auto oldScale=contentScaleFactor;
        const auto oldRect=getRect(),oldHostRect=hostRect_;
        const auto generation=generation_;
        contentScaleFactor=factor;
        const auto wanted=pixelRect();
        IPtr<IPlugFrame> host=plugFrame;
        if(!keepFrame&&!host){setRect(wanted);return kResultOk;}
        dpiBusy_=true;
        bool accepted=false;
        if(keepFrame){
            accepted=keepFrame->setZoom(getAbsScaleFactor());
        }else{
            setRect(oldHostRect);
            auto request=wanted;
            accepted=host->resizeView(this,&request)==kResultTrue&&equalDimensions(request,wanted);
        }
        dpiBusy_=false;
        if(generation!=generation_||getFrame()!=keepFrame.get()){
            // A nested attachment, not the old request, owns the view.
            return open_&&getFrame()&&!attaching_&&contentScaleFactor==factor&&
                frameMatches(wanted)?kResultOk:kResultFalse;
        }
        if(!accepted||plugFrame.get()!=host.get()){
            contentScaleFactor=oldScale;setRect(oldRect);hostRect_=oldHostRect;
            return kResultFalse;
        }
        setRect(wanted);hostRect_=wanted;return kResultOk;
    }
#endif
    void valueChanged(VSTGUI::CControl* control) override {
        if(control&&control->getTag()==kMotionUiTag){
            if(open_&&control->isEditing())parent_->setMotionEnabled(control->getValueNormalized()>.5f);
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
                control->setValueNormalized(parent_->isMotionEnabled()?1.f:0.f);
                control->invalid();
            }
        }
    }
    void onViewRemoved(VSTGUI::CFrame* frame,VSTGUI::CView* view) override {
        if(view==motionControl_)motionControl_=nullptr;
        VSTGUI::VST3Editor::onViewRemoved(frame,view);
    }
private:
    static bool equalDimensions(const ViewRect& a,const ViewRect& b){
        return a.getWidth()==b.getWidth()&&a.getHeight()==b.getHeight();
    }
    ViewRect pixelRect() const {
        return ViewRect(0,0,static_cast<int32>(std::floor(960.*contentScaleFactor)),
                            static_cast<int32>(std::floor(640.*contentScaleFactor)));
    }
    bool frameMatches(const ViewRect& wanted) {
        if(!getFrame())return false;
        VSTGUI::CRect native;
        return std::abs(getFrame()->getWidth()-wanted.getWidth())<1.&&
            std::abs(getFrame()->getHeight()-wanted.getHeight())<1.&&
            getFrame()->getSize(native)&&
            std::abs(native.getWidth()-wanted.getWidth())<1.&&
            std::abs(native.getHeight()-wanted.getHeight())<1.;
    }
    static bool containsDimensions(const ViewRect& actual,const ViewRect& wanted){
        return actual.getWidth()>=wanted.getWidth()&&actual.getHeight()>=wanted.getHeight();
    }
    static bool readParentRect(void* parent,ViewRect& result){
#ifdef _WIN32
        RECT client{};
        if(!parent||!GetClientRect(static_cast<HWND>(parent),&client))return false;
        result=ViewRect(0,0,client.right-client.left,client.bottom-client.top);
        return true;
#else
        (void)parent;(void)result;return false;
#endif
    }
    static bool parentCanContain(void* parent,const ViewRect& wanted){
#ifdef _WIN32
        ViewRect actual{};
        return readParentRect(parent,actual)&&containsDimensions(actual,wanted);
#else
        // Native parent inspection is implemented for the Windows target.
        // Other platforms still depend on the host's resize result.
        (void)parent;(void)wanted;return true;
#endif
    }
    Controller* parent_;
    VSTGUI::CControl* motionControl_=nullptr;
    bool open_=false,building_=false,attachCanceled_=false,attaching_=false,dpiBusy_=false;
    IPtr<IPlugFrame> attachHost_;
    ViewRect hostRect_{};
    std::uint64_t generation_=0;
};

// The controller tracks open editors only for MOTION synchronization;
// no geometry or user zoom state remains here.
void Controller::editorOpened(PsychiatryEditor* editor) {
    if(std::find(editors_.begin(),editors_.end(),editor)==editors_.end())
        editors_.push_back(editor);
}
void Controller::editorClosed(PsychiatryEditor* editor) noexcept {
    editors_.erase(std::remove(editors_.begin(),editors_.end(),editor),editors_.end());
}
void Controller::setMotionEnabled(bool value) {
    motionEnabled_=value;
    for(auto* editor:editors_)editor->syncMotion();
}
void Controller::applyUiState(bool german,bool motion) {
    germanUi_=german;
    setMotionEnabled(motion);
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
