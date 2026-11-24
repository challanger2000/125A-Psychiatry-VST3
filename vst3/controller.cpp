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
constexpr Steinberg::int32 kLanguageUiTag=9001;
constexpr Steinberg::int32 kMotionUiTag=9002;
constexpr Steinberg::int32 kLanguageStateMagic=0x50535955; // 'PSYU'
constexpr Steinberg::int32 kLanguageStateVersion=2;
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
       !stream.writeInt32(motionEnabled_?1:0))return kResultFalse;
    return kResultOk;
}
tresult PLUGIN_API Controller::setState(IBStream* state) {
    if(!state)return kResultFalse;
    IBStreamer stream(state,kLittleEndian);
    int32 magic=0,version=0,lang=0,motion=0;
    if(!stream.readInt32(magic)) {
        germanUi_=false;
        motionEnabled_=false; // no legacy UI state => safely static CRT
        return kResultOk;
    }
    if(magic!=kLanguageStateMagic||
       !stream.readInt32(version)||(version!=1&&version!=2)||
       !stream.readInt32(lang)||(lang!=0&&lang!=1))return kResultFalse;
    if(version==2 && (!stream.readInt32(motion)||(motion!=0&&motion!=1)))
        return kResultFalse;
    germanUi_=(lang==1);
    motionEnabled_=(motion!=0);
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

// P009 provisional audiovisual diagnostic monitor.
// Entirely original vector graphics: no patient photography, no borrowed
// imagery, no disk I/O, no DSP/audio callback, and no external textures.
// Rendered on the UI thread, max ~30 Hz via VSTGUI CView::onIdle.
class DiagnosticMonitor final : public VSTGUI::CView {
public:
    explicit DiagnosticMonitor(Controller* controller)
        : VSTGUI::CView(VSTGUI::CRect(0,0,288,325)), controller_(controller) {
        static constexpr const char* names[]{"scene-1.png","scene-2.png","scene-3.png",
            "scene-4.png","scene-5.png","scene-6.png","scene-7.png","scene-8.png","scene-9.png"};
        for(std::size_t i=0;i<scenes_.size();++i)
            scenes_[i]=VSTGUI::owned(new VSTGUI::CBitmap(VSTGUI::CResourceDescription(names[i])));
        model_=readModel();
        motion_=controller_->isMotionEnabled();
        german_=controller_->isGermanUi();
        setWantsIdle(true);
    }
    void onIdle() override {
        const auto next=readModel();
        const bool motion=controller_->isMotionEnabled(),german=controller_->isGermanUi();
        const bool changed=!(next==model_)||motion!=motion_||german!=german_;
        model_=next;motion_=motion;german_=german;
        // In reduced-motion mode the frame is truly static, including after
        // switching off motion or reopening the editor with saved settings.
        if(motion_&&model_.peak>0.f) {
            tick_=(tick_+1)%1200u;
            if(changed||tick_%3==0)invalid();
        } else {
            const bool reset=tick_!=0;
            tick_=0;
            if(changed||reset)invalid();
        }
    }
    void draw(VSTGUI::CDrawContext* dc) override {
        using namespace VSTGUI;
        const auto r=getViewSize();
        const auto w=r.getWidth(),h=r.getHeight();
        auto x=[&](double n){return r.left+n*w;};
        auto y=[&](double n){return r.top+n*h;};
        // True monochrome CRT; the controls outside remain orange-lit.
        const CColor black{8,8,8,255}, dark{21,21,21,255};
        const CColor green{183,183,183,255}, faint{75,75,75,255};
        const CColor bright{224,224,224,255}, dim{36,36,36,255};
        auto fill=[&](double l,double t,double rr,double bb,const CColor& color){
            dc->setFillColor(color);dc->drawRect(CRect(x(l),y(t),x(rr),y(bb)),kDrawFilled);
        };
        auto stroke=[&](double x1,double y1,double x2,double y2,const CColor& color,double thickness=1.){
            dc->setFrameColor(color);dc->setLineWidth(thickness);
            dc->drawLine(CPoint(x(x1),y(y1)),CPoint(x(x2),y(y2)));
        };
        fill(0,0,1,1,black);
        fill(.035,.035,.965,.962,dark);
        fill(.055,.066,.945,.928,black);
        // Scene selection shares the same twelve real macro control values
        // as the FX processor; when several are active, the strongest is
        // the dominant image, other effects contribute contamination.
        const int chosen=model_.dominant;
        const float top=model_.peak,total=model_.total;
        const float intensity=top;
        const double phase=static_cast<double>(tick_)*0.04;
        // Film gate corners and registration marks.
        for(double sy:{.095,.905}){
            stroke(.08,sy,.16,sy,green);
            stroke(.84,sy,.92,sy,green);
        }
        stroke(.5,.074,.5,.098,faint);
        stroke(.5,.902,.5,.925,faint);
        // The existing nine diagnostic bitmaps remain temporary. Reuse
        // three of them for new worlds until the owner-approved art pass.
        // Logical world index and physical bitmap index are independent.
        constexpr std::array<std::size_t,MonitorModel::worldCount> previewScene{
            0,1,2,3,4,5,6,7,8,2,5,8};
        const auto sceneIndex=previewScene[static_cast<std::size_t>(chosen)];
        const CRect imageRect(x(.5)-112,y(.14),x(.5)+112,y(.14)+196);
        auto* scene=scenes_[sceneIndex].get();
        if(scene&&scene->isLoaded()){
            // Original grayscale perspective-rendered rooms/equipment. All
            // decoding is editor-side, no external files or audio-thread work.
            scene->draw(dc,imageRect);
            if(intensity>0.f){
                // Bounded visual cross-exposure from the second strongest
                // macro, with independent scene geometry rather than RNG.
                const int secondWorld=model_.secondary();
                if(secondWorld>=0){
                    const auto index=static_cast<std::size_t>(secondWorld);
                    const auto second=previewScene[index];
                    if(second!=sceneIndex&&scenes_[second]&&scenes_[second]->isLoaded())
                        scenes_[second]->draw(dc,imageRect,CPoint(0,0),.23f*model_.levels[index]);
                }
                // Subtle local analog horizontal displacement; do not flash
                // the full picture. Still mode uses a fixed phase.
                const int strips=chosen==8?5:chosen==6?3:chosen==1?2:1;
                for(int j=0;j<strips;++j){
                    const double row=18.+std::fmod(j*47.+phase*1.1,150.);
                    const double shift=std::round(10.*intensity*std::sin(j*2.4+phase*.17+.8));
                    const double left=std::max(0.,shift),right=224.+std::min(0.,shift);
                    if(right>left)scene->draw(dc,CRect(imageRect.left+left,imageRect.top+row,
                        imageRect.left+right,imageRect.top+row+5.),CPoint(left-shift,row),.55f);
                }
            }
        }else if(chosen==0 || top<.08f){ // vector fallback if resource loading fails
            fill(.11,.14,.89,.84,dim);
            for(int i=0;i<6;++i) {
                const double d=static_cast<double>(i)/6.;
                const double ll=.10+.33*d,rr=.90-.33*d;
                stroke(ll,.15+.30*d,ll,.84-.18*d,faint);
                stroke(rr,.15+.30*d,rr,.84-.18*d,faint);
                stroke(.10,.15+.30*d,.90,.15+.30*d,faint);
                stroke(.10,.84-.18*d,.90,.84-.18*d,faint);
            }
            fill(.455,.44,.545,.73,black);
            stroke(.455,.44,.545,.44,green);
            if(intensity>.18f)fill(.469,.465,.485,.55,bright);
        }else if(chosen==1){ // surveillance plate: tracking gaze
            for(int i=0;i<7;++i){
                const double t=.16+i*.094;
                stroke(.11,t,.89,t,faint);
            }
            const double shift=.07*intensity*std::sin(phase*.24);
            stroke(.25+shift,.34,.75+shift,.34,green,2);
            stroke(.25+shift,.68,.75+shift,.68,green,2);
            stroke(.25+shift,.34,.25+shift,.68,green);
            stroke(.75+shift,.34,.75+shift,.68,green);
            fill(.47+shift,.49,.53+shift,.54,bright);
        }else if(chosen==2){ // double exposed diagnostic radiograph
            for(int copy=0;copy<2;++copy){
                const double ofs=copy==0?-.075*intensity:.075*intensity;
                const auto col=copy==0?faint:green;
                for(int i=0;i<6;++i){
                    const double py=.21+i*.103;
                    stroke(.29+ofs,py,.71+ofs,py,col);
                    stroke(.29+ofs,py,.42+ofs,py+.058,col);
                    stroke(.71+ofs,py,.58+ofs,py+.058,col);
                }
                stroke(.50+ofs,.14,.50+ofs,.83,col,2);
            }
        }else if(chosen==3){ // manic accelerating analog meter
            for(int i=0;i<28;++i){
                const double xx=.11+i*.028;
                const double hh=.13+.22*std::abs(std::sin(i*.91+phase*.33));
                stroke(xx,.70,xx,.70-hh,i%3==0?bright:faint);
            }
            const double v=.14+.7*(.5+.5*std::sin(phase*.2));
            stroke(v,.18,v,.84,green,2);
        }else if(chosen==4){ // electrical examination / EEG pulse
            double px=.10,py=.55;
            for(int i=1;i<=70;++i){
                const double xx=.10+(.80*i)/70.;
                const double pulse=(i%18==8?-.30*intensity:i%18==10?.24*intensity:0.);
                const double yy=.55+pulse+.035*std::sin(i*.42+phase*.15);
                stroke(px,py,xx,yy,i%9==0?bright:green);
                px=xx;py=yy;
            }
            stroke(.18,.75,.82,.75,faint);
        }else if(chosen==5){ // isolation / empty observation room
            fill(.18,.19,.82,.82,dim);
            for(int i=0;i<5;++i){
                const double z=i*.043;
                stroke(.18+z,.19+z,.82-z,.19+z,faint);
                stroke(.18+z,.19+z,.18+z,.82-z,faint);
                stroke(.82-z,.19+z,.82-z,.82-z,faint);
            }
            stroke(.48,.33,.52,.33,bright,2);
            stroke(.50,.33,.50,.69,green);
        }else if(chosen==6){ // repeated institutional record cycles
            for(int j=0;j<7;++j){
                const double cy=.15+j*.102;
                stroke(.13,cy,.86,cy,faint);
                for(int i=0;i<12;++i){
                    const double xx=.16+i*.055;
                    const double hv=.013+.065*std::abs(std::sin(phase*.18+i*.6+j*.45));
                    stroke(xx,cy,xx,cy+hv,i%3==0?green:faint);
                }
            }
        }else if(chosen==7){ // shadow echo in empty ward
            for(int k=0;k<3;++k){
                const double offset=(k-1)*.065*intensity;
                const auto color=k==1?green:faint;
                stroke(.5+offset,.23,.40+offset,.40,color,2);
                stroke(.5+offset,.23,.60+offset,.40,color,2);
                stroke(.40+offset,.40,.36+offset,.72,color);
                stroke(.60+offset,.40,.64+offset,.72,color);
                stroke(.36+offset,.72,.64+offset,.72,color);
            }
        }else{ // fragmented signal/clinical chart
            for(int i=0;i<13;++i){
                const double t=.12+i*.055;
                const double displacement=.09*intensity*std::sin(i*3.1+phase*.11);
                stroke(.12+displacement,t,.78-displacement,t+.14,green);
                if(i%3==0)stroke(.22,t+.14,.84,t+.03,bright);
            }
        }
        // Continuous CRT signal strength strip. It MUST visibly respond
        // even when DEPRESSION remains the dominant scene both before and
        // after a randomization (a previous GUI smoke test exposed this).
        // 0% is an empty black strip; any active macro fills by intensity.
        fill(.12,.758,.88,.830,black);
        fill(.14,.769,.14+.72*static_cast<double>(intensity),.817,green);
        stroke(.12,.758,.88,.758,faint);
        stroke(.12,.830,.88,.830,faint);

        // Cross-macro interference: extra non-dominant worlds affect density.
        const float contamination=std::clamp(total-top,0.f,4.f);
        const int bands=static_cast<int>(std::min(15.f,contamination*3.f));
        for(int i=0;i<bands;++i){
            const double yy=.12+std::fmod((i*0.173+tick_*.0007),.75);
            stroke(.09,yy,.92,yy,i%3==0?green:faint);
        }
        // Restrained scanlines over the photographic-style artwork.
        for(int i=0;i<84;++i){
            const double yy=.104+i*.0095;
            stroke(.062,yy,.938,yy,i%5==0?CColor{12,12,12,72}:CColor{0,0,0,30});
        }
        fill(.075,.848,.925,.887,black);
        for(std::size_t i=0;i<model_.levels.size();++i){
            const double xx=.12+i*.064;
            fill(xx,.860,xx+.042,.878,dim);
            const float level=model_.levels[i];
            if(level>0.f)fill(xx,.860,xx+.042*level,.878,green);
        }
        static constexpr const char* sceneEnglish[]{
            "WARD / CORRIDOR","OBSERVATION","DOUBLE EXPOSURE",
            "ACTIVITY METER","ELECTRICAL TRACE","ISOLATION ROOM",
            "REPEATED RECORD","SHADOW ECHO","SIGNAL FRACTURE",
            "DELIRIUM","CATATONIA","AMNESIA"
        };
        static constexpr const char* sceneGerman[]{
            "LEERER FLUR","BEOBACHTUNG","DOPPELBELICHTUNG",
            "AKTIVITAET","ELEKTRISCHE KURVE","ISOLATIONSRAUM",
            "WIEDERHOLUNG","SCHATTEN-ECHO","SIGNALBRUCH",
            "DELIRIUM","KATATONIE","AMNESIE"
        };
        const bool german=controller_->isGermanUi();
        char display[96]{};
        if(model_.bypass)
            std::snprintf(display,sizeof(display),"BYPASS  /  %s",german?"ORIGINAL":"DRY");
        else if(model_.wet==0.f)
            std::snprintf(display,sizeof(display),"%s  /  %s",german?"THERAPIE":"THERAPY",german?"ORIGINAL":"DRY");
        else if(top==0.f)
            std::snprintf(display,sizeof(display),"%s",german?"BEREIT  /  0%":"STANDBY  /  0%");
        else
            std::snprintf(display,sizeof(display),"%s %02d  %s  %02d%%",
                          german?"KAN":"CH",chosen+1,
                          german?sceneGerman[chosen]:sceneEnglish[chosen],
                          static_cast<int>(std::lround(intensity*100.f)));
        dc->setFont(VSTGUI::kNormalFontSmall);
        dc->setFontColor(bright);
        dc->drawString(display,CRect(x(.075),y(.085),x(.925),y(.125)),
                       VSTGUI::kCenterText,true);
        dc->setFrameColor(green);
        dc->setLineWidth(1.);
        dc->drawRect(CRect(x(.036),y(.036),x(.964),y(.963)),kDrawStroked);
        setDirty(false);
    }
private:
    Controller* controller_;
    MonitorModel readModel() const {
        static_assert(kMacroIds.size()==MonitorModel::worldCount);
        std::array<double,MonitorModel::worldCount> macros{};
        for(std::size_t i=0;i<macros.size();++i)
            macros[i]=controller_->getParamNormalized(kMacroIds[i]);
        return MonitorModel::make(macros,controller_->getParamNormalized(kTherapy),
                                 controller_->getParamNormalized(kBypass));
    }
    std::array<VSTGUI::SharedPointer<VSTGUI::CBitmap>,9> scenes_{};
    MonitorModel model_{};
    bool motion_=false,german_=false;
    unsigned tick_=0;
};

// This is a host-embedded editor, not a stand-alone UI.
// Its private UI-only tag (9000) does not enter the VST3 parameter list.
class PsychiatryEditor final : public VSTGUI::VST3Editor, public VSTGUI::VST3EditorDelegate {
public:
    explicit PsychiatryEditor(Controller* parent)
        : VSTGUI::VST3Editor(parent,parent->isGermanUi()?"view_de":"view","psychiatry.uidesc"), parent_(parent) {
        setDelegate(this);
    }
    VSTGUI::CView* createCustomView(VSTGUI::UTF8StringPtr name,
            const VSTGUI::UIAttributes&,
            const VSTGUI::IUIDescription*,
            VSTGUI::VST3Editor*) override {
        if(name && std::strcmp(name,"PSYCHIATRY_DIAGNOSTIC")==0)
            return new DiagnosticMonitor(parent_);
        return nullptr;
    }
protected:
    void valueChanged(VSTGUI::CControl* control) override {
        if(control && control->getTag()==kMotionUiTag) {
            parent_->setMotionEnabled(control->getValueNormalized()>0.5f);
            return;
        }
        if(control && control->getTag()==kLanguageUiTag) {
            if(control->getValueNormalized()>0.5f) {
                const bool german=!parent_->isGermanUi();
                if(exchangeView(german?"view_de":"view"))
                    parent_->setGermanUi(german);
            }
            return;
        }
        if(control && control->getTag()==kLossOfControlUiTag) {
            // VSTGUI CTextButton with kick-style sends pressed and released
            // values. Trigger once on press only, not twice per click.
            if(control->getValueNormalized()>0.5f)
                parent_->triggerLossOfControl();
            return;
        }
        VSTGUI::VST3Editor::valueChanged(control);
    }
    void onViewAdded(VSTGUI::CFrame* frame,VSTGUI::CView* view) override {
        VSTGUI::VST3Editor::onViewAdded(frame,view);
        if(auto* control=dynamic_cast<VSTGUI::CControl*>(view)) {
            if(control->getTag()==kMotionUiTag){
                control->setValueNormalized(parent_->isMotionEnabled()?1.f:0.f);
                control->invalid();
            }
        }
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
