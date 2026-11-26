// Local performance host: load the exact P030d Windows module, not linked DSP.
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <objbase.h>
#include "vst3/ids.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "base/source/fstreamer.h"
#include "public.sdk/source/vst/utility/memoryibstream.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <thread>
#include <vector>
#include <string>
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
template<int n,int instances> static int practiceCase(IPluginFactory* factory,std::ostream& out,int rate,int seconds){
    using Clock=std::chrono::steady_clock;
    constexpr int warmup=128;
    // At least three seconds of audio per case, including several freeze recaptures.
    const int groups=std::max(100,(seconds*rate+n-1)/n);
    const double budget=1.e6*n/rate;
    const auto period=std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(double(n)/rate));
    int failures=0;
    for(int mode=0;mode<2;++mode){
        std::array<std::unique_ptr<LoadedProcessor>,instances> processors;
        for(auto& p:processors){
            p=std::make_unique<LoadedProcessor>(factory);
            check(p->initialize(nullptr)==kResultOk,"R01 init");
            check(p->activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p->activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"R01 buses");
            Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,n,rate};
            check(p->setupProcessing(setup)==kResultOk,"R01 setup");
            ResizableMemoryIBStream s;IBStreamer w(&s,kLittleEndian);
            check(w.writeInt32(kStateMagic)&&w.writeInt32(kStateVersion),"R01 state header");
            for(int i=0;i<9;++i)check(w.writeFloat(mode?.85f:(i==0?.45f:i==1?.35f:i==4?.50f:0.f)),"practice macro");
            check(w.writeFloat(mode?.15f:.25f)&&w.writeInt32(0),"practice globals");
            for(int i=0;i<3;++i)check(w.writeFloat(mode?.85f:(i==1?.55f:0.f)),"practice appended macro");
            s.rewind();check(p->setState(&s)==kResultOk&&p->setActive(true)==kResultOk&&p->setProcessing(true)==kResultOk,"R01 activate");
        }
        std::array<float,n> left{},right{};
        std::array<std::array<float,n>,instances> ol{},orr{};
        float* ins[]{left.data(),right.data()};
        Vst::AudioBusBuffers bi{};bi.numChannels=2;bi.channelBuffers32=ins;
        std::array<Vst::AudioBusBuffers,instances> bo{};
        std::array<std::array<float*,2>,instances> ptrs{};
        for(int i=0;i<instances;++i){ptrs[i]={ol[i].data(),orr[i].data()};bo[i].numChannels=2;bo[i].channelBuffers32=ptrs[i].data();}
        constexpr std::array<Vst::ParamID,14> ids{100,101,102,103,104,105,106,107,108,109,110,111,kTherapy,kBypass};
        Vst::ParameterChanges changes(14);
        std::vector<double> elapsed;elapsed.reserve(groups);
        std::vector<Sample> samples; samples.reserve(groups);
        int lateFinishes=0; double peak=0.;
        int missed=0;
        std::vector<unsigned long long> cycles;cycles.reserve(groups);
        auto deadline=Clock::now();
        for(int g=-warmup;g<groups;++g){
            for(int i=0;i<n;++i){
                const double time=double((g+warmup)*n+i)/rate;
                left[i]=.23f*tone(173.,time)+.13f*tone(711.,time);
                right[i]=.21f*tone(257.,time)+.12f*tone(997.,time);
            }
            changes.clearQueue();
            if(mode)for(int lane=0;lane<14;++lane){
                int32 qi=0,pi=0;auto* q=changes.addParameterData(ids[lane],qi);check(q!=nullptr,"R01 queue");
                for(int i=0;i<n;++i){
                    const double v=lane<12?.75+.2*double((g+warmup+i+lane)%17)/16.:
                        lane==12?.05+.2*double((g+warmup+i)%19)/18.:double(((g+warmup)*n+i)/97%2);
                    check(q->addPoint(i,v,pi)==kResultOk,"R01 dense points");
                }
            }
            if(g>=0){std::this_thread::sleep_until(deadline);}
            const auto planned=deadline;
            unsigned long long cycleBegin=0,cycleEnd=0;
#ifdef _WIN32
            check(QueryThreadCycleTime(GetCurrentThread(),&cycleBegin)!=0,"R01 thread cycle begin");
#endif
            const auto begin=Clock::now();
            const double startLate=g>=0?std::max(0.,std::chrono::duration<double,std::micro>(begin-planned).count()):0.;
            bool ok=true;
            for(int i=0;i<instances;++i){
                Vst::ProcessData d{};d.processMode=Vst::kRealtime;d.symbolicSampleSize=Vst::kSample32;
                d.numSamples=n;d.numInputs=d.numOutputs=1;d.inputs=&bi;d.outputs=&bo[i];
                d.inputParameterChanges=mode?&changes:nullptr;
                ok=(processors[i]->process(d)==kResultOk)&&ok;
            }
            const auto end=Clock::now();
#ifdef _WIN32
            check(QueryThreadCycleTime(GetCurrentThread(),&cycleEnd)!=0,"R01 thread cycle end");
#endif
            const double us=std::chrono::duration<double,std::micro>(end-begin).count();
            check(ok,"R01 process");
            for(int i=0;i<instances;++i)for(int j=0;j<n;++j)
                {check(std::isfinite(ol[i][j])&&std::isfinite(orr[i][j]),"R01 finite");peak=std::max(peak,std::max(std::abs(double(ol[i][j])),std::abs(double(orr[i][j]))));}
            if(g>=0){
                elapsed.push_back(us);missed+=us>budget;cycles.push_back(cycleEnd-cycleBegin);
                const bool finishLate=startLate+us>budget;lateFinishes+=finishLate;
                samples.push_back({us,startLate,cycleEnd-cycleBegin,finishLate});
                deadline=planned+period;
                if(deadline<end)deadline=end;
            }else deadline=end;
        }
        for(int g=0;g<groups;++g){const auto& x=samples[g];out<<rate<<','<<n<<','<<instances<<','<<mode<<','<<g<<','<<x.wall<<','<<budget<<','<<(x.wall>budget)<<','<<x.cycles<<','<<x.startLate<<','<<x.finishLate<<'\n';}
        out.flush();check(out.good(),"CSV write");
        std::sort(elapsed.begin(),elapsed.end());
        std::sort(cycles.begin(),cycles.end());
        std::printf("PRACTICE %dHz/%d samples %d instances mode=%s p50=%.3f p95=%.3f p99=%.3f max=%.3f us misses=%d/%d budget=%.3f us median cycles=%llu status=%s\n",
            rate,n,instances,mode?"high-dense":"representative-steady",elapsed[groups/2],elapsed[groups*95/100],elapsed[groups*99/100],elapsed.back(),missed,groups,budget,cycles[groups/2],missed?"FAIL":"PASS");
        std::printf("  p99 load %.1f%%; synthetic finish-late %d/%d; output peak %.6f\n",100.*elapsed[groups*99/100]/budget,lateFinishes,groups,peak);std::fflush(stdout);
        check(peak>1.e-8,"nonzero audio");
        failures+=missed;
        for(auto& p:processors)check(p->setProcessing(false)==kResultOk&&p->setActive(false)==kResultOk&&p->terminate()==kResultOk,"R01 terminate");
    }
    return failures;
}


int wmain(int argc,wchar_t** argv){
    std::setvbuf(stdout,nullptr,_IONBF,0);std::setvbuf(stderr,nullptr,_IONBF,0);
    if(argc!=3&&argc!=4){std::fprintf(stderr,"Usage: performance.exe module.vst3 result.csv [--smoke]\n");return 3;}
    const bool smoke=argc==4&&std::wstring(argv[3])==L"--smoke";
    if(argc==4&&!smoke)return 3;
    check(SUCCEEDED(CoInitialize(nullptr)),"COM initialization");
    check(timeBeginPeriod(1)==TIMERR_NOERROR,"Windows timer resolution");
    const auto path=std::filesystem::absolute(argv[1]);
    HMODULE dll=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if(!dll)std::fprintf(stderr,"LoadLibraryExW Windows error %lu\n",GetLastError());
    check(dll!=nullptr,"load exact plugin DLL");
    auto init=reinterpret_cast<bool(*)()>(GetProcAddress(dll,"InitDll"));
    auto exitModule=reinterpret_cast<bool(*)()>(GetProcAddress(dll,"ExitDll"));
    auto factoryFn=reinterpret_cast<IPluginFactory*(PLUGIN_API*)()>(GetProcAddress(dll,"GetPluginFactory"));
    check(init&&exitModule&&factoryFn&&init(),"VST3 module init");
    auto* factory=factoryFn();check(factory&&factory->countClasses()==2,"VST3 factory");
    std::ofstream out{std::filesystem::path(argv[2])};check(out.good(),"CSV open");
    out<<std::setprecision(12);
    out<<"sample_rate,block,instances,mode,group,wall_us,deadline_us,miss,thread_cycles,start_late_us,synthetic_finish_late\n";
    std::printf("125A PSYCHIATRY P030d local DLL test; 0=representative, 1=all high+dense automation\n");
    std::printf("Callback wall time vs block budget. Synthetic scheduler lateness separately; NOT ASIO.\n");
    int misses=0;
    if(smoke)misses+=practiceCase<64,1>(factory,out,48000,1);
    else for(int rate:{44100,48000}){
        misses+=practiceCase<256,1>(factory,out,rate,10);misses+=practiceCase<256,4>(factory,out,rate,10);
        misses+=practiceCase<128,1>(factory,out,rate,10);misses+=practiceCase<128,4>(factory,out,rate,10);
        misses+=practiceCase<64,1>(factory,out,rate,10);misses+=practiceCase<64,4>(factory,out,rate,10);
    }
    out.close();check(!out.fail(),"CSV close");
    factory->release();check(exitModule(),"module exit");FreeLibrary(dll);timeEndPeriod(1);CoUninitialize();
    std::printf("COMPLETE: callback deadline %s, misses=%d; no native ASIO claim\n",misses?"FAIL":"PASS",misses);
    return misses?4:0;
}
