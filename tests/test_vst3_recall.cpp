// P006: exercise the *actual* VST3 processor/controller state interfaces.
// Runs on the Windows Steinberg SDK build, not a duplicated serialization
// reimplementation. The final Studio Pro project save/reopen test remains due.
#include "vst3/processor.h"
#include "vst3/controller.h"
#include "vst3/ids.h"
#include "vst3/monitor_model.h"

#include "public.sdk/source/vst/utility/memoryibstream.h"
#include "base/source/fstreamer.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <set>
#include <vector>
#include <atomic>
#include "pluginterfaces/vst/ivsteditcontroller.h"

#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"P006 state FAIL %s:%d %s\n",__FILE__,__LINE__,#x);std::abort();}}while(0)
using namespace psychiatry::vst3;
using namespace Steinberg;

// The SDK's standalone test executable is not a VST3 DLL; it has no DllMain.
// Provide only the module-handle symbol expected by VSTGUI support.
// The real plugin bundle receives its module handle from Steinberg's DLL entry.
void* moduleHandle = nullptr;

// Minimal host spy: verifies that UI randomization is reported to the real
// Steinberg IComponentHandler interfaces, with one grouped edit and nine
// begin/perform/end cycles. It does not pretend to be a DAW.
struct HandlerSpy final : Vst::IComponentHandler, Vst::IComponentHandler2 {
    std::atomic<uint32> refs{1};
    struct E {char type; Vst::ParamID id; double value;};
    std::vector<E> events;
    int groupStarts=0,groupFinishes=0;
    tresult PLUGIN_API queryInterface(const TUID iid,void** out) override {
        if(!out)return kInvalidArgument;
        *out=nullptr;
        if(FUnknownPrivate::iidEqual(iid,FUnknown::iid)||
           FUnknownPrivate::iidEqual(iid,Vst::IComponentHandler::iid)) {
            *out=static_cast<Vst::IComponentHandler*>(this);
        } else if(FUnknownPrivate::iidEqual(iid,Vst::IComponentHandler2::iid)) {
            *out=static_cast<Vst::IComponentHandler2*>(this);
        } else return kNoInterface;
        addRef();
        return kResultOk;
    }
    uint32 PLUGIN_API addRef() override {return ++refs;}
    uint32 PLUGIN_API release() override {
        auto count=--refs;
        if(count==0)delete this;
        return count;
    }
    tresult PLUGIN_API beginEdit(Vst::ParamID id) override {
        events.push_back({'B',id,0.});return kResultOk;
    }
    tresult PLUGIN_API performEdit(Vst::ParamID id,Vst::ParamValue value) override {
        events.push_back({'P',id,value});return kResultOk;
    }
    tresult PLUGIN_API endEdit(Vst::ParamID id) override {
        events.push_back({'E',id,0.});return kResultOk;
    }
    tresult PLUGIN_API restartComponent(int32) override {return kResultOk;}
    tresult PLUGIN_API setDirty(TBool) override {return kResultOk;}
    tresult PLUGIN_API requestOpenEditor(FIDString) override {return kResultOk;}
    tresult PLUGIN_API startGroupEdit() override {++groupStarts;return kResultOk;}
    tresult PLUGIN_API finishGroupEdit() override {++groupFinishes;return kResultOk;}
};

static void writeLegacy(ResizableMemoryIBStream& memory,int version,bool invalid=false){
    IBStreamer stream(&memory,kLittleEndian);
    CHECK(stream.writeInt32(kStateMagic));
    CHECK(stream.writeInt32(version));
    const float dep=invalid?std::numeric_limits<float>::quiet_NaN():0.13f;
    if(version==1) {
        CHECK(stream.writeFloat(0.37f));     // paranoia
        CHECK(stream.writeFloat(0.82f));     // therapy
    } else if(version==2) {
        CHECK(stream.writeFloat(dep));       // depression
        CHECK(stream.writeFloat(0.37f));     // paranoia
        CHECK(stream.writeFloat(0.61f));     // obsession
        CHECK(stream.writeFloat(0.82f));     // therapy
    } else if(version==3) {
        CHECK(stream.writeFloat(dep));       // depression
        CHECK(stream.writeFloat(0.37f));     // paranoia
        CHECK(stream.writeFloat(0.29f));     // mania
        CHECK(stream.writeFloat(0.48f));     // panic
        CHECK(stream.writeFloat(0.61f));     // obsession
        CHECK(stream.writeFloat(0.73f));     // psychosis
        CHECK(stream.writeFloat(0.82f));     // therapy
    } else if(version>=4) {
        CHECK(stream.writeFloat(dep));       // depression
        CHECK(stream.writeFloat(0.37f));     // paranoia
        CHECK(stream.writeFloat(0.24f));     // schizophrenia
        CHECK(stream.writeFloat(0.29f));     // mania
        CHECK(stream.writeFloat(0.48f));     // panic
        CHECK(stream.writeFloat(0.39f));     // dissociation
        CHECK(stream.writeFloat(0.61f));     // obsession
        CHECK(stream.writeFloat(0.56f));     // hallucination
        CHECK(stream.writeFloat(0.73f));     // psychosis
        CHECK(stream.writeFloat(0.82f));     // therapy
    }
    CHECK(stream.writeInt32(1));             // bypass
    if(version>=5){CHECK(stream.writeFloat(.18f));CHECK(stream.writeFloat(.42f));CHECK(stream.writeFloat(.67f));}
    memory.rewind();
}
static void equals(float actual,float expected){
    CHECK(std::abs(actual-expected)<1.e-6f);
}
int main(){
    Processor component;
    Controller controller;
    CHECK(component.initialize(nullptr)==kResultOk);
    CHECK(controller.initialize(nullptr)==kResultOk);
    // This is a non-GUI executable without a Win32 host platform/frame.
    // Creating a VSTGUI editor here would initialize the plugin UI outside
    // a valid host lifecycle. UI resource/binding inspection is performed
    // separately in the Windows packaging step; actual rendering in a DAW
    // remains a distinct acceptance test.

    constexpr std::array<Vst::ParamID,10> params{
        kDepression,kParanoia,kSchizophrenia,kMania,kPanic,
        kDissociation,kObsession,kHallucination,kPsychosis,kTherapy};
    for(int version=1;version<=5;++version) {
        ResizableMemoryIBStream state;
        writeLegacy(state,version);
        CHECK(component.setState(&state)==kResultOk);
        state.rewind();
        CHECK(controller.setComponentState(&state)==kResultOk);

        std::array<float,10> expected{
            version>=2?0.13f:0.f,
            0.37f,
            version>=4?0.24f:0.f,
            version>=3?0.29f:0.f,
            version>=3?0.48f:0.f,
            version>=4?0.39f:0.f,
            version>=2?0.61f:0.f,
            version>=4?0.56f:0.f,
            version>=3?0.73f:0.f,
            0.82f
        };
        for(std::size_t i=0;i<params.size();++i)
            equals(static_cast<float>(controller.getParamNormalized(params[i])),expected[i]);
        equals(static_cast<float>(controller.getParamNormalized(kBypass)),1.f);

        ResizableMemoryIBStream roundtrip;
        CHECK(component.getState(&roundtrip)==kResultOk);
        roundtrip.rewind();
        IBStreamer read(&roundtrip,kLittleEndian);
        int32 magic=0,outputVersion=0,bypass=0;
        CHECK(read.readInt32(magic)&&magic==kStateMagic);
        CHECK(read.readInt32(outputVersion)&&outputVersion==kStateVersion);
        for(std::size_t i=0;i<params.size();++i) {
            float v=-100.f;
            CHECK(read.readFloat(v));
            equals(v,expected[i]);
        }
        CHECK(read.readInt32(bypass)&&bypass==1);
        const std::array<Vst::ParamID,3> extra{kDelirium,kCatatonia,kAmnesia};
        const std::array<float,3> targets{.18f,.42f,.67f};
        for(std::size_t i=0;i<extra.size();++i){float v=-1.f;CHECK(read.readFloat(v));
            const float target=version==5?targets[i]:0.f;
            equals(v,target);equals(static_cast<float>(controller.getParamNormalized(extra[i])),target);}
        CHECK(roundtrip.getCursor()==64); // v4 prefix + 3 float32
    }
    {
        ResizableMemoryIBStream corrupt;
        writeLegacy(corrupt,5,true);
        CHECK(component.setState(&corrupt)==kResultFalse);
        corrupt.rewind();
        CHECK(controller.setComponentState(&corrupt)==kResultFalse);
    }
    {
        ResizableMemoryIBStream truncated;
        IBStreamer stream(&truncated,kLittleEndian);
        CHECK(stream.writeInt32(kStateMagic));
        CHECK(stream.writeInt32(kStateVersion));
        CHECK(stream.writeFloat(0.2f));
        truncated.rewind();
        CHECK(component.setState(&truncated)==kResultFalse);
        truncated.rewind();
        CHECK(controller.setComponentState(&truncated)==kResultFalse);
    }
    {
        ResizableMemoryIBStream future;
        IBStreamer stream(&future,kLittleEndian);
        CHECK(stream.writeInt32(kStateMagic));
        CHECK(stream.writeInt32(kStateVersion+100));
        future.rewind();
        CHECK(component.setState(&future)==kResultFalse);
        future.rewind();
        CHECK(controller.setComponentState(&future)==kResultFalse);
    }
    // New v5 tail is mandatory and must fail closed if truncated or nonfinite.
    {
        ResizableMemoryIBStream baseline;
        writeLegacy(baseline,5);
        CHECK(component.setState(&baseline)==kResultOk);
        baseline.rewind();CHECK(controller.setComponentState(&baseline)==kResultOk);
        for(int bad=0;bad<4;++bad) {
            ResizableMemoryIBStream corrupted;
            IBStreamer w(&corrupted,kLittleEndian);
            CHECK(w.writeInt32(kStateMagic)&&w.writeInt32(5));
            for(int i=0;i<10;++i)CHECK(w.writeFloat(.2f));
            CHECK(w.writeInt32(0));
            if(bad<3) {
                for(int i=0;i<bad;++i)CHECK(w.writeFloat(.9f));
            } else {
                CHECK(w.writeFloat(.9f)&&w.writeFloat(std::numeric_limits<float>::quiet_NaN())&&w.writeFloat(.9f));
            }
            corrupted.rewind();CHECK(component.setState(&corrupted)==kResultFalse);
            corrupted.rewind();CHECK(controller.setComponentState(&corrupted)==kResultFalse);
            equals(static_cast<float>(controller.getParamNormalized(kDelirium)),.18f);
            equals(static_cast<float>(controller.getParamNormalized(kCatatonia)),.42f);
            equals(static_cast<float>(controller.getParamNormalized(kAmnesia)),.67f);
            ResizableMemoryIBStream saved;
            CHECK(component.getState(&saved)==kResultOk);saved.rewind();
            IBStreamer r(&saved,kLittleEndian);int32 v=0;
            CHECK(r.readInt32(v)&&r.readInt32(v)&&v==5);
            float f=0.f;for(int i=0;i<10;++i)CHECK(r.readFloat(f));
            CHECK(r.readInt32(v)&&v==1);
            CHECK(r.readFloat(f));equals(f,.18f);
            CHECK(r.readFloat(f));equals(f,.42f);
            CHECK(r.readFloat(f));equals(f,.67f);
        }
    }
    // The controller randomizes the nine existing macro IDs only. Legacy
    // project values are valid, so we can check both preservation and change.
    {
        ResizableMemoryIBStream state;
        writeLegacy(state,4);
        CHECK(controller.setComponentState(&state)==kResultOk);
        const auto therapyBefore=controller.getParamNormalized(kTherapy);
        const auto bypassBefore=controller.getParamNormalized(kBypass);
        auto* spy=new HandlerSpy;
        CHECK(controller.setComponentHandler(spy)==kResultOk);
        controller.triggerLossOfControl();
        CHECK(spy->groupStarts==1 && spy->groupFinishes==1);
        CHECK(spy->events.size()==27);
        for(std::size_t i=0;i<9;++i) {
            CHECK(spy->events[i*3].type=='B' && spy->events[i*3+1].type=='P' &&
                  spy->events[i*3+2].type=='E');
            CHECK(spy->events[i*3].id==params[i]);
            CHECK(spy->events[i*3+1].id==params[i]);
            CHECK(spy->events[i*3+2].id==params[i]);
            CHECK(spy->events[i*3+1].value==controller.getParamNormalized(params[i]));
        }
        std::array<double,9> firstValues{};
        for(std::size_t i=0;i<9;++i) {
            const double value=controller.getParamNormalized(params[i]);
            CHECK(value>=0.0 && value<=0.78 && std::isfinite(value));
            firstValues[i]=value;
        }
        CHECK(controller.getParamNormalized(kTherapy)==therapyBefore);
        CHECK(controller.getParamNormalized(kBypass)==bypassBefore);
        controller.triggerLossOfControl();
        bool different=false;
        for(std::size_t i=0;i<9;++i) {
            const double value=controller.getParamNormalized(params[i]);
            CHECK(value>=0.0 && value<=0.78 && std::isfinite(value));
            if(value!=firstValues[i])different=true;
        }
        CHECK(different); // consecutive presses must not repeat every knob
        CHECK(controller.getParamNormalized(kTherapy)==therapyBefore);
        CHECK(controller.getParamNormalized(kBypass)==bypassBefore);
        CHECK(spy->groupStarts==2 && spy->groupFinishes==2);
        CHECK(spy->events.size()==54);
        CHECK(controller.setComponentHandler(nullptr)==kResultOk);
        spy->release();
    }
    // UI readouts display meaningful percentages, not raw 0.0000..1.0000.
    {
        Vst::String128 label{};
        CHECK(controller.getParamStringByValue(kDepression,0.37,label)==kResultOk);
        CHECK(label[0]=='3' && label[1]=='7' && label[2]==' ' && label[3]=='%');
        CHECK(controller.getParamStringByValue(kTherapy,1.0,label)==kResultOk);
        CHECK(label[0]=='1' && label[1]=='0' && label[2]=='0' && label[3]==' ' && label[4]=='%');
    }
    // Controller-only UI state roundtrip: language preference is persisted
    // independently of the processor's stable 52-byte v4 audio state.
    {
        ResizableMemoryIBStream saved;
        CHECK(!controller.isGermanUi());
        CHECK(!controller.isMotionEnabled());
        controller.setGermanUi(true);
        controller.setMotionEnabled(true);
        CHECK(controller.getState(&saved)==kResultOk);
        controller.setGermanUi(false);
        controller.setMotionEnabled(false);
        saved.rewind();
        CHECK(controller.setState(&saved)==kResultOk);
        CHECK(controller.isGermanUi());
        CHECK(controller.isMotionEnabled());
        controller.setGermanUi(false);
        controller.setMotionEnabled(false);
        ResizableMemoryIBStream savedEn;
        CHECK(controller.getState(&savedEn)==kResultOk);
        controller.setGermanUi(true);
        controller.setMotionEnabled(true);
        savedEn.rewind();
        CHECK(controller.setState(&savedEn)==kResultOk);
        CHECK(!controller.isGermanUi());
        CHECK(!controller.isMotionEnabled());
        // P009 compatibility: old v1 controller state only stored language.
        ResizableMemoryIBStream oldUi;
        IBStreamer legacy(&oldUi,kLittleEndian);
        CHECK(legacy.writeInt32(0x50535955));
        CHECK(legacy.writeInt32(1));
        CHECK(legacy.writeInt32(1));
        oldUi.rewind();
        controller.setMotionEnabled(true);
        CHECK(controller.setState(&oldUi)==kResultOk);
        CHECK(controller.isGermanUi());
        CHECK(!controller.isMotionEnabled());
    }
    // P022: the monitor represents effective macro settings, not audio level.
    {
        std::array<double,9> macros{};macros[3]=.8;macros[6]=.4;
        const auto full=MonitorModel::make(macros,0.,0.);
        CHECK(full.dominant==3&&full.peak==.8f&&full.levels[6]==.4f);
        const auto half=MonitorModel::make(macros,.5,0.);
        CHECK(half.dominant==3&&half.peak==.4f&&half.levels[6]==.2f);
        const auto dry=MonitorModel::make(macros,1.,0.);
        const auto bypass=MonitorModel::make(macros,0.,1.);
        CHECK(dry.peak==0.f&&dry.total==0.f&&dry.wet==0.f&&!dry.bypass);
        CHECK(bypass.peak==0.f&&bypass.total==0.f&&bypass.wet==0.f&&bypass.bypass);
        CHECK(!(dry==bypass));
    }
    CHECK(controller.terminate()==kResultOk);
    CHECK(component.terminate()==kResultOk);
    std::puts("P006 VST3 state PASS: versions 1,2,3,4,5; controller/processor sync; truncation, NaN and future versions rejected");
    return 0;
}
