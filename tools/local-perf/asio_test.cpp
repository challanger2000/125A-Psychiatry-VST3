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
// Diagnostic modes: -2/-1 driver-only controls; 2 neutral; 3..14
// individual macros; 15 all high/static; original 0/1 unchanged.
static const char* modeName(int mode){
    static constexpr const char* names[]={"DEPRESSION","PARANOIA","SCHIZOPHRENIA","MANIA","PANIC","DISSOCIATION","OBSESSION","HALLUCINATION","PSYCHOSIS","DELIRIUM","CATATONIA","AMNESIA"};
    if(mode==-2)return "driver-only-before";
    if(mode==-1)return "driver-only-after";
    if(mode==0)return "steady";
    if(mode==1)return "dense";
    if(mode==2)return "neutral";
    if(mode>=3&&mode<=14)return names[mode-3];
    return "all-high-static";
}
static float macroValue(int mode,int j){
    if(mode==1||mode==15)return .85f;
    if(mode==0)return j==0?.45f:j==1?.35f:j==4?.50f:j==10?.55f:0.f;
    return mode==3+j?.85f:0.f;
}
struct Record {double wall=0,interval=0,peak=0,pluginUs=0,driverUs=0;uint64_t position=0,cycles=0;int overloadTotal=0;bool validPosition=false,processOk=false;};
struct Case {
    int rate,block,count,mode,groups;
    IASIO* driver;
    bool outputReady=false;
    std::array<ASIOBufferInfo,4> buffers{};
    int bufferCount=2, inputCount=0;
    std::array<int,4> bytes{};
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
            for(int j=0;j<9;++j)check(w.writeFloat(macroValue(mode,j)),"macro state");
            check(w.writeFloat(mode==0?.25f:.15f)&&w.writeInt32(0),"globals");
            for(int j=0;j<3;++j)check(w.writeFloat(macroValue(mode,9+j)),"extra macros");
            state.rewind();
            check(p->setState(&state)==kResultOk&&p->setActive(true)==kResultOk&&p->setProcessing(true)==kResultOk,"activate");
            ptrs[i]={ol[i].data(),orr[i].data()};bo[i].numChannels=2;bo[i].channelBuffers32=ptrs[i].data();
        }
        // Prebuild queues off the callback thread. Stress all macros at high
        // values, with one point per sample; no bypass toggling or allocations.
        if(mode==1)for(int lane=0;lane<13;++lane){
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
        for(int i=inputCount;i<bufferCount;++i)if(buffers[i].buffers[index])std::memset(buffers[i].buffers[index],0,size_t(block)*bytes[i]);
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
            data.inputParameterChanges=mode==1?&changes:nullptr;
            const auto ps=Clock::now();
            rec.processOk=(processors[i]->process(data)==kResultOk)&&rec.processOk;
            rec.pluginUs+=std::chrono::duration<double,std::micro>(Clock::now()-ps).count();
            for(int j=0;j<block;++j){
                const double a=ol[i][j],b=orr[i][j];
                rec.processOk=std::isfinite(a)&&std::isfinite(b)&&rec.processOk;
                rec.peak=std::max(rec.peak,std::max(std::abs(a),std::abs(b)));
            }
        }
        // Real hardware clock/callback; deliberately silent physical output.
        clear(index);const auto ds=Clock::now();if(outputReady)driver->outputReady();
        const auto end=Clock::now();
        rec.driverUs=std::chrono::duration<double,std::micro>(end-ds).count();
        rec.overloadTotal=overloads.load(std::memory_order_relaxed);
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
    std::vector<double> times,pluginTimes,driverTimes;
    for(int g=warmup;g<done;++g){
        const auto& r=c.records[g];const auto& prev=c.records[g-1];
        const bool gap=r.validPosition&&prev.validPosition&&r.position!=prev.position+uint64_t(c.block);
        positionCount+=r.validPosition;gaps+=gap;miss+=r.wall>budget;invalid+=!r.processOk;peak=std::max(peak,r.peak);times.push_back(r.wall);pluginTimes.push_back(r.pluginUs);driverTimes.push_back(r.driverUs);
        out<<c.rate<<','<<c.block<<','<<c.count<<','<<c.mode<<','<<g-warmup<<','<<r.wall<<','<<budget<<','<<(r.wall>budget)<<','<<r.interval<<','<<r.cycles<<','<<r.validPosition<<','<<r.position<<','<<gap<<','<<r.processOk<<','<<r.peak<<','<<r.pluginUs<<','<<r.driverUs<<','<<r.overloadTotal<<'\n';
    }
    out.flush();
    const bool incomplete=done!=int(c.records.size())||invalid||(c.count>0&&peak<=1.e-8)||c.reset||c.rateChange||c.reentrant||!out.good();
    const bool failed=miss||gaps||c.overloads||c.resync;
    std::sort(times.begin(),times.end());
    const double p99=times.empty()?0.:times[times.size()*99/100],max=times.empty()?0.:times.back();
    std::printf("%d Hz / %d samples / %d instances / %s: %s measured=%zu/%d p99=%.2f us max=%.2f us budget=%.2f us misses=%d position_gaps=%d position_valid=%d overloads=%d resync=%d resets=%d rate_changes=%d reentrant=%d invalid=%d peak=%.6f\n",
       c.rate,c.block,c.count,modeName(c.mode),incomplete?"INCOMPLETE":failed?"FAIL":"PASS",
       times.size(),c.groups,p99,max,budget,miss,gaps,positionCount,c.overloads.load(),c.resync.load(),c.reset.load(),c.rateChange.load(),c.reentrant.load(),invalid,peak);
    std::sort(pluginTimes.begin(),pluginTimes.end());std::sort(driverTimes.begin(),driverTimes.end());
    const auto p99of=[](const std::vector<double>& x){return x.empty()?0.:x[x.size()*99/100];};
    const int warmOver=done>=warmup?c.records[warmup-1].overloadTotal:0;
    const int measuredOver=done>warmup?c.records[done-1].overloadTotal-warmOver:0;
    std::printf("  plugin_p99_us=%.2f driver_outputReady_p99_us=%.2f overloads_before_measurement=%d overloads_during_measurement=%d overloads_after_last_record=%d\n",p99of(pluginTimes),p99of(driverTimes),warmOver,measuredOver,c.overloads.load()-warmOver-measuredOver);
    return incomplete?3:failed?4:0;
}
static int runCase(IPluginFactory* factory,IASIO* driver,int rate,int block,int count,int mode,std::ostream& out,bool& retry,int seconds){
    retry=false;
    ASIOSampleRate actual=0;
    if(driver->getSampleRate(&actual)!=ASE_OK||std::abs(actual-rate)>.01){
        std::printf("%d/%d/%d/%d NOT RUN: driver sample rate mismatch\n",rate,block,count,mode);return 3;
    }
    long min=0,max=0,pref=0,gran=0;
    if(driver->getBufferSize(&min,&max,&pref,&gran)!=ASE_OK||block<min||block>max||
        (gran==0&&block!=pref)||(gran>0&&(block-min)%gran!=0)){
        std::printf("%d/%d/%d/%d NOT RUN: driver does not accept requested buffer; min=%ld max=%ld preferred=%ld gran=%ld\n",rate,block,count,mode,min,max,pref,gran);return 3;
    }
    Case c(factory,driver,rate,block,count,mode,seconds);
    long inputs=0,outputs=0;
    if(driver->getChannels(&inputs,&outputs)!=ASE_OK||outputs<2){c.close();return 3;}
    // Open the interface in full duplex when available, as DAWs normally do.
    // Input samples are not read, recorded or exported.
    c.inputCount=int(std::min(2L,inputs));c.bufferCount=c.inputCount+2;
    for(int i=0;i<c.bufferCount;++i){
        const bool input=i<c.inputCount;
        const int channel=input?i:i-c.inputCount;
        c.buffers[i].isInput=input?ASIOTrue:ASIOFalse;c.buffers[i].channelNum=channel;
        ASIOChannelInfo info{};info.channel=channel;info.isInput=c.buffers[i].isInput;
        if(driver->getChannelInfo(&info)!=ASE_OK||!(c.bytes[i]=sampleBytes(info.type))){
            std::puts("NOT RUN: unsupported driver output type");c.close();return 3;}
    }
    ASIOCallbacks callbacks{bufferSwitch,rateChanged,message,bufferTime};
    active.store(&c,std::memory_order_release);
    const auto created=driver->createBuffers(c.buffers.data(),c.bufferCount,block,&callbacks);
    if(created!=ASE_OK){active.store(nullptr,std::memory_order_release);std::printf("NOT RUN: createBuffers failed %ld\n",long(created));retry=c.reset||c.rateChange;c.close();return 3;}
    c.clear(0);c.clear(1);
    long inLatency=0,outLatency=0;
    const auto lat=driver->getLatencies(&inLatency,&outLatency);
    c.outputReady=driver->outputReady()==ASE_OK;
    std::printf("Driver buffer=%d latency_samples input=%ld output=%ld latency_status=%ld overload_reporting=%s\n",block,inLatency,outLatency,long(lat),driver->future(kAsioCanReportOverload,nullptr)==ASE_SUCCESS?"supported":"unknown/not supported");
    active.store(&c,std::memory_order_release);
    const auto started=(c.reset||c.rateChange)?ASE_NotPresent:driver->start();
    if(started!=ASE_OK){active.store(nullptr,std::memory_order_release);check(driver->disposeBuffers()==ASE_OK,"dispose after start rejection");retry=c.reset||c.rateChange;std::printf("NOT RUN: start failed/skipped %ld resets=%d rate_changes=%d\n",long(started),c.reset.load(),c.rateChange.load());c.close();return 3;}
    const auto until=Clock::now()+std::chrono::seconds(25);
    while(c.completed.load(std::memory_order_acquire)<int(c.records.size())&&!c.reset&&!c.rateChange&&Clock::now()<until){
        MSG msg{};while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        Sleep(10);
    }
    // ASIO contract: after successful stop there are no further callbacks.
    check(driver->stop()==ASE_OK,"driver stop failed");
    active.store(nullptr,std::memory_order_release);
    check(driver->disposeBuffers()==ASE_OK,"dispose buffers");
    retry=c.reset||c.rateChange;
    if(retry){
        // Keep interrupted attempts out of the accepted measurement CSV.
        // Never mix two driver sessions or silently count a reset as PASS.
        std::printf("INTERRUPTED: callbacks=%d resets=%d rate_changes=%d overloads=%d resync=%d; reopen requested\n",c.completed.load(),c.reset.load(),c.rateChange.load(),c.overloads.load(),c.resync.load());
        c.close();return 3;
    }
    const int result=analyze(c,out);c.close();return result;
}
int wmain(int argc,wchar_t** argv){
    std::setvbuf(stdout,nullptr,_IONBF,0);std::setvbuf(stderr,nullptr,_IONBF,0);
    if(argc!=3&&argc!=4)return 3;
    const std::wstring option=argc==4?argv[3]:L"";
    const bool diagnostic=option==L"--diagnostic";
    const bool smoke=option==L"--selftest"||option==L"--diagnostic-selftest";
    const bool diagSmoke=option==L"--diagnostic-selftest";
    if(argc==4&&!smoke&&!diagnostic)return 3;
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
    out<<std::setprecision(12)<<"rate,block,instances,mode,group,wall_us,budget_us,miss,interval_us,thread_cycles,position_valid,position,position_gap,process_ok,peak,plugin_us,output_ready_us,overload_total\n";
    int failures=0,incomplete=0;
    if(smoke){
        // No real driver on CI. Exercise the exact DLL callback path, output
        // zeroing and metric generation; never report this as a hardware run.
        const std::vector<std::pair<int,int>> probes=diagSmoke?std::vector<std::pair<int,int>>{{0,-2},{1,2},{1,3},{1,14},{1,15},{0,-1}}:std::vector<std::pair<int,int>>{{1,1}};
        for(const auto& probe:probes){
        Case c(factory,nullptr,48000,64,probe.first,probe.second,1);
        std::array<std::array<unsigned char,256>,4> samples{};
        for(int i=0;i<2;++i){c.bytes[i]=4;c.buffers[i].buffers[0]=samples[2*i].data();c.buffers[i].buffers[1]=samples[2*i+1].data();}
        for(int g=0;g<int(c.records.size());++g){
            const int index=g%2;for(int i=0;i<2;++i)samples[2*i+index].fill(0xff);
            c.callback(index,nullptr);
            for(int i=0;i<2;++i)check(std::all_of(samples[2*i+index].begin(),samples[2*i+index].end(),[](unsigned char x){return x==0;}),"silence verification");
        }
        const int result=analyze(c,out);c.close();check(result!=3,"selftest complete");failures+=result==4;
        }
        std::puts("SELFTEST ONLY, no hardware driver used");
    }else{
        AsioDriverList drivers;
        const long num=drivers.asioGetNumDev();
        check(num>0,"no installed native drivers found");
        int selected=-1;
        std::puts("Installed native drivers (select Yamaha Steinberg USB ASIO for UR22):");
        for(int i=0;i<num;++i){char name[128]{};if(drivers.asioGetDriverName(i,name,sizeof(name))==0)std::printf("%d: %s\n",i,name);}
        if(num==1){selected=0;std::puts("Only one driver: automatically selected 0");}
        else {std::printf("Driver number: ");std::cin>>selected;check(std::cin.good()&&selected>=0&&selected<num,"invalid driver choice");}
        auto openDriver=[&]()->IASIO*{
            void* raw=nullptr;
            if(drivers.asioOpenDriver(selected,&raw)!=0||!raw)return nullptr;
            auto* d=static_cast<IASIO*>(raw);
            if(!d->init(GetConsoleWindow())){drivers.asioCloseDriver(selected);return nullptr;}
            return d;
        };
        auto* driver=openDriver();check(driver!=nullptr,"driver init; close other audio clients");
        char name[128]{};driver->getDriverName(name);
        ASIOSampleRate rate=0;long lo=0,hi=0,block=0,gran=0;
        check(driver->getSampleRate(&rate)==ASE_OK&&(std::abs(rate-44100.)<.01||std::abs(rate-48000.)<.01),"set 44.1 or 48 kHz in driver control panel");
        check(driver->getBufferSize(&lo,&hi,&block,&gran)==ASE_OK&&(block==64||block==128||block==256),"set preferred buffer to 64, 128 or 256 in driver panel");
        std::printf("Native driver: %s; version=%ld; fixed current rate=%.1f block=%ld; no rate or buffer changes\n",name,driver->getDriverVersion(),rate,block);
        // Release probe session. Every measurement uses a freshly initialized
        // driver, with bounded full reopen after a reset/rate-change request.
        drivers.asioCloseDriver(selected);
        struct Probe{int count,mode,seconds;};
        std::vector<Probe> probes;
        if(diagnostic){
            probes.push_back({0,-2,10});probes.push_back({1,2,5});
            for(int m=3;m<=14;++m)probes.push_back({1,m,5});
            probes.push_back({1,15,10});probes.push_back({1,1,10});probes.push_back({0,-1,10});
        }else for(int count:{1,4})for(int mode:{0,1})probes.push_back({count,mode,10});
        for(const auto& probe:probes){
            const int count=probe.count,mode=probe.mode;
            int code=3;
            for(int attempt=0;attempt<3;++attempt){
                driver=openDriver();
                if(!driver){std::puts("NOT RUN: driver reopen failed");break;}
                long l=0,h=0,p=0,g=0;
                ASIOSampleRate actual=0;
                if(driver->getSampleRate(&actual)!=ASE_OK||std::abs(actual-rate)>.01||
                   driver->getBufferSize(&l,&h,&p,&g)!=ASE_OK||p!=block){
                    std::puts("NOT RUN: configuration changed on reopen");drivers.asioCloseDriver(selected);break;
                }
                std::printf("Case %d instances mode=%d (%s) attempt=%d\n",count,mode,modeName(mode),attempt+1);
                bool retry=false;
                code=runCase(factory,driver,int(rate+.5),int(block),count,mode,out,retry,probe.seconds);
                drivers.asioCloseDriver(selected);
                if(!retry)break;
                std::puts("Reset recovery: driver fully closed; retry with same hardware configuration");
                Sleep(750);
            }
            failures+=code==4;incomplete+=code==3;
        }
    }
    out.close();check(!out.fail(),"CSV close");
    factory->release();check(exitModule(),"module exit");FreeLibrary(dll);CoUninitialize();
    std::printf("COMPLETE %s failed_cases=%d incomplete_cases=%d; no acoustic loopback proof\n",incomplete?"INCOMPLETE":failures?"FAIL":"PASS",failures,incomplete);
    return incomplete?3:failures?4:0;
}
