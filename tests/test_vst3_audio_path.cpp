// P010: render audio through the ACTUAL VST3 Processor::process callback,
// not a separately reimplemented DSP path. Check 0/100 endpoint invariants,
// inverse THERAPY, 9 real wet algorithms, level-independent residual against
// dry (to reject gain-only 'placebo'), and sample-accurate VST3 automation.
// Synthetic bass, drums, guitar, voice-like and atmosphere sources; the test
// makes no claim to be a human listening evaluation.
#include "vst3/processor.h"
#include "vst3/ids.h"
#include "base/source/fstreamer.h"
#include "public.sdk/source/vst/utility/memoryibstream.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace Steinberg;
using namespace psychiatry::vst3;
namespace {
constexpr double kPi=3.14159265358979323846;
constexpr int kRate=48000;
constexpr int kFrames=60000; // 1.25 sec, sufficient for 0.2sec reverse segments
constexpr int kBlock=256;
constexpr std::array<Vst::ParamID,9> kMacroIds{
    kDepression,kParanoia,kSchizophrenia,kMania,kPanic,
    kDissociation,kObsession,kHallucination,kPsychosis};
constexpr std::array<const char*,9> kMacroNames{
    "DEPRESSION","PARANOIA","SCHIZOPHRENIA","MANIA","PANIC",
    "DISSOCIATION","OBSESSION","HALLUCINATION","PSYCHOSIS"};
constexpr std::array<const char*,5> kInputs{
    "synth_bass","drums","guitar","voice_like","atmosphere"};
struct Stereo {
    std::vector<float> l,r;
    explicit Stereo(std::size_t count=0):l(count),r(count){}
    std::size_t size() const{return l.size();}
};
void check(bool result,const char* why){
    if(!result){std::fprintf(stderr,"P010 FAIL: %s\n",why);std::exit(3);}
}
float tone(double hz,double t){
    return static_cast<float>(std::sin(2.*kPi*hz*t));
}
Stereo source(unsigned type){
    Stereo s(kFrames);
    std::uint32_t seed=0x125AF91Du;
    for(int i=0;i<kFrames;++i){
        const double t=static_cast<double>(i)/kRate;
        seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
        const float noise=static_cast<float>(seed&0xffffu)/32768.f-1.f;
        float left=0.f,right=0.f;
        if(type==0){
            const float env=0.35f+0.65f*std::exp(-static_cast<float>(i%12000)/3500.f);
            left=env*(0.35f*tone(73.416,t)+0.11f*tone(146.832,t)+0.055f*tone(367.08,t));
            right=env*(0.33f*tone(73.416,t)+0.10f*tone(220.25,t)+0.045f*tone(366.9,t));
        } else if(type==1){
            const int k=i%12000,n=i%24000;
            const float kick=k<3200 ? 0.64f*std::exp(-static_cast<float>(k)/640.f)*
                       tone(75.+60.*std::exp(-static_cast<double>(k)/460.),t):0.f;
            const float sn=n>12000&&n<14500?
                       0.35f*std::exp(-static_cast<float>(n-12000)/460.f)*noise:0.f;
            const float hat=k%3000<360 ? 0.12f*std::exp(-static_cast<float>(k%3000)/100.f)*noise:0.f;
            left=kick+sn+hat;right=.94f*kick+.81f*sn-.9f*hat;
        } else if(type==2){
            const float gate=0.5f+0.5f*(tone(3.8,t)>0.0f?1.f:0.f);
            const float base=0.26f*tone(98.,t)+0.14f*tone(196.,t)+0.10f*tone(294.,t);
            left=gate*std::tanh(2.5f*base);
            right=gate*std::tanh(2.3f*(base+0.025f*tone(578.,t)));
        } else if(type==3){
            const float phrasing=0.18f+0.82f*std::max(0.f,tone(1.6,t));
            const float pitch=118.f+3.5f*tone(2.4,t);
            float sum=0.f;
            for(int h=1;h<=10;++h){
                const float freq=pitch*h;
                const float w=0.03f+0.12f*std::exp(-std::pow((freq-525.f)/290.f,2.f))+
                                            0.09f*std::exp(-std::pow((freq-1420.f)/370.f,2.f));
                sum+=w*tone(freq,t);
            }
            left=phrasing*sum;right=phrasing*(sum*.95f+0.025f*tone(242.,t));
        } else {
            const float env=0.4f+0.6f*tone(.37,t);
            left=0.22f*tone(42.,t)+0.15f*tone(61.2,t)+0.08f*tone(512.,t)+0.036f*env*noise;
            right=0.20f*tone(42.6,t)+0.14f*tone(60.8,t)+0.075f*tone(508.,t)-0.033f*env*noise;
        }
        s.l[static_cast<std::size_t>(i)]=left;
        s.r[static_cast<std::size_t>(i)]=right;
    }
    return s;
}
void writeState(ResizableMemoryIBStream& state,int macro,float amount,
                float therapy,bool bypass){
    IBStreamer w(&state,kLittleEndian);
    check(w.writeInt32(kStateMagic)&&w.writeInt32(kStateVersion),"state header");
    for(unsigned i=0;i<9;++i)
        check(w.writeFloat(static_cast<int>(i)==macro?amount:0.f),"macro state");
    check(w.writeFloat(therapy)&&w.writeInt32(bypass?1:0),"therapy/bypass state");
    state.rewind();
}
Stereo render(const Stereo& dry,int macro,float amount,
              float therapy=0.f,bool bypass=false,bool inplace=false) {
    Processor p;
    check(p.initialize(nullptr)==kResultOk,"real VST3 initialize");
    Vst::ProcessSetup setup{};
    setup.processMode=Vst::kRealtime;
    setup.symbolicSampleSize=Vst::kSample32;
    setup.maxSamplesPerBlock=kBlock;
    setup.sampleRate=kRate;
    check(p.setupProcessing(setup)==kResultOk,"real VST3 setupProcessing");
    ResizableMemoryIBStream state;
    writeState(state,macro,amount,therapy,bypass);
    check(p.setState(&state)==kResultOk,"real VST3 component state");
    check(p.setActive(true)==kResultOk,"real VST3 setActive");
    Stereo out(dry.size());
    for(int start=0;start<kFrames;start+=kBlock){
        const int n=std::min(kBlock,kFrames-start);
        auto* p0=const_cast<float*>(dry.l.data())+start;
        auto* p1=const_cast<float*>(dry.r.data())+start;
        if(inplace){
            std::copy_n(p0,n,out.l.data()+start);
            std::copy_n(p1,n,out.r.data()+start);
            p0=out.l.data()+start;
            p1=out.r.data()+start;
        }
        float* in[]={p0,p1};
        float* o[]={out.l.data()+start,out.r.data()+start};
        Vst::AudioBusBuffers bIn{},bOut{};
        bIn.numChannels=2;bOut.numChannels=2;
        bIn.channelBuffers32=in;bOut.channelBuffers32=o;
        Vst::ProcessData data{};
        data.processMode=Vst::kRealtime;
        data.symbolicSampleSize=Vst::kSample32;
        data.numSamples=n;
        data.numInputs=1;data.numOutputs=1;
        data.inputs=&bIn;data.outputs=&bOut;
        check(p.process(data)==kResultOk,"actual VST3 Processor::process");
    }
    check(p.setActive(false)==kResultOk,"deactivate real VST3");
    check(p.terminate()==kResultOk,"terminate real VST3");
    return out;
}
bool identical(const Stereo& a,const Stereo& b){
    return a.l==b.l && a.r==b.r;
}
struct Metric {
    double dryRms=0,wetRms=0,changeDb=0,peak=0;
    double nullRatio=0,shapeResidual=0,fullScaleDb=0;
};
Metric measure(const Stereo& dry,const Stereo& wet){
    Metric m; double xx=0,yy=0,xy=0,diff=0;
    for(std::size_t i=0;i<dry.size();++i){
        const std::array<double,2> x{dry.l[i],dry.r[i]};
        const std::array<double,2> y{wet.l[i],wet.r[i]};
        for(int c=0;c<2;++c){
            check(std::isfinite(y[c]),"non-finite plugin output");
            xx+=x[c]*x[c];yy+=y[c]*y[c];xy+=x[c]*y[c];
            diff+=(x[c]-y[c])*(x[c]-y[c]);
            m.peak=std::max(m.peak,std::abs(y[c]));
        }
    }
    const double samples=2.*dry.size();
    m.dryRms=std::sqrt(xx/samples);m.wetRms=std::sqrt(yy/samples);
    m.changeDb=20.*std::log10(std::max(m.wetRms,1.e-12)/std::max(m.dryRms,1.e-12));
    m.nullRatio=std::sqrt(diff/std::max(xx,1.e-12));
    const double gain=xy/std::max(xx,1.e-12);
    const double residual=std::max(0.,yy-2.*gain*xy+gain*gain*xx);
    m.shapeResidual=std::sqrt(residual/std::max(yy,1.e-12));
    m.fullScaleDb=20.*std::log10(std::max(m.peak,1.e-12));
    return m;
}
// Negative control: a pure gain change MUST NOT be mistaken for a new effect.
void rejectGainPlacebo(const Stereo& dry){
    Stereo gain=dry;
    for(auto& x:gain.l)x*=.41f;
    for(auto& x:gain.r)x*=.41f;
    const auto m=measure(dry,gain);
    check(m.nullRatio>.30,"negative-control raw null should differ");
    check(m.shapeResidual<1.e-5,"negative-control gain-only fails spectral-identity check");
}
void write16(std::ofstream& f,std::uint16_t v){
    f.put(static_cast<char>(v&255));f.put(static_cast<char>((v>>8)&255));
}
void write32(std::ofstream& f,std::uint32_t v){
    for(int i=0;i<4;++i)f.put(static_cast<char>((v>>(i*8))&255));
}
void writeWav(const std::string& filename,const Stereo& s){
    std::ofstream f(filename,std::ios::binary);
    check(f.good(),"open WAV output");
    const std::uint32_t dataLen=static_cast<std::uint32_t>(s.size()*4);
    f.write("RIFF",4);write32(f,dataLen+36);
    f.write("WAVEfmt ",8);write32(f,16);
    write16(f,1);write16(f,2);write32(f,kRate);write32(f,kRate*4);
    write16(f,4);write16(f,16);
    f.write("data",4);write32(f,dataLen);
    for(std::size_t i=0;i<s.size();++i){
        for(const auto v:{s.l[i],s.r[i]}){
            check(std::isfinite(v),"WAV contains NaN");
            const auto clipped=std::clamp(v,-1.f,1.f);
            write16(f,static_cast<std::uint16_t>(static_cast<std::int16_t>(
                std::lround(clipped*32767.f))));
        }
    }
    check(f.good(),"write WAV");
}
void checkTherapy(const Stereo& dry,int effect){
    const Stereo wet=render(dry,effect,1.f,0.f);
    const Stereo half=render(dry,effect,1.f,.5f);
    const Stereo fullyDry=render(dry,effect,1.f,1.f);
    check(identical(dry,fullyDry),"THERAPY=100% MUST return bit-exact dry");
    double worst=0;
    for(std::size_t i=0;i<dry.size();++i){
        const double targetLeft=static_cast<double>(dry.l[i])+
                           .5*(static_cast<double>(wet.l[i])-dry.l[i]);
        const double targetRight=static_cast<double>(dry.r[i])+
                           .5*(static_cast<double>(wet.r[i])-dry.r[i]);
        worst=std::max(worst,std::abs(static_cast<double>(half.l[i])-targetLeft));
        worst=std::max(worst,std::abs(static_cast<double>(half.r[i])-targetRight));
    }
    if(worst>5.e-6)std::fprintf(stderr,"P010 THERAPY mismatch macro=%s max=%g\n",kMacroNames[effect],worst);
    check(worst<=5.e-6,"THERAPY=50% must be half dry + half wet");
}
// Proves the VST3 parameter automation queue reaches audio at the precise
// sample offset. MANIA is chosen because it has no lookahead/reverse latency.
void testAutomation(){
    Processor p;
    check(p.initialize(nullptr)==kResultOk,"automation init");
    Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,kBlock,kRate};
    check(p.setupProcessing(setup)==kResultOk,"automation setup");
    ResizableMemoryIBStream initial;writeState(initial,-1,0.f,0.f,false);
    check(p.setState(&initial)==kResultOk,"automation neutral state");
    check(p.setActive(true)==kResultOk,"automation active");
    Vst::ParameterChanges changes(1);
    int32 index=0,point=0;
    auto* q=changes.addParameterData(kMania,index);
    check(q && q->addPoint(128,1.0,point)==kResultTrue,"automation point at sample 128");
    std::array<float,256> inL{},inR{},outL{},outR{};
    for(int i=0;i<256;++i){
        inL[i]=.32f*tone(220.,static_cast<double>(i)/kRate);
        inR[i]=.28f*tone(315.,static_cast<double>(i)/kRate);
    }
    float* src[]{inL.data(),inR.data()};
    float* dst[]{outL.data(),outR.data()};
    Vst::AudioBusBuffers bIn{},bOut{};
    bIn.numChannels=2;bIn.channelBuffers32=src;
    bOut.numChannels=2;bOut.channelBuffers32=dst;
    Vst::ProcessData data{};
    data.processMode=Vst::kRealtime;
    data.symbolicSampleSize=Vst::kSample32;
    data.numSamples=256;data.numInputs=1;data.numOutputs=1;
    data.inputs=&bIn;data.outputs=&bOut;
    data.inputParameterChanges=&changes;
    check(p.process(data)==kResultOk,"automation VST3 process");
    bool changed=false;
    for(int i=0;i<256;++i){
        if(i<128)check(outL[i]==inL[i]&&outR[i]==inR[i],
                       "automation changed signal BEFORE event sample");
        if(i>=128 && (std::abs(outL[i]-inL[i])>1.e-5f||
                      std::abs(outR[i]-inR[i])>1.e-5f))changed=true;
    }
    check(changed,"automation event did not cause audio change");
    check(p.setActive(false)==kResultOk,"automation deactivate");
    check(p.terminate()==kResultOk,"automation terminate");
}
} // namespace
int main(int argc,char** argv){
    std::ofstream csv;
    const bool exportWav=argc>1;
    if(exportWav){
        const std::string path=argv[1];
        csv.open(path+"/P010_actual_vst3_audio_metrics.csv");
        check(csv.good(),"create CSV");
        csv<<"source,macro,amount,therapy,dry_rms,output_rms,gain_db,peak_dbfs,raw_null_ratio,gain_matched_residual\n";
    }
    testAutomation();
    int substantial=0,weak=0;
    for(unsigned input=0;input<kInputs.size();++input){
        const Stereo dry=source(input);
        rejectGainPlacebo(dry);
        const Stereo allZero=render(dry,-1,0.f,0.f);
        check(identical(dry,allZero),"all zero macros + THERAPY 0 not bit-exact dry");
        const Stereo bypass=render(dry,3,1.f,0.f,true);
        check(identical(dry,bypass),"native Bypass must be bit-exact dry");
        if(input==0 && exportWav)writeWav(std::string(argv[1])+"/P010_DRY_Bass.wav",dry);
        for(int effect=0;effect<9;++effect){
            const Stereo zero=render(dry,effect,0.f);
            check(identical(dry,zero),"individual 0% is not bit-exact dry");
            for(const float amount:{.20f,.50f,.75f,1.f}){
                const Stereo wet=render(dry,effect,amount,0.f);
                const Metric m=measure(dry,wet);
                if(input==0 && amount==1.f && exportWav)
                    writeWav(std::string(argv[1])+"/P010_"+std::string(kMacroNames[effect])+"_WET100.wav",wet);
                if(amount==1.f) {
                    if(m.shapeResidual>=.035 && m.nullRatio>=.06)
                        ++substantial;
                    else{
                        ++weak;
                        std::fprintf(stderr,
                            "P010 WEAK_FULL_WET %s %s gain=%+.2fdB residual=%.4f null=%.4f\n",
                            kInputs[input],kMacroNames[effect],
                            m.changeDb,m.shapeResidual,m.nullRatio);
                    }
                }
                if(csv.good())csv<<kInputs[input]<<","<<kMacroNames[effect]<<","
                    <<std::fixed<<std::setprecision(6)<<amount<<",0,"
                    <<m.dryRms<<","<<m.wetRms<<","<<m.changeDb<<","
                    <<m.fullScaleDb<<","<<m.nullRatio<<","<<m.shapeResidual<<"\n";
                if(amount==1.f)
                    std::printf("P010 full wet: %-13s %-16s gain=%+7.2f dB rawNull=%6.3f gainMatchedResidual=%6.3f\n",
                        kInputs[input],kMacroNames[effect],
                        m.changeDb,m.nullRatio,m.shapeResidual);
            }
            if(input==0)checkTherapy(dry,effect);
        }
    }
    std::printf("P010 VST3 audio processor endpoint invariants PASS; substantive full-wet cases=%d/45, WEAK=%d; negative gain-placebo control PASS; sample-accurate automation PASS\n",
        substantial,weak);
    // Real counterexample must fail, not be disguised by loudness. Zero effect,
    // exact dry, therapy endpoints, placebo negatives always fail CTest.
    // Weak musical transformations are reported as evidence and kept for review.
    check(weak==0,"at least one 100%-wet effect/source has insufficient gain-independent change");
    return 0;
}
