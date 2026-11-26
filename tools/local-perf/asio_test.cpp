// SPDX-License-Identifier: GPL-3.0-only
// Private engineering host, separate executable; no product DSP is linked.
#define NOMINMAX
#include <windows.h>
#include <objbase.h>
#include "asiosys.h"
#include "iasiodrv.h"
#include "asiolist.h"
#include "vst3/ids.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "base/source/fstreamer.h"
#include "public.sdk/source/vst/utility/memoryibstream.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <thread>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
using namespace Steinberg;
using namespace psychiatry::vst3;
static void check(bool result,const char* why){
    if(!result){std::fprintf(stderr,"ERROR: %s\n",why);std::fflush(stderr);std::exit(3);}
}
static float tone(double hz,double t){return float(std::sin(6.283185307179586*hz*t));}
struct Sample{double wall,startLate;unsigned long long cycles;bool finishLate;};
struct LoadedProcessor{
    Vst::IComponent* component=nullptr;
    FUnknownPtr<Vst::IAudioProcessor> audio;
    explicit LoadedProcessor(IPluginFactory* factory){
        TUID cid,iid;kProcessorCID.toTUID(cid);Vst::IComponent::iid.toTUID(iid);
        check(factory->createInstance(cid,iid,reinterpret_cast<void**>(&component))==kResultOk&&component,"DLL component creation");
        audio=FUnknownPtr<Vst::IAudioProcessor>(component);check(audio.get()!=nullptr,"DLL audio interface");
    }
    ~LoadedProcessor(){audio=nullptr;component->release();}
    tresult initialize(FUnknown* context){return component->initialize(context);}
    tresult activateBus(Vst::MediaType type,Vst::BusDirection direction,int32 index,TBool active){return component->activateBus(type,direction,index,active);}
    tresult setupProcessing(Vst::ProcessSetup& setup){return audio->setupProcessing(setup);}
    tresult setState(IBStream* stream){return component->setState(stream);}
    tresult setActive(TBool value){return component->setActive(value);}
    tresult setProcessing(TBool value){return audio->setProcessing(value);}
    tresult process(Vst::ProcessData& data){return audio->process(data);}
    tresult terminate(){return component->terminate();}
};

using Clock=std::chrono::steady_clock;
static constexpr int warmup=128;
struct Record {double wall=0,interval=0,peak=0;uint64_t position=0,cycles=0;bool validPosition=false,processOk=false;};
struct Case {
    int rate,block,count,mode,groups;
    IASIO* driver;
    bool outputReady=false;
    std::array<ASIOBufferInfo,2> buffers{};
    std::array<int,2> bytes{};
    std::array<std::unique_ptr<LoadedProcessor>,4> processors;
    std::array<float,256> left{},right{};
    std::array<std::array<float,256>,4> ol{},orr{};
    Vst::AudioBusBuffers bi{};
    std::array<Vst::AudioBusBuffers,4> bo{};
    std::array<std::array<float*,2>,4> ptrs{};
    std::array<float*,2> ins{};
    Vst::ParameterChanges changes{13};
    std::vector<Record> records;
    std::atomic<int> completed{0},overloads{0},resync{0},reset{0},rateChange{0},reentrant{0};
    std::atomic_flag busy=ATOMIC_FLAG_INIT;
    Clock::time_point last{};
    Case(IPluginFactory* factory,IASIO* d,int r,int n,int instances,int stress,int seconds)
      :rate(r),block(n),count(instances),mode(stress),groups((seconds*r+n-1)/n),driver(d),records(groups+warmup){
        ins={left.data(),right.data()};bi.numChannels=2;bi.channelBuffers32=ins.data();
        for(int i=0;i<n;++i){left[i]=.23f*tone(173.,double(i)/r)+.13f*tone(711.,double(i)/r);
            right[i]=.21f*tone(257.,double(i)/r)+.12f*tone(997.,double(i)/r);}
        for(int i=0;i<count;++i){
            auto& p=processors[i];p=std::make_unique<LoadedProcessor>(factory);
            check(p->initialize(nullptr)==kResultOk,"component init");
            check(p->activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p->activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"stereo buses");
            Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,n,double(r)};
            check(p->setupProcessing(setup)==kResultOk,"processor setup");
            ResizableMemoryIBStream state;IBStreamer w(&state,kLittleEndian);
            check(w.writeInt32(kStateMagic)&&w.writeInt32(kStateVersion),"state header");
            for(int j=0;j<9;++j)check(w.writeFloat(mode?.85f:(j==0?.45f:j==1?.35f:j==4?.50f:0.f)),"macro state");
            check(w.writeFloat(mode?.15f:.25f)&&w.writeInt32(0),"globals");
            for(int j=0;j<3;++j)check(w.writeFloat(mode?.85f:(j==1?.55f:0.f)),"extra macros");
            state.rewind();
            check(p->setState(&state)==kResultOk&&p->setActive(true)==kResultOk&&p->setProcessing(true)==kResultOk,"activate");
            ptrs[i]={ol[i].data(),orr[i].data()};bo[i].numChannels=2;bo[i].channelBuffers32=ptrs[i].data();
        }
        // Prebuild queues off the callback thread. Stress all macros at high
        // values, with one point per sample; no bypass toggling or allocations.
        if(mode)for(int lane=0;lane<13;++lane){
            int32 qi=0,pi=0;auto* q=changes.addParameterData(lane<12?100+lane:kTherapy,qi);
            check(q!=nullptr,"parameter queue");
            for(int i=0;i<n;++i)check(q->addPoint(i,lane<12?.75+.2*((i+lane)%17)/16.:.05+.2*(i%19)/18.,pi)==kResultOk,"parameter point");
        }
    }
    void close(){
        for(int i=0;i<count;++i){auto& p=processors[i];check(p->setProcessing(false)==kResultOk&&p->setActive(false)==kResultOk&&p->terminate()==kResultOk,"terminate");p.reset();}
    }
    void clear(long index){
        if(index<0||index>1)return;
        for(int i=0;i<2;++i)if(buffers[i].buffers[index])std::memset(buffers[i].buffers[index],0,size_t(block)*bytes[i]);
    }
    void callback(long index,const ASIOTime* timing){
        if(busy.test_and_set(std::memory_order_acquire)){++reentrant;return;}
        const auto start=Clock::now();
        const int g=completed.load(std::memory_order_relaxed);
        if(g>=int(records.size())){clear(index);if(outputReady)driver->outputReady();busy.clear(std::memory_order_release);return;}
        Record rec{};
        rec.interval=g?std::chrono::duration<double,std::micro>(start-last).count():0.;last=start;
        if(timing&&(timing->timeInfo.flags&kSamplePositionValid)){
            rec.validPosition=true;
            rec.position=(uint64_t(timing->timeInfo.samplePosition.hi)<<32)|uint64_t(timing->timeInfo.samplePosition.lo);
        }
        unsigned long long cb=0,ce=0;
        const bool gotCycles=QueryThreadCycleTime(GetCurrentThread(),&cb)!=0;
        rec.processOk=index==0||index==1;
        for(int i=0;i<count;++i){
            Vst::ProcessData data{};data.processMode=Vst::kRealtime;data.symbolicSampleSize=Vst::kSample32;
            data.numSamples=block;data.numInputs=data.numOutputs=1;data.inputs=&bi;data.outputs=&bo[i];
            data.inputParameterChanges=mode?&changes:nullptr;
            rec.processOk=(processors[i]->process(data)==kResultOk)&&rec.processOk;
            for(int j=0;j<block;++j){
                const double a=ol[i][j],b=orr[i][j];
                rec.processOk=std::isfinite(a)&&std::isfinite(b)&&rec.processOk;
                rec.peak=std::max(rec.peak,std::max(std::abs(a),std::abs(b)));
            }
        }
        // Real hardware clock/callback; deliberately silent physical output.
        clear(index);if(outputReady)driver->outputReady();
        const auto end=Clock::now();
        if(gotCycles&&QueryThreadCycleTime(GetCurrentThread(),&ce))rec.cycles=ce-cb;
        rec.wall=std::chrono::duration<double,std::micro>(end-start).count();
        records[g]=rec;completed.store(g+1,std::memory_order_release);
        busy.clear(std::memory_order_release);
    }
};
static std::atomic<Case*> active{nullptr};
static void bufferSwitch(long i,ASIOBool){if(auto* p=active.load(std::memory_order_acquire))p->callback(i,nullptr);}
static ASIOTime* bufferTime(ASIOTime* t,long i,ASIOBool){if(auto* p=active.load(std::memory_order_acquire))p->callback(i,t);return nullptr;}
static void rateChanged(ASIOSampleRate){if(auto* p=active.load(std::memory_order_acquire))++p->rateChange;}
static long message(long selector,long value,void*,double*){
    if(selector==kAsioSelectorSupported)return value==kAsioEngineVersion||value==kAsioSupportsTimeInfo||value==kAsioOverload||value==kAsioResetRequest||value==kAsioResyncRequest||value==kAsioLatenciesChanged;
    if(selector==kAsioEngineVersion)return 2;
    if(selector==kAsioSupportsTimeInfo)return 1;
    auto* p=active.load(std::memory_order_acquire);
    if(selector==kAsioOverload){if(p)++p->overloads;return 1;}
    if(selector==kAsioResetRequest){if(p)++p->reset;return 1;}
    if(selector==kAsioResyncRequest){if(p)++p->resync;return 1;}
    if(selector==kAsioLatenciesChanged)return 1;
    return 0;
}
static int sampleBytes(ASIOSampleType t){
    switch(t){
        case ASIOSTInt16LSB:case ASIOSTInt16MSB:return 2;
        case ASIOSTInt24LSB:case ASIOSTInt24MSB:return 3;
        case ASIOSTInt32LSB:case ASIOSTInt32MSB:case ASIOSTFloat32LSB:case ASIOSTFloat32MSB:
        case ASIOSTInt32LSB16:case ASIOSTInt32LSB18:case ASIOSTInt32LSB20:case ASIOSTInt32LSB24:
        case ASIOSTInt32MSB16:case ASIOSTInt32MSB18:case ASIOSTInt32MSB20:case ASIOSTInt32MSB24:return 4;
        case ASIOSTFloat64LSB:case ASIOSTFloat64MSB:return 8;
        default:return 0;
    }
}
static int analyze(Case& c,std::ostream& out){
    const int done=c.completed.load(std::memory_order_acquire);
    const double budget=1.e6*c.block/c.rate;
    int miss=0,gaps=0,invalid=0,positionCount=0;double peak=0.;
    std::vector<double> times;
    for(int g=warmup;g<done;++g){
        const auto& r=c.records[g];const auto& prev=c.records[g-1];
        const bool gap=r.validPosition&&prev.validPosition&&r.position!=prev.position+uint64_t(c.block);
        positionCount+=r.validPosition;gaps+=gap;miss+=r.wall>budget;invalid+=!r.processOk;peak=std::max(peak,r.peak);times.push_back(r.wall);
        out<<c.rate<<','<<c.block<<','<<c.count<<','<<c.mode<<','<<g-warmup<<','<<r.wall<<','<<budget<<','<<(r.wall>budget)<<','<<r.interval<<','<<r.cycles<<','<<r.validPosition<<','<<r.position<<','<<gap<<','<<r.processOk<<','<<r.peak<<'\n';
    }
    out.flush();
    const bool incomplete=done!=int(c.records.size())||invalid||peak<=1.e-8||c.reset||c.rateChange||c.reentrant||!out.good();
    const bool failed=miss||gaps||c.overloads||c.resync;
    std::sort(times.begin(),times.end());
    const double p99=times.empty()?0.:times[times.size()*99/100],max=times.empty()?0.:times.back();
    std::printf("%d Hz / %d samples / %d instances / %s: %s measured=%zu/%d p99=%.2f us max=%.2f us budget=%.2f us misses=%d position_gaps=%d position_valid=%d overloads=%d resync=%d resets=%d rate_changes=%d reentrant=%d invalid=%d peak=%.6f\n",
       c.rate,c.block,c.count,c.mode?"dense":"steady",incomplete?"INCOMPLETE":failed?"FAIL":"PASS",
       times.size(),c.groups,p99,max,budget,miss,gaps,positionCount,c.overloads.load(),c.resync.load(),c.reset.load(),c.rateChange.load(),c.reentrant.load(),invalid,peak);
    return incomplete?3:failed?4:0;
}
static int runCase(IPluginFactory* factory,IASIO* driver,int rate,int block,int count,int mode,std::ostream& out){
    ASIOSampleRate actual=0;
    if(driver->getSampleRate(&actual)!=ASE_OK||std::abs(actual-rate)>.01){
        std::printf("%d/%d/%d/%d NOT RUN: driver sample rate mismatch\n",rate,block,count,mode);return 3;
    }
    long min=0,max=0,pref=0,gran=0;
    if(driver->getBufferSize(&min,&max,&pref,&gran)!=ASE_OK||block<min||block>max||
        (gran==0&&block!=pref)||(gran>0&&(block-min)%gran!=0)){
        std::printf("%d/%d/%d/%d NOT RUN: driver does not accept requested buffer; min=%ld max=%ld preferred=%ld gran=%ld\n",rate,block,count,mode,min,max,pref,gran);return 3;
    }
    Case c(factory,driver,rate,block,count,mode,10);
    for(int i=0;i<2;++i){
        c.buffers[i].isInput=ASIOFalse;c.buffers[i].channelNum=i;
        ASIOChannelInfo info{};info.channel=i;info.isInput=ASIOFalse;
        if(driver->getChannelInfo(&info)!=ASE_OK||!(c.bytes[i]=sampleBytes(info.type))){
            std::puts("NOT RUN: unsupported driver output type");c.close();return 3;}
    }
    ASIOCallbacks callbacks{bufferSwitch,rateChanged,message,bufferTime};
    const auto created=driver->createBuffers(c.buffers.data(),2,block,&callbacks);
    if(created!=ASE_OK){std::printf("NOT RUN: createBuffers failed %ld\n",long(created));c.close();return 3;}
    c.clear(0);c.clear(1);
    long inLatency=0,outLatency=0;
    const auto lat=driver->getLatencies(&inLatency,&outLatency);
    c.outputReady=driver->outputReady()==ASE_OK;
    std::printf("Driver buffer=%d latency_samples input=%ld output=%ld latency_status=%ld overload_reporting=%s\n",block,inLatency,outLatency,long(lat),driver->future(kAsioCanReportOverload,nullptr)==ASE_SUCCESS?"supported":"unknown/not supported");
    active.store(&c,std::memory_order_release);
    const auto started=driver->start();
    if(started!=ASE_OK){active.store(nullptr,std::memory_order_release);driver->disposeBuffers();c.close();std::printf("NOT RUN: driver start failed %ld\n",long(started));return 3;}
    const auto until=Clock::now()+std::chrono::seconds(25);
    while(c.completed.load(std::memory_order_acquire)<int(c.records.size())&&!c.reset&&!c.rateChange&&Clock::now()<until){
        MSG msg{};while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        Sleep(10);
    }
    // ASIO contract: after successful stop there are no further callbacks.
    check(driver->stop()==ASE_OK,"driver stop failed");
    active.store(nullptr,std::memory_order_release);
    check(driver->disposeBuffers()==ASE_OK,"dispose buffers");
    const int result=analyze(c,out);c.close();return result;
}
int wmain(int argc,wchar_t** argv){
    std::setvbuf(stdout,nullptr,_IONBF,0);std::setvbuf(stderr,nullptr,_IONBF,0);
    if(argc!=3&&argc!=4)return 3;
    const bool smoke=argc==4&&std::wstring(argv[3])==L"--selftest";if(argc==4&&!smoke)return 3;
    check(SUCCEEDED(CoInitialize(nullptr)),"COM");
    const auto path=std::filesystem::absolute(argv[1]);
    HMODULE dll=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    check(dll!=nullptr,"load exact plugin");
    auto init=reinterpret_cast<bool(*)()>(GetProcAddress(dll,"InitDll"));
    auto exitModule=reinterpret_cast<bool(*)()>(GetProcAddress(dll,"ExitDll"));
    auto factoryFn=reinterpret_cast<IPluginFactory*(PLUGIN_API*)()>(GetProcAddress(dll,"GetPluginFactory"));
    check(init&&exitModule&&factoryFn&&init(),"module init");
    auto* factory=factoryFn();check(factory&&factory->countClasses()==2,"factory");
    std::ofstream out{std::filesystem::path(argv[2])};check(out.good(),"CSV open");
    out<<std::setprecision(12)<<"rate,block,instances,mode,group,wall_us,budget_us,miss,interval_us,thread_cycles,position_valid,position,position_gap,process_ok,peak\n";
    int failures=0,incomplete=0;
    if(smoke){
        // No real driver on CI. Exercise the exact DLL callback path, output
        // zeroing and metric generation; never report this as a hardware run.
        Case c(factory,nullptr,48000,64,1,1,1);
        std::array<std::array<unsigned char,256>,4> samples{};
        for(int i=0;i<2;++i){c.bytes[i]=4;c.buffers[i].buffers[0]=samples[2*i].data();c.buffers[i].buffers[1]=samples[2*i+1].data();}
        for(int g=0;g<int(c.records.size());++g){
            const int index=g%2;for(int i=0;i<2;++i)samples[2*i+index].fill(0xff);
            c.callback(index,nullptr);
            for(int i=0;i<2;++i)check(std::all_of(samples[2*i+index].begin(),samples[2*i+index].end(),[](unsigned char x){return x==0;}),"silence verification");
        }
        const int result=analyze(c,out);c.close();check(result!=3,"selftest complete");failures=result==4;
        std::puts("SELFTEST ONLY, no hardware driver used");
    }else{
        AsioDriverList drivers;
        const long num=drivers.asioGetNumDev();
        check(num>0,"no installed native drivers found");
        int selected=-1;
        std::puts("Installed native drivers (select Yamaha Steinberg USB ASIO for UR22):");
        for(int i=0;i<num;++i){char name[128]{};if(drivers.asioGetDriverName(i,name,sizeof(name))==0)std::printf("%d: %s\n",i,name);}
        std::printf("Driver number: ");std::cin>>selected;
        check(std::cin.good()&&selected>=0&&selected<num,"invalid driver choice");
        void* raw=nullptr;check(drivers.asioOpenDriver(selected,&raw)==0&&raw,"driver open");
        auto* driver=static_cast<IASIO*>(raw);
        check(driver->init(GetConsoleWindow())!=0,"driver initialization; close DAW and other audio clients");
        char name[128]{};driver->getDriverName(name);
        long in=0,outChannels=0;check(driver->getChannels(&in,&outChannels)==ASE_OK&&outChannels>=2,"two output channels required");
        ASIOSampleRate original=0;check(driver->getSampleRate(&original)==ASE_OK,"read original sample rate");
        std::printf("Native driver: %s; version=%ld; outputs=%ld; original rate=%.1f; hardware output silent\n",name,driver->getDriverVersion(),outChannels,original);
        for(int rate:{48000,44100}){
            ASIOSampleRate current=0;const bool same=driver->getSampleRate(&current)==ASE_OK&&std::abs(current-rate)<.01;
            if(!same&&(driver->canSampleRate(rate)!=ASE_OK||driver->setSampleRate(rate)!=ASE_OK)){
                std::printf("NOT RUN: requested %d Hz unavailable\n",rate);incomplete+=12;continue;}
            for(int block:{256,128,64})for(int count:{1,4})for(int mode:{0,1}){
                const int code=runCase(factory,driver,rate,block,count,mode,out);
                failures+=code==4;incomplete+=code==3;
            }
        }
        if(original>0&&driver->setSampleRate(original)!=ASE_OK){std::puts("WARNING: original driver sample rate could not be restored");++incomplete;}
        drivers.asioCloseDriver(selected);
    }
    out.close();check(!out.fail(),"CSV close");
    factory->release();check(exitModule(),"module exit");FreeLibrary(dll);CoUninitialize();
    std::printf("COMPLETE %s failed_cases=%d incomplete_cases=%d; no acoustic loopback proof\n",incomplete?"INCOMPLETE":failures?"FAIL":"PASS",failures,incomplete);
    return incomplete?3:failures?4:0;
}
