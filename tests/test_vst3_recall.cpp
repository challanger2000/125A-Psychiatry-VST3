// P006: exercise the *actual* VST3 processor/controller state interfaces.
// Runs on the Windows Steinberg SDK build, not a duplicated serialization
// reimplementation. The final Studio Pro project save/reopen test remains due.
#include "vst3/processor.h"
#include "vst3/controller.h"
#include "vst3/ids.h"
#include "vst3/monitor_model.h"

#include "public.sdk/source/vst/utility/memoryibstream.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "base/source/fstreamer.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <set>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include "pluginterfaces/vst/ivsteditcontroller.h"

#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"P006 state FAIL %s:%d %s\n",__FILE__,__LINE__,#x);std::abort();}}while(0)
using namespace psychiatry::vst3;
using namespace Steinberg;

// The SDK's standalone test executable is not a VST3 DLL; it has no DllMain.
// Provide only the module-handle symbol expected by VSTGUI support.
// The real plugin bundle receives its module handle from Steinberg's DLL entry.
void* moduleHandle = nullptr;

// Minimal host spy: verifies that UI randomization is reported to the real
// Steinberg IComponentHandler interfaces, with one grouped edit and twelve
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
// Bridge the actual controller edit events through the SDK host queue
// into the actual processor. Then restore a fresh processor/controller
// from saved state. This is a host harness, not a DAW or DLL-host test.
static void testRandomizerRecall(){
    constexpr std::array<Vst::ParamID,12> ids{
        kDepression,kParanoia,kSchizophrenia,kMania,kPanic,kDissociation,
        kObsession,kHallucination,kPsychosis,kDelirium,kCatatonia,kAmnesia};
    constexpr int block=256,frames=block*192;
    Processor live,restored,unchanged;
    Controller control,reopened;
    CHECK(control.initialize(nullptr)==kResultOk);
    CHECK(reopened.initialize(nullptr)==kResultOk);
    for(auto* p:{&live,&restored,&unchanged}){
        CHECK(p->initialize(nullptr)==kResultOk);
        CHECK(p->activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk);
        CHECK(p->activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk);
        Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,block,48000.};
        CHECK(p->setupProcessing(setup)==kResultOk);
    }
    ResizableMemoryIBStream initial;
    {
        IBStreamer w(&initial,kLittleEndian);
        CHECK(w.writeInt32(kStateMagic)&&w.writeInt32(kStateVersion));
        for(int i=0;i<9;++i)CHECK(w.writeFloat(.85f));
        CHECK(w.writeFloat(.25f)&&w.writeInt32(0));
        for(int i=0;i<3;++i)CHECK(w.writeFloat(.85f));
    }
    initial.rewind();CHECK(live.setState(&initial)==kResultOk);
    initial.rewind();CHECK(unchanged.setState(&initial)==kResultOk);
    initial.rewind();CHECK(control.setComponentState(&initial)==kResultOk);
    CHECK(live.setActive(true)==kResultOk);
    CHECK(live.setProcessing(true)==kResultOk&&live.setProcessing(false)==kResultOk&&live.setProcessing(true)==kResultOk);
    CHECK(unchanged.setActive(true)==kResultOk);
    control.setGermanUi(true);control.setMotionEnabled(true);
    auto* spy=new HandlerSpy;
    CHECK(control.setComponentHandler(spy)==kResultOk);
    control.triggerLossOfControl();
    CHECK(spy->events.size()==36&&spy->groupStarts==1&&spy->groupFinishes==1);
    Vst::ParameterChanges changes(12);
    unsigned active=0,zero=0;
    for(std::size_t i=0;i<ids.size();++i){
        const auto& e=spy->events[3*i+1];
        CHECK(e.type=='P'&&e.id==ids[i]);
        CHECK(e.value==control.getParamNormalized(ids[i]));
        int32 index=0,point=0;
        auto* q=changes.addParameterData(e.id,index);
        CHECK(q&&q->addPoint(0,e.value,point)==kResultTrue);
        if(e.value>0.)++active;else ++zero;
    }
    CHECK(active>=3&&active<=6&&zero==12-active);
    CHECK(control.getParamNormalized(kTherapy)==.25&&control.getParamNormalized(kBypass)==0.);
    CHECK(control.isGermanUi()&&control.isMotionEnabled());
    CHECK(control.setComponentHandler(nullptr)==kResultOk);spy->release();

    using Samples=std::array<float,block>;
    const auto audioBlock=[&](Processor& p,int offset,Samples& l,Samples& r,
                              Vst::ParameterChanges* queues=nullptr){
        Samples inL{},inR{};
        for(int n=0;n<block;++n){
            const double t=double(offset+n)/48000.;
            const double envelope=.6+.4*std::sin(6.283185307179586*3.*t);
            inL[n]=float(envelope*(.23*std::sin(6.283185307179586*117.*t)+
                                      .11*std::sin(6.283185307179586*781.*t)));
            inR[n]=float(envelope*(.19*std::sin(6.283185307179586*143.*t)+
                                      .09*std::sin(6.283185307179586*531.*t)));
        }
        float* src[]{inL.data(),inR.data()},*dst[]{l.data(),r.data()};
        Vst::AudioBusBuffers bi{},bo{};
        bi.numChannels=bo.numChannels=2;bi.channelBuffers32=src;bo.channelBuffers32=dst;
        Vst::ProcessData d{};
        d.processMode=Vst::kRealtime;d.symbolicSampleSize=Vst::kSample32;
        d.numSamples=block;d.numInputs=d.numOutputs=1;d.inputs=&bi;d.outputs=&bo;
        d.inputParameterChanges=queues;
        CHECK(p.process(d)==kResultOk);
        for(int i=0;i<block;++i)CHECK(std::isfinite(l[i])&&std::isfinite(r[i])&&
                                     std::abs(l[i])<4.f&&std::abs(r[i])<4.f);
    };
    double difference=0.;
    for(int n=0;n<frames;n+=block){
        Samples a{},b{},c{},d{};
        audioBlock(live,n,a,b,n==0?&changes:nullptr);
        audioBlock(unchanged,n,c,d);
        for(int i=0;i<block;++i)difference+=std::abs(double(a[i])-c[i])+std::abs(double(b[i])-d[i]);
    }
    CHECK(difference>1.); // actual random edits cannot be an inert UI change

    ResizableMemoryIBStream saved,ui;
    CHECK(live.getState(&saved)==kResultOk);
    CHECK(control.getState(&ui)==kResultOk);
    saved.rewind();CHECK(restored.setState(&saved)==kResultOk);
    saved.rewind();CHECK(reopened.setComponentState(&saved)==kResultOk);
    ui.rewind();CHECK(reopened.setState(&ui)==kResultOk);
    for(auto id:ids)equals(float(reopened.getParamNormalized(id)),float(control.getParamNormalized(id)));
    equals(float(reopened.getParamNormalized(kTherapy)),.25f);
    CHECK(reopened.getParamNormalized(kBypass)==0.&&reopened.isGermanUi()&&reopened.isMotionEnabled());

    // Recreate audio history from a clean transport lifecycle on both
    // instances, rather than requiring a saved project to preserve delay tails.
    CHECK(live.setActive(false)==kResultOk&&live.setActive(true)==kResultOk);
    CHECK(restored.setActive(true)==kResultOk);
    double energy=0.;
    for(int n=0;n<frames;n+=block){
        Samples a{},b{},c{},d{};
        audioBlock(live,n,a,b);audioBlock(restored,n,c,d);
        CHECK(a==c&&b==d);
        for(int i=0;i<block;++i)energy+=double(a[i])*a[i]+double(b[i])*b[i];
    }
    CHECK(energy>1.e-5);
    for(auto* p:{&live,&restored,&unchanged}){
        CHECK(p->setActive(false)==kResultOk&&p->terminate()==kResultOk);
    }
    CHECK(control.terminate()==kResultOk&&reopened.terminate()==kResultOk);
    std::printf("P027g UI-host-audio-recall PASS: %u active, %u zeroed, live delta %.3f, bit-exact restored stereo\n",active,zero,difference);
}

// Deterministic host-stream reentrancy and full-count error injection.
// The old code writes the first two A fields, then reads the rest from B.
class AuditStream : public ResizableMemoryIBStream {
public:
    int readFailAt=-1,writeFailAt=-1,callbackAt=-1;
    std::function<void()> callback;
    tresult PLUGIN_API read(void* bytes,int32 n,int32* count) override {
        const auto at=getCursor();
        const auto result=ResizableMemoryIBStream::read(bytes,n,count);
        return at==static_cast<std::size_t>(readFailAt)?kResultFalse:result;
    }
    tresult PLUGIN_API write(void* bytes,int32 n,int32* count) override {
        const auto at=getCursor();
        const auto result=ResizableMemoryIBStream::write(bytes,n,count);
        if(callback&&at==static_cast<std::size_t>(callbackAt)){
            auto action=std::move(callback);action();
        }
        return at==static_cast<std::size_t>(writeFailAt)?kResultFalse:result;
    }
};
static void uniformState(ResizableMemoryIBStream& memory,float value){
    IBStreamer w(&memory,kLittleEndian);
    CHECK(w.writeInt32(kStateMagic)&&w.writeInt32(5));
    for(int i=0;i<10;++i)CHECK(w.writeFloat(value));
    CHECK(w.writeInt32(0));
    for(int i=0;i<3;++i)CHECK(w.writeFloat(value));
    memory.rewind();
}
static void verifyUniform(ResizableMemoryIBStream& memory,float expected){
    memory.rewind();IBStreamer r(&memory,kLittleEndian);
    int32 magic=0,version=0,bypass=0;
    CHECK(r.readInt32(magic)&&magic==kStateMagic&&r.readInt32(version)&&version==5);
    float v=0.f;
    for(int i=0;i<10;++i){CHECK(r.readFloat(v));equals(v,expected);}
    CHECK(r.readInt32(bypass)&&bypass==0);
    for(int i=0;i<3;++i){CHECK(r.readFloat(v));equals(v,expected);}
}
static void testAuditState(){
    Processor p;Controller c;
    CHECK(p.initialize(nullptr)==kResultOk&&c.initialize(nullptr)==kResultOk);
    Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,256,48000.};
    CHECK(p.setupProcessing(setup)==kResultOk);
    CHECK(p.setActive(true)==kResultOk&&p.setProcessing(true)==kResultOk);
    constexpr std::array<Vst::ParamID,13> ids{
        kDepression,kParanoia,kSchizophrenia,kMania,kPanic,kDissociation,kObsession,
        kHallucination,kPsychosis,kTherapy,kDelirium,kCatatonia,kAmnesia};
    auto automate=[&](double v){
        Vst::ParameterChanges changes(13);
        for(auto id:ids){int32 index=0,point=0;
            auto* q=changes.addParameterData(id,index);
            CHECK(q&&q->addPoint(0,v,point)==kResultOk);
        }
        Vst::ProcessData data{};data.symbolicSampleSize=Vst::kSample32;
        data.inputParameterChanges=&changes;
        CHECK(p.process(data)==kResultOk);
    };
    automate(.2);
    AuditStream interleaved;
    interleaved.callbackAt=16;interleaved.callback=[&]{automate(.8);};
    CHECK(p.getState(&interleaved)==kResultOk);verifyUniform(interleaved,.2f);
    ResizableMemoryIBStream after;
    CHECK(p.getState(&after)==kResultOk);verifyUniform(after,.8f);
    // Actual concurrent audio publications; a complete saved tuple is A or B.
    std::atomic<bool> done{false};
    std::thread audio([&]{for(int i=0;i<4000;++i)automate(i%2?.8:.2);done.store(true);});
    int saves=0;
    do {
        ResizableMemoryIBStream save;
        CHECK(p.getState(&save)==kResultOk);save.rewind();
        IBStreamer r(&save,kLittleEndian);int32 h=0;float value=0.f;
        CHECK(r.readInt32(h)&&r.readInt32(h)&&r.readFloat(value));
        CHECK(value==.2f||value==.8f);verifyUniform(save,value);++saves;
    }while(!done.load());
    audio.join();CHECK(saves>0);
    // A valid newest state is visible immediately, even before audio adoption.
    ResizableMemoryIBStream recall;uniformState(recall,.6f);
    CHECK(p.setState(&recall)==kResultOk);
    ResizableMemoryIBStream immediate;CHECK(p.getState(&immediate)==kResultOk);verifyUniform(immediate,.6f);
    automate(.4);
    ResizableMemoryIBStream adopted;CHECK(p.getState(&adopted)==kResultOk);verifyUniform(adopted,.4f);
    // Failed host reads with full reported byte counts must fail closed.
    for(int offset=0;offset<64;offset+=4){
        AuditStream bad;uniformState(bad,.9f);bad.readFailAt=offset;
        CHECK(p.setState(&bad)==kResultFalse);
        bad.rewind();CHECK(c.setComponentState(&bad)==kResultFalse);
        ResizableMemoryIBStream preserved;CHECK(p.getState(&preserved)==kResultOk);verifyUniform(preserved,.4f);
        AuditStream failedWrite;failedWrite.writeFailAt=offset;
        CHECK(p.getState(&failedWrite)==kResultFalse);
    }
    for(int length=1;length<4;++length){
        AuditStream truncated;unsigned char bytes[3]{};int32 n=0;
        CHECK(truncated.write(bytes,length,&n)==kResultOk);truncated.rewind();
        c.setMotionEnabled(true);c.setZoomIndex(2);
        CHECK(c.setState(&truncated)==kResultFalse&&c.isMotionEnabled()&&c.zoomIndex()==2);
    }
    for(int offset=0;offset<20;offset+=4){
        AuditStream bad;CHECK(c.getState(&bad)==kResultOk);bad.rewind();bad.readFailAt=offset;
        CHECK(c.setState(&bad)==kResultFalse);
        AuditStream write;write.writeFailAt=offset;CHECK(c.getState(&write)==kResultFalse);
    }
    ResizableMemoryIBStream empty;CHECK(c.setState(&empty)==kResultOk&&!c.isMotionEnabled()&&c.zoomIndex()==0);
    CHECK(p.setProcessing(false)==kResultOk&&p.setActive(false)==kResultOk);
    CHECK(c.terminate()==kResultOk&&p.terminate()==kResultOk);
    std::printf("P028k A01/A03/A04 state PASS: %d concurrent saves, reentrant stream, recall, full-count errors and truncated UI\n",saves);
}
int main(){
    testAuditState();
    testRandomizerRecall();
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
    // Exercise the actual controller action, independently checking the
    // explicit set of sound IDs (not a production-derived count or list).
    {
        constexpr std::array<Vst::ParamID,12> soundIds{
            kDepression,kParanoia,kSchizophrenia,kMania,kPanic,kDissociation,
            kObsession,kHallucination,kPsychosis,kDelirium,kCatatonia,kAmnesia};
        ResizableMemoryIBStream state;
        writeLegacy(state,5);
        CHECK(controller.setComponentState(&state)==kResultOk);
        const auto therapyBefore=controller.getParamNormalized(kTherapy);
        const auto bypassBefore=controller.getParamNormalized(kBypass);
        auto* spy=new HandlerSpy;
        CHECK(controller.setComponentHandler(spy)==kResultOk);
        std::array<double,12> previous{};
        std::array<bool,12> observedNonzero{};
        for(int pass=0;pass<64;++pass) {
            const bool german=(pass%2)==0,motion=(pass%3)==0;
            controller.setGermanUi(german);
            controller.setMotionEnabled(motion);
            spy->events.clear();
            // Simulate a previously automated out-of-range-for-randomization
            // macro value. Each ID MUST be overwritten by this action.
            for(auto id:soundIds)CHECK(controller.setParamNormalized(id,1.)==kResultOk);
            controller.triggerLossOfControl();
            CHECK(spy->groupStarts==pass+1&&spy->groupFinishes==pass+1);
            CHECK(spy->events.size()==36);
            bool different=false;
            unsigned activeCount=0;
            for(std::size_t i=0;i<soundIds.size();++i) {
                const auto id=soundIds[i];
                const auto& begin=spy->events[i*3];
                const auto& perform=spy->events[i*3+1];
                const auto& end=spy->events[i*3+2];
                CHECK(begin.type=='B'&&perform.type=='P'&&end.type=='E');
                CHECK(begin.id==id&&perform.id==id&&end.id==id);
                const double value=controller.getParamNormalized(id);
                CHECK(perform.value==value);
                CHECK(std::isfinite(value)&&(value==0.||(value>=.12&&value<=.78)));
                if(value>0.){observedNonzero[i]=true;++activeCount;}
                if(value!=previous[i])different=true;
                previous[i]=value;
            }
            CHECK(activeCount>=3&&activeCount<=6&&different);
            CHECK(controller.getParamNormalized(kTherapy)==therapyBefore);
            CHECK(controller.getParamNormalized(kBypass)==bypassBefore);
            CHECK(controller.isGermanUi()==german&&controller.isMotionEnabled()==motion);
        }
        for(bool active:observedNonzero)CHECK(active);
        CHECK(controller.setComponentHandler(nullptr)==kResultOk);
        spy->release();
        controller.setGermanUi(false);
        controller.setMotionEnabled(false);
        std::puts("P027a LOSS OF CONTROL PASS: 64 presses, 12 IDs, 3-6 active, 36 host edits per press, globals preserved");
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
        controller.setZoomIndex(3);
        CHECK(controller.getState(&saved)==kResultOk);
        controller.setGermanUi(false);
        controller.setMotionEnabled(false);
        controller.setZoomIndex(0);
        saved.rewind();
        CHECK(controller.setState(&saved)==kResultOk);
        CHECK(controller.isGermanUi());
        CHECK(controller.isMotionEnabled());
        CHECK(controller.zoomIndex()==3);
        controller.setZoomIndex(0);
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
        std::array<double,12> macros{};macros[3]=.8;macros[6]=.4;
        const auto full=MonitorModel::make(macros,0.,0.);
        CHECK(full.dominant==3&&full.peak==.8f&&full.levels[6]==.4f);
        const auto half=MonitorModel::make(macros,.5,0.);
        CHECK(half.dominant==3&&half.peak==.4f&&half.levels[6]==.2f);
        const auto dry=MonitorModel::make(macros,1.,0.);
        const auto bypass=MonitorModel::make(macros,0.,1.);
        CHECK(dry.peak==0.f&&dry.total==0.f&&dry.wet==0.f&&!dry.bypass);
        CHECK(bypass.peak==0.f&&bypass.total==0.f&&bypass.wet==0.f&&bypass.bypass);
        CHECK(!(dry==bypass));
        CHECK(full.secondary()==6&&dry.secondary()==-1&&bypass.secondary()==-1);
        constexpr std::array<Vst::ParamID,12> ids{
            kDepression,kParanoia,kSchizophrenia,kMania,kPanic,kDissociation,
            kObsession,kHallucination,kPsychosis,kDelirium,kCatatonia,kAmnesia};
        // Exercise all twelve actual controller values, including the
        // newly implemented worlds. Single-world dominance and per-world
        // change detection must not alias any of the previous nine IDs.
        for(std::size_t world=0;world<ids.size();++world){
            for(auto id:ids)CHECK(controller.setParamNormalized(id,0.)==kResultOk);
            CHECK(controller.setParamNormalized(ids[world],.75)==kResultOk);
            std::array<double,12> actual{};
            for(std::size_t i=0;i<ids.size();++i)actual[i]=controller.getParamNormalized(ids[i]);
            const auto isolated=MonitorModel::make(actual,0.,0.);
            CHECK(isolated.dominant==static_cast<int>(world)&&isolated.peak==.75f);
            CHECK(isolated.total==.75f&&isolated.secondary()==-1);
            for(std::size_t i=0;i<actual.size();++i)
                CHECK(isolated.levels[i]==(i==world?.75f:0.f));
            CHECK(!(isolated==MonitorModel::make({},0.,0.)));
            const auto reduced=MonitorModel::make(actual,.5,0.);
            CHECK(reduced.peak==.375f&&reduced.dominant==static_cast<int>(world));
            CHECK(MonitorModel::make(actual,1.,0.).total==0.f);
            CHECK(MonitorModel::make(actual,0.,1.).total==0.f);
        }
        macros.fill(0.);
        macros[9]=.8;macros[10]=.8;macros[11]=.6;
        const auto tie=MonitorModel::make(macros,0.,0.);
        CHECK(tie.dominant==9&&tie.secondary()==10);
        macros[11]=1.;
        const auto newer=MonitorModel::make(macros,0.,0.);
        CHECK(newer.dominant==11&&newer.secondary()==9&&!(tie==newer));
        macros[9]=std::numeric_limits<double>::quiet_NaN();
        macros[10]=-1.;macros[11]=2.;
        const auto clamped=MonitorModel::make(macros,0.,0.);
        CHECK(clamped.dominant==11&&clamped.peak==1.f&&clamped.total==1.f&&clamped.secondary()==-1);
        std::puts("P027f monitor logic PASS: all twelve worlds, secondary, ties, THERAPY, bypass, bounds");
    }
    CHECK(controller.terminate()==kResultOk);
    CHECK(component.terminate()==kResultOk);
    std::puts("P006 VST3 state PASS: versions 1,2,3,4,5; controller/processor sync; truncation, NaN and future versions rejected");
    return 0;
}
