// P006: exercise the *actual* VST3 processor/controller state interfaces.
// Runs on the Windows Steinberg SDK build, not a duplicated serialization
// reimplementation. The final Studio Pro project save/reopen test remains due.
#include "vst3/processor.h"
#include "vst3/controller.h"
#include "vst3/ids.h"

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
    } else if(version==4) {
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
    for(int version=1;version<=4;++version) {
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
        CHECK(roundtrip.getCursor()==52); // 8 header + 10 float32 + 4 bypass
    }
    {
        ResizableMemoryIBStream corrupt;
        writeLegacy(corrupt,4,true);
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
    // Controller-only UI state roundtrip: language preference is persisted
    // independently of the processor's stable 52-byte v4 audio state.
    {
        ResizableMemoryIBStream saved;
        CHECK(!controller.isGermanUi());
        controller.setGermanUi(true);
        CHECK(controller.getState(&saved)==kResultOk);
        controller.setGermanUi(false);
        saved.rewind();
        CHECK(controller.setState(&saved)==kResultOk);
        CHECK(controller.isGermanUi());
        controller.setGermanUi(false);
        ResizableMemoryIBStream savedEn;
        CHECK(controller.getState(&savedEn)==kResultOk);
        controller.setGermanUi(true);
        savedEn.rewind();
        CHECK(controller.setState(&savedEn)==kResultOk);
        CHECK(!controller.isGermanUi());
    }
    CHECK(controller.terminate()==kResultOk);
    CHECK(component.terminate()==kResultOk);
    std::puts("P006 VST3 state PASS: versions 1,2,3,4; controller/processor sync; truncation, NaN and future versions rejected");
    return 0;
}
