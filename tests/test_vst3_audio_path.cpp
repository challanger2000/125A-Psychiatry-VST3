// P010: render audio through the ACTUAL VST3 Processor::process callback,
// not a separately reimplemented DSP path. Check 0/100 endpoint invariants,
// inverse THERAPY, 9 real wet algorithms, level-independent residual against
// dry (to reject gain-only 'placebo'), and sample-accurate VST3 automation.
// Synthetic bass, drums, guitar, voice-like and atmosphere sources; the test
// makes no claim to be a human listening evaluation.
#include "vst3/processor.h"
#include "vst3/ids.h"
#include "vst3/loss_of_control.h"
#include "base/source/fstreamer.h"
#include "public.sdk/source/vst/utility/memoryibstream.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include <algorithm>
#include <array>
#include <chrono>
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
constexpr std::array<Vst::ParamID,12> kMacroIds{
    kDepression,kParanoia,kSchizophrenia,kMania,kPanic,
    kDissociation,kObsession,kHallucination,kPsychosis,kDelirium,kCatatonia,kAmnesia};
constexpr std::array<const char*,12> kMacroNames{
    "DEPRESSION","PARANOIA","SCHIZOPHRENIA","MANIA","PANIC",
    "DISSOCIATION","OBSESSION","HALLUCINATION","PSYCHOSIS","DELIRIUM","CATATONIA","AMNESIA"};
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
    for(int i=9;i<12;++i)check(w.writeFloat(i==macro?amount:0.f),"v5 macro state");
    state.rewind();
}
constexpr int kLatency=static_cast<int>(Processor::kLatency);
Stereo compensate(const Stereo& raw,std::size_t size) {
    check(raw.size()>=size+kLatency,"P018 padded capture");
    Stereo result(size);
    std::copy_n(raw.l.begin()+kLatency,size,result.l.begin());
    std::copy_n(raw.r.begin()+kLatency,size,result.r.begin());
    return result;
}
Stereo padded(const Stereo& dry) {
    Stereo x(dry.size()+kLatency);
    std::copy(dry.l.begin(),dry.l.end(),x.l.begin());
    std::copy(dry.r.begin(),dry.r.end(),x.r.begin());
    return x;
}
Stereo render(const Stereo& dry,int macro,float amount,
              float therapy=0.f,bool bypass=false,bool inplace=false,double sampleRate=kRate,
              bool compensateLatency=true,const std::array<float,12>* combination=nullptr) {
    Processor p;
    check(p.initialize(nullptr)==kResultOk,"real VST3 initialize");
        check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"activate audio buses");
    Vst::ProcessSetup setup{};
    setup.processMode=Vst::kRealtime;
    setup.symbolicSampleSize=Vst::kSample32;
    setup.maxSamplesPerBlock=kBlock;
    setup.sampleRate=sampleRate;
    check(p.setupProcessing(setup)==kResultOk,"real VST3 setupProcessing");
    ResizableMemoryIBStream state;
    if(combination){
        IBStreamer w(&state,kLittleEndian);
        check(w.writeInt32(kStateMagic)&&w.writeInt32(kStateVersion),"P027c combo header");
        for(int i=0;i<9;++i)check(w.writeFloat((*combination)[i]),"P027c macro");
        check(w.writeFloat(therapy)&&w.writeInt32(bypass?1:0),"P027c globals");
        for(int i=9;i<12;++i)check(w.writeFloat((*combination)[i]),"P027c appended macro");
        state.rewind();
    }else writeState(state,macro,amount,therapy,bypass);
    check(p.setState(&state)==kResultOk,"real VST3 component state");
    check(p.setActive(true)==kResultOk,"real VST3 setActive");
    check(p.getLatencySamples()==kLatency,"P018 fixed reported latency");
    Stereo input=padded(dry),out(input.size());
    for(std::size_t start=0;start<input.size();start+=kBlock){
        const int n=static_cast<int>(std::min<std::size_t>(kBlock,input.size()-start));
        auto* p0=input.l.data()+start;
        auto* p1=input.r.data()+start;
        if(inplace){
            std::copy_n(p0,n,out.l.data()+start);
            std::copy_n(p1,n,out.r.data()+start);
            p0=out.l.data()+start;p1=out.r.data()+start;
        }
        float* in[]={p0,p1};float* o[]={out.l.data()+start,out.r.data()+start};
        Vst::AudioBusBuffers bIn{},bOut{};
        bIn.numChannels=2;bOut.numChannels=2;
        bIn.channelBuffers32=in;bOut.channelBuffers32=o;
        Vst::ProcessData data{};
        data.processMode=Vst::kRealtime;data.symbolicSampleSize=Vst::kSample32;
        data.numSamples=n;data.numInputs=1;data.numOutputs=1;
        data.inputs=&bIn;data.outputs=&bOut;
        check(p.process(data)==kResultOk,"actual VST3 Processor::process");
    }
    check(p.setActive(false)==kResultOk,"deactivate real VST3");
    check(p.terminate()==kResultOk,"terminate real VST3");
    if(compensateLatency)return compensate(out,dry.size());
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
// Reference the actual effect classes at exactly 100% wet. This is not
// a mock output: it is the same real DSP class compiled into the plugin.
// All other VST3 macro stages are at 0%; the plugin's output MUST match
// this wet-only signal without a parallel undistorted/dry mix.
template<class FX>
void directSimple(FX& effect,Stereo& processed,float amount) {
    effect.prepare(kRate);effect.setAmount(amount);effect.reset();
    for(std::size_t start=0;start<processed.size();start+=kBlock){
        const auto n=std::min<std::size_t>(kBlock,processed.size()-start);
        float* ptr[]{processed.l.data()+start,processed.r.data()+start};
        effect.process(ptr,2,n);
    }
}
Stereo pureWetReference(const Stereo& dry,int effect) {
    // Independently run the nine concrete stages in their documented order,
    // including neutral stages' constant delays, then compensate exactly once.
    Stereo ref=padded(dry);
    psychiatry::Depression d;directSimple(d,ref,effect==0?1.f:0.f);
    psychiatry::Schizophrenia s;directSimple(s,ref,effect==2?1.f:0.f);
    psychiatry::Mania m;directSimple(m,ref,effect==3?1.f:0.f);
    psychiatry::Panic pn;directSimple(pn,ref,effect==4?1.f:0.f);
    psychiatry::Paranoia p;p.prepare(kRate);
    psychiatry::Paranoia::Settings ps;
    ps.amount=effect==1?1.f:0.f;ps.therapy=0.f;
    ps.trigger=psychiatry::Paranoia::Trigger::Hybrid;
    ps.pattern=psychiatry::Paranoia::Pattern::Every;
    p.setSettings(ps);p.reset();
    for(std::size_t start=0;start<ref.size();start+=kBlock){
        float* ptr[]{ref.l.data()+start,ref.r.data()+start};
        p.process(ptr,2,std::min<std::size_t>(kBlock,ref.size()-start));
    }
    psychiatry::Dissociation ds;directSimple(ds,ref,effect==5?1.f:0.f);
    psychiatry::Obsession o;directSimple(o,ref,effect==6?1.f:0.f);
    psychiatry::Hallucination h;directSimple(h,ref,effect==7?1.f:0.f);
    psychiatry::Psychosis z;directSimple(z,ref,effect==8?1.f:0.f);
    psychiatry::Delirium dl;directSimple(dl,ref,effect==9?1.f:0.f);
    psychiatry::Catatonia ct;directSimple(ct,ref,effect==10?1.f:0.f);
    psychiatry::Amnesia am;directSimple(am,ref,effect==11?1.f:0.f);
    return compensate(ref,dry.size());
}
double spectralBandDistance(const Stereo& a,const Stereo& b) {
    // Four actual frequency ranges by splitting low-pass outputs at
    // 140 Hz / 900 Hz / 3600 Hz. Compare normalized band energies,
    // therefore gain-only changes must return approximately zero.
    auto bands=[](const Stereo& s){
        std::array<double,4> energy{};
        const double alpha1=1.-std::exp(-2.*kPi*140./kRate);
        const double alpha2=1.-std::exp(-2.*kPi*900./kRate);
        const double alpha3=1.-std::exp(-2.*kPi*3600./kRate);
        for(const auto* input:{&s.l,&s.r}){
            double l1=0.,l2=0.,l3=0.;
            for(const float sample:*input){
                l1+=alpha1*(sample-l1);
                l2+=alpha2*(sample-l2);
                l3+=alpha3*(sample-l3);
                const std::array<double,4> p{
                    l1,l2-l1,l3-l2,static_cast<double>(sample)-l3};
                for(std::size_t k=0;k<4;++k)energy[k]+=p[k]*p[k];
            }
        }
        double total=0.;
        for(double v:energy)total+=v;
        for(auto& v:energy)v/=std::max(total,1.e-18);
        return energy;
    };
    const auto dry=bands(a),wet=bands(b);
    double distance=0.;
    for(std::size_t i=0;i<4;++i)
        distance+=std::pow(std::sqrt(dry[i])-std::sqrt(wet[i]),2.);
    return std::sqrt(distance);
}

// Negative control: a pure gain change MUST NOT be mistaken for a new effect.
void rejectGainPlacebo(const Stereo& dry){
    Stereo gain=dry;
    for(auto& x:gain.l)x*=.41f;
    for(auto& x:gain.r)x*=.41f;
    const auto m=measure(dry,gain);
    check(m.nullRatio>.30,"negative-control raw null should differ");
    check(m.shapeResidual<1.e-5,"negative-control gain-only fails spectral-identity check");
    check(spectralBandDistance(dry,gain)<1.e-5,
          "negative-control pure gain must preserve normalized spectral bands");
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
        check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"activate audio buses");
    Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,kBlock,kRate};
    check(p.setupProcessing(setup)==kResultOk,"automation setup");
    ResizableMemoryIBStream initial;writeState(initial,-1,0.f,0.f,false);
    check(p.setState(&initial)==kResultOk,"automation neutral state");
    check(p.setActive(true)==kResultOk,"automation active");
    Vst::ParameterChanges changes(1);
    int32 index=0,point=0;
    auto* q=changes.addParameterData(kMania,index);
    check(q && q->addPoint(127,0.0,point)==kResultTrue,"step anchor");
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
        const float dryL=i>=kLatency?inL[i-kLatency]:0.f;
        const float dryR=i>=kLatency?inR[i-kLatency]:0.f;
        if(i<128+kLatency)check(outL[i]==dryL&&outR[i]==dryR,
                       "automation changed signal BEFORE event sample");
        if(i>=128+kLatency && (std::abs(outL[i]-dryL)>1.e-5f||
                      std::abs(outR[i]-dryR)>1.e-5f))changed=true;
    }
    check(changed,"automation event did not cause audio change");
    check(p.setActive(false)==kResultOk,"automation deactivate");
    check(p.terminate()==kResultOk,"automation terminate");
}
// P012: high-band single-tone alias probe on the SAME real VST3 processor.
// 15 kHz at 48 kHz makes third-harmonic foldback land at exactly 3 kHz.
// This reports narrowband harmonic artifacts and does not claim to be
// a complete anti-alias specification or a listening test.
double toneMagnitude(const Stereo& signal,double hz,int from=12000,int length=48000,double sampleRate=kRate) {
    double co=0.,si=0.;
    const auto omega=2.*kPi*hz/sampleRate;
    for(int n=0;n<length;++n) {
        const auto pos=static_cast<std::size_t>(from+n);
        const double sample=.5*(static_cast<double>(signal.l[pos])+signal.r[pos]);
        const double angle=omega*n;
        co+=sample*std::cos(angle);
        si+=sample*std::sin(angle);
    }
    return 2.*std::hypot(co,si)/length;
}
void testHighBandAliasing(const std::string& dir) {
    std::ofstream csv;
    if(!dir.empty()) {
        csv.open(dir+"/P012_alias_and_cpu_metrics.csv");
        check(csv.good(),"P012 alias metrics open");
        csv<<"probe,effect,input_hz,alias_hz,main_rms_amp,alias_rms_amp,alias_relative_dbc,seconds_processed,elapsed_seconds,x_realtime,max_abs_sample\n";
    }
    for(double inputHz:{14000.,15000.}) {
        Stereo high(kFrames);
        for(int i=0;i<kFrames;++i){
            const float val=0.35f*tone(inputHz,static_cast<double>(i)/kRate);
            high.l[static_cast<std::size_t>(i)]=val;
            high.r[static_cast<std::size_t>(i)]=val;
        }
        const double aliasHz=std::abs(3.*inputHz-kRate);
        const auto baseline=toneMagnitude(high,aliasHz);
        const auto reference=toneMagnitude(high,inputHz);
        check(reference>.30,"P012 pure reference fundamental unexpectedly weak");
        check(baseline<reference*0.0005,
              "P012 negative reference contains a false folded harmonic");
        for(int effect:{-1,0,3,4,8}) {
            const auto processed=(effect==-1)?high:render(high,effect,1.f);
            const double fundamental=toneMagnitude(processed,inputHz);
            const double alias=toneMagnitude(processed,aliasHz);
            const double db=20.*std::log10(std::max(alias,1.e-15)/
                                          std::max(fundamental,1.e-15));
            double maxSample=0.;
            for(const float v:processed.l)
                maxSample=std::max(maxSample,std::abs(static_cast<double>(v)));
            std::printf("P012 SPECTRAL src=%.0fHz fx=%-13s folded=%.0fHz rel=%+.2fdBc main=%.6f alias=%.6f\n",
                inputHz,effect<0?"DRY":kMacroNames[effect],aliasHz,db,fundamental,alias);
            // This guard compares the measurable third-harmonic foldback
            // against our P012 pre-oversampling baseline (-18.85/-20.33).
            // It does NOT certify all sources or all alias harmonics.
            if(effect==8)check(db<=-45.0,
                "P012 PSYCHOSIS third-harmonic alias suppression insufficient");
            // P013: fail if the newly 2x oversampled wet shapers do not
            // substantially suppress the exact tested 3rd-harmonic fold.
            if(effect==0||effect==3)check(db<=-50.0,
                "P013 DEPRESSION/MANIA folded-harmonic alias above -50 dBc");
            // P014: the same real-VST3 3rd-harmonic stress probe protects
            // PANIC's new nonlinear wet-only oversampling.
            if(effect==4)check(db<=-45.0,
                "P014 PANIC folded-harmonic alias above -45 dBc");
            if(csv.good())csv<<"narrowband,"<<(effect<0?"DRY":kMacroNames[effect])
                <<","<<inputHz<<","<<aliasHz<<","<<fundamental<<","
                <<alias<<","<<db<<",,,, "<<maxSample<<"\n";
        }
    }
    // Real Windows runner CPU metrology (not a hard RT guarantee).
    // Both all-disabled and nine-at-85%-wet paths are timed on *the same*
    // ProcessData path as the DAW. We publish max block time as evidence.
    const Stereo signal=source(0);
    for(bool twelve:{false,true})for(float level:{0.f,.85f,1.f}) {
        Processor p;
        check(p.initialize(nullptr)==kResultOk,"P012 realtime initialize");
        check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"activate audio buses");
        Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,kBlock,kRate};
        check(p.setupProcessing(setup)==kResultOk,"P012 realtime setup");
        ResizableMemoryIBStream state;
        IBStreamer sw(&state,kLittleEndian);
        check(sw.writeInt32(kStateMagic)&&sw.writeInt32(twelve?kStateVersion:4),
              "P012 FX state header");
        for(int i=0;i<9;++i)check(sw.writeFloat(level),"P012 macro amount");
        check(sw.writeFloat(0.f)&&sw.writeInt32(0),"P012 therapy and bypass");
        if(twelve)for(int i=0;i<3;++i)check(sw.writeFloat(level),"P026k appended FX");
        state.rewind();
        check(p.setState(&state)==kResultOk,"P012 nine FX state load");
        check(p.setActive(true)==kResultOk,"P012 realtime activate");
        Stereo out(kFrames);
        double maxBlockMs=0.,peak=0.;
        const auto started=std::chrono::steady_clock::now();
        for(int start=0;start<kFrames;start+=kBlock) {
            const int n=std::min(kBlock,kFrames-start);
            float* in[2]{
                const_cast<float*>(signal.l.data())+start,
                const_cast<float*>(signal.r.data())+start};
            float* output[2]{out.l.data()+start,out.r.data()+start};
            Vst::AudioBusBuffers ins{},outs{};
            ins.numChannels=2;ins.channelBuffers32=in;
            outs.numChannels=2;outs.channelBuffers32=output;
            Vst::ProcessData data{};
            data.processMode=Vst::kRealtime;
            data.symbolicSampleSize=Vst::kSample32;
            data.numSamples=n;data.numInputs=1;data.numOutputs=1;
            data.inputs=&ins;data.outputs=&outs;
            const auto blockStart=std::chrono::steady_clock::now();
            check(p.process(data)==kResultOk,"P012 real nine FX callback");
            const double blockMs=std::chrono::duration<double,std::milli>(
                std::chrono::steady_clock::now()-blockStart).count();
            maxBlockMs=std::max(maxBlockMs,blockMs);
            for(int i=0;i<n;++i) {
                const double a=out.l[static_cast<std::size_t>(start+i)];
                const double b=out.r[static_cast<std::size_t>(start+i)];
                check(std::isfinite(a)&&std::isfinite(b),"P012 nine FX non-finite");
                peak=std::max({peak,std::abs(a),std::abs(b)});
            }
        }
        const double elapsed=std::chrono::duration<double>(
            std::chrono::steady_clock::now()-started).count();
        check(p.setActive(false)==kResultOk,"P012 realtime deactivate");
        check(p.terminate()==kResultOk,"P012 realtime terminate");
        const double rt=elapsed/(static_cast<double>(kFrames)/kRate);
        std::printf("P012 REAL VST3 CPU macros=%d amount=%.2f time=%.4fs xRealtime=%.4f max_callback_ms=%.4f peak=%.4f\n",
                    twelve?12:9,level,elapsed,rt,maxBlockMs,peak);
        if(csv.good())csv<<(twelve?"cpu_all_twelve,":"cpu_all_nine,")<<level
            <<",,,,,"<<(kFrames/static_cast<double>(kRate))
            <<","<<elapsed<<","<<rt<<","<<peak<<"\n";
        if(level==0.f)for(int n=0;n<kFrames;++n)
            check(out.l[n]==(n>=kLatency?signal.l[n-kLatency]:0.f)&&
                  out.r[n]==(n>=kLatency?signal.r[n-kLatency]:0.f),
                  "P012 nine-at-zero exact latency-matched dry");
    }
}
// P015: exercise the actual VST3 callback at 44.1/48/96 kHz, including
// bit-identical macro/THERAPY endpoints and high-band two-tone intermodulation.
// Do not call 96-kHz third harmonics "aliases": 42/45 kHz are below Nyquist.
// This is a diagnostic baseline; only verified spectral limits become guards.
void testMultirateIntermod(const std::string& dir) {
    std::ofstream csv;
    if(!dir.empty()) {
        csv.open(dir+"/P015_multirate_intermod_metrics.csv");
        check(csv.good(),"P015 multirate CSV open");
        csv<<"rate_hz,effect,probe,frequency_hz,reference_amp,measured_amp,relative_to_input_db,peak\n";
    }
    constexpr std::array<int,4> highProducts{42000,45000,43000,44000};
    constexpr std::array<const char*,4> labels{
        "3x14k","3x15k","2x14k+15k","2x15k+14k"};
    for(const int sr:{44100,48000,96000}) {
        // Exactly one second after warm-up gives integer-cycle bins at
        // 14/15 kHz and at every monitored two-tone product frequency.
        const int warm=12000;
        Stereo input(static_cast<std::size_t>(warm+sr));
        for(std::size_t n=0;n<input.size();++n) {
            const double t=static_cast<double>(n)/sr;
            const float v=0.175f*(tone(14000.,t)+tone(15000.,t));
            input.l[n]=v;
            input.r[n]=v;
        }
        const double amp14=toneMagnitude(input,14000.,warm,sr,sr);
        const double amp15=toneMagnitude(input,15000.,warm,sr,sr);
        const double reference=std::hypot(amp14,amp15);
        check(amp14>.17 && amp15>.17,"P015 source fundamentals invalid");
        for(int fx=0;fx<12;++fx) {
            // The 0% and global THERAPY=100% invariants are verified
            // for EVERY macro at EACH host sample rate, not just at 48k.
            const Stereo zero=render(input,fx,0.f,0.f,false,false,sr);
            check(identical(input,zero),"P015 0% macro not exact Dry");
            const Stereo therapy=render(input,fx,1.f,1.f,false,false,sr);
            check(identical(input,therapy),"P015 THERAPY=100% not exact Dry");
            const Stereo wet=render(input,fx,1.f,0.f,false,false,sr);
            double peak=0.;
            for(std::size_t n=0;n<wet.size();++n) {
                check(std::isfinite(wet.l[n]) && std::isfinite(wet.r[n]),
                      "P015 nonfinite high-rate VST3 sample");
                peak=std::max({peak,std::abs(static_cast<double>(wet.l[n])),
                                   std::abs(static_cast<double>(wet.r[n]))});
            }
            check(peak<4.,"P015 uncontrolled peak in two-tone high-band");
            if(csv.good())csv<<sr<<","<<kMacroNames[fx]
                <<",wet_peak,0,"<<reference<<",,,"<<peak<<"\n";
            if(fx!=0 && fx!=3 && fx!=4 && fx!=8)continue;
            // Two-tone excitation reveals intermodulation lines missed by
            // the P012-P014 isolated single-tone third-harmonic probes.
            for(std::size_t k=0;k<highProducts.size();++k) {
                const double product=highProducts[k];
                // At 96 kHz these frequencies are REAL third-order products,
                // not folded aliases. At 44.1/48 kHz they fold into baseband.
                const double inspected=sr==96000?product:std::abs(product-sr);
                const double measured=toneMagnitude(wet,inspected,warm,sr,sr);
                const double ratio=20.*std::log10(std::max(measured,1.e-15)/
                                                  std::max(reference,1.e-15));
                check(std::isfinite(ratio),"P015 spectral measurement nonfinite");
                // Input-referenced dB, not a ratio to the processed carrier.
                // On the verified 44.1/48-kHz P015 baseline every monitored
                // folded line is at or below -52.18 dB relative to Dry.
                // Keep a conservative 7.18-dB regression margin.
                if(sr!=96000)check(ratio<=-45.,
                    "P015 two-tone folded product above -45 dB vs Dry input");
                if(csv.good())csv<<sr<<","<<kMacroNames[fx]<<","
                    <<(sr==96000?"inband_":"folded_")<<labels[k]<<","
                    <<inspected<<","<<reference<<","<<measured<<","
                    <<ratio<<","<<peak<<"\n";
                std::printf("P015 rate=%d fx=%s product=%s frequency=%.0fHz rel=%+.2fdB(input)\n",
                            sr,kMacroNames[fx],labels[k],inspected,ratio);
                // Avoid unearned pass/fail targets for new intermodulation
                // lines until they have a measured Windows baseline.
            }
        }
        // Prove the reference pair itself contains no spur at the first
        // alias location, so numerical FFT/window leakage is not mistaken
        // for real signal generation. This line is 2100/6000 Hz.
        if(sr!=96000) {
            const double drySpur=toneMagnitude(input,std::abs(42000-sr),
                                                warm,sr,sr);
            check(drySpur<reference*1.e-4,
                  "P015 multi-tone negative control contains folded spur");
        }
    }
}
// P016: stimulus-specific phase and transient measurements of actual
// nonlinear, time-varying effects. Do not interpret as LTI transfer curves.
std::array<double,2> projection(const std::vector<float>& x,double hz,
                                 int start,int count,int sr) {
    double re=0.,im=0.;
    for(int n=0;n<count;++n) {
        const double theta=2.*kPi*hz*(start+n)/sr;
        const double s=x[static_cast<std::size_t>(start+n)];
        re+=s*std::cos(theta);im-=s*std::sin(theta);
    }
    return {2.*re/count,2.*im/count};
}
void testPhaseAndTransients(const std::string& dir) {
    std::ofstream phase,transient;
    if(!dir.empty()) {
        phase.open(dir+"/P016_phase_metrics.csv");
        transient.open(dir+"/P016_transient_metrics.csv");
        check(phase.good()&&transient.good(),"P016 CSV open");
        phase<<"rate_hz,effect,channel,frequency_hz,wet_gain_db,phase_deg,half_mix_gain_db,coherent_sum_loss_db\n";
        transient<<"rate_hz,effect,channel,first_peak_sample_offset,peak_20ms,energy_2ms,energy_20ms,energy_tail,compensated_prering_energy\n";
    }
    constexpr std::array<double,6> frequencies{100.,1000.,4000.,8000.,12000.,16000.};
    for(int sr:{44100,48000,96000}) {
        const int warm=sr/4;
        Stereo signal(static_cast<std::size_t>(warm+sr));
        for(std::size_t n=0;n<signal.size();++n) {
            double v=0.;
            for(double hz:frequencies)v+=.025*tone(hz,static_cast<double>(n)/sr);
            signal.l[n]=static_cast<float>(v);signal.r[n]=static_cast<float>(v);
        }
        Stereo pulse(static_cast<std::size_t>(sr));
        const int onset=sr/4;
        pulse.l[onset]=.25f;pulse.r[onset]=-.25f;
        for(int fx:{0,3,4,8}) {
            const Stereo wet=render(signal,fx,1.f,0.f,false,false,sr);
            const Stereo half=render(signal,fx,1.f,.5f,false,false,sr);
            const Stereo imp=render(pulse,fx,1.f,0.f,false,false,sr);
            const Stereo rawImpulse=render(pulse,fx,1.f,0.f,false,false,sr,false);
            for(int c=0;c<2;++c) {
                const auto& d=c?signal.r:signal.l;
                const auto& w=c?wet.r:wet.l;
                const auto& h=c?half.r:half.l;
                const auto& z=c?imp.r:imp.l;
                double maxMixError=0.;
                for(std::size_t n=0;n<d.size();++n) {
                    check(std::isfinite(w[n])&&std::isfinite(h[n]),"P016 nonfinite phase probe");
                    maxMixError=std::max(maxMixError,
                        std::abs(static_cast<double>(h[n])-.5*(d[n]+static_cast<double>(w[n]))));
                }
                check(maxMixError<5.e-6,"P016 actual THERAPY half mix mismatch");
                for(double hz:frequencies) {
                    const auto D=projection(d,hz,warm,sr,sr);
                    const auto W=projection(w,hz,warm,sr,sr);
                    const auto H=projection(h,hz,warm,sr,sr);
                    const double da=std::hypot(D[0],D[1]),wa=std::hypot(W[0],W[1]);
                    const double ha=std::hypot(H[0],H[1]);
                    const double gain=20.*std::log10(std::max(wa,1.e-15)/da);
                    const double angle=std::atan2(W[1]*D[0]-W[0]*D[1],
                                                  W[0]*D[0]+W[1]*D[1])*180./kPi;
                    const double halfGain=20.*std::log10(std::max(ha,1.e-15)/da);
                    const double loss=20.*std::log10(std::max(ha,1.e-15)/(.5*(da+wa)));
                    check(loss>=(fx==0?-3.5:-1.5),"P019 phase cancellation regression");
                    if(phase.good())phase<<sr<<","<<kMacroNames[fx]<<","<<c<<","<<hz<<","<<gain<<","<<angle<<","<<halfGain<<","<<loss<<"\n";
                    std::printf("P016 PHASE rate=%d fx=%s ch=%d hz=%.0f wet=%+.2f angle=%+.1f half=%+.2f cancellation=%+.2f dB\n",
                                sr,kMacroNames[fx],c,hz,gain,angle,halfGain,loss);
                }
                double peak=0.,early=0.,window=0.,tail=0.,prering=0.;
                const auto& raw=c?rawImpulse.r:rawImpulse.l;
                for(int n=0;n<onset;++n)check(raw[static_cast<std::size_t>(n)]==0.f,"P018 raw impulse response must be causal");
                int offset=0;
                const int n2=sr*2/1000,n20=sr*20/1000;
                for(int n=0;n<static_cast<int>(z.size());++n) {
                    const double v=z[static_cast<std::size_t>(n)];
                    check(std::isfinite(v),"P016 nonfinite impulse output");
                    if(n<onset) {
                        if(n<onset-kLatency)check(v==0.,"P018 pre-ring outside compensated FIR latency");
                        prering+=v*v;
                    }
                    else if(n<onset+n20) {
                        if(std::abs(v)>peak){peak=std::abs(v);offset=n-onset;}
                        window+=v*v;
                        if(n<onset+n2)early+=v*v;
                    } else tail+=v*v;
                }
                check(peak>1.e-5 && peak<1.,"P016 impulse missing or uncontrolled peak");
                if(transient.good())transient<<sr<<","<<kMacroNames[fx]<<","<<c<<","<<offset<<","<<peak<<","<<early<<","<<window<<","<<tail<<","<<prering<<"\n";
                std::printf("P016 IMPULSE rate=%d fx=%s ch=%d peak_offset=%d peak=%.6f energy2ms=%.6f energy20ms=%.6f tail=%.6f prering=%.6f\n",
                            sr,kMacroNames[fx],c,offset,peak,early,window,tail,prering);
            }
        }
    }
}
void testObsessionWetLoop() {
    for(int sr:{44100,48000,96000}) {
        const auto grain=static_cast<std::size_t>(sr*(.275-.257));
        Stereo input(grain*5);
        std::fill(input.l.begin(),input.l.end(),.3f);
        std::fill(input.r.begin(),input.r.end(),-.3f);
        Stereo changed=input;
        for(std::size_t n=grain;n<input.size();++n) {
            changed.l[n]=-.3f;changed.r[n]=.3f;
        }
        const auto a=render(input,6,1.f,0.f,false,false,sr);
        const auto b=render(changed,6,1.f,0.f,false,false,sr);
        check(identical(a,b),"P020 live input leaked into full wet frozen loop");
        check(*std::max_element(a.l.begin(),a.l.end())>.1f &&
              *std::min_element(a.l.begin(),a.l.end())>=0.f,
              "P020 captured loop not replaying");
    }
}
void testWetStartup() {
    for(int sr:{44100,48000,96000}) {
        Stereo x(64);
        std::fill(x.l.begin(),x.l.end(),.4f);
        std::fill(x.r.begin(),x.r.end(),-.4f);
        const auto y=render(x,2,1.f,0.f,false,false,sr);
        for(std::size_t n=0;n<x.size();++n) {
            check(std::abs(y.l[n]-1.10f*std::tanh(.33f*.4f))<1.e-6f,"P019 SCHIZOPHRENIA wet from first sample");
            check(std::abs(y.r[n]-1.10f*std::tanh(-.33f*.4f))<1.e-6f,"P019 SCHIZOPHRENIA stereo wet from first sample");
        }
    }
}
// P019: explicit raw host buffer, mono, instance isolation and restart checks.
void testHostIntegrity() {
    constexpr int total=48000;
    constexpr int maxBlock=1024;
    const auto src=source(1);
    const auto setup=[&](Processor& p,int channels,int rate,bool twelve,float amount,float therapy){
        check(p.initialize(nullptr)==kResultOk,"P019 host init");
        check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"activate audio buses");
        auto layout=channels==1?Vst::SpeakerArr::kMono:Vst::SpeakerArr::kStereo;
        check(p.setBusArrangements(&layout,1,&layout,1)==kResultTrue,"P019 bus arrangement");
        Vst::ProcessSetup st{Vst::kRealtime,Vst::kSample32,maxBlock,static_cast<double>(rate)};
        check(p.setupProcessing(st)==kResultOk,"P019 host setup");
        ResizableMemoryIBStream state;
        IBStreamer writer(&state,kLittleEndian);
        check(writer.writeInt32(kStateMagic)&&writer.writeInt32(twelve?kStateVersion:4),"P019 state header");
        for(int i=0;i<9;++i)check(writer.writeFloat(amount),"P019 macro");
        check(writer.writeFloat(therapy)&&writer.writeInt32(0),"P019 therapy");
        if(twelve)for(int i=0;i<3;++i)check(writer.writeFloat(amount),"P027b v5 macro");
        state.rewind();
        check(p.setState(&state)==kResultOk&&p.setActive(true)==kResultOk,"P019 host activate");
    };
    const auto block=[&](Processor& p,const Stereo& input,Stereo& output,int at,int count,int channels,bool inplace){
        auto* l=const_cast<float*>(input.l.data())+at;
        auto* r=const_cast<float*>(input.r.data())+at;
        if(inplace) {
            std::copy_n(l,count,output.l.data()+at);
            if(channels==2)std::copy_n(r,count,output.r.data()+at);
            l=output.l.data()+at;r=output.r.data()+at;
        }
        float* in[]{l,r};float* out[]{output.l.data()+at,output.r.data()+at};
        Vst::AudioBusBuffers bi{},bo{};
        bi.numChannels=channels;bo.numChannels=channels;bi.channelBuffers32=in;bo.channelBuffers32=out;
        Vst::ProcessData data{};data.processMode=Vst::kRealtime;data.symbolicSampleSize=Vst::kSample32;
        data.numSamples=count;data.numInputs=1;data.numOutputs=1;data.inputs=&bi;data.outputs=&bo;
        check(p.process(data)==kResultOk,"P019 actual host callback");
        for(int n=at;n<at+count;++n) {
            check(std::isfinite(output.l[n])&&std::abs(output.l[n])<4.f,"P019 host finite left");
            if(channels==2)check(std::isfinite(output.r[n])&&std::abs(output.r[n])<4.f,"P019 host finite right");
        }
    };
    for(bool twelve:{false,true})for(int rate:{44100,48000,96000})for(int channels:{1,2}) {
        Processor a,b,isolated;
        setup(a,channels,rate,twelve,.85f,.25f);setup(b,channels,rate,twelve,.85f,.25f);setup(isolated,channels,rate,twelve,0.f,1.f);
        Stereo outA(total),outB(total),outIso(total),other=source(4);
        int pos=0,step=0;
        constexpr std::array<int,6> sizes{17,127,1024,3,256,1537}; // also exercise chunking beyond setup maximum
        while(pos<total) {
            const int n=std::min(sizes[step++%sizes.size()],total-pos);
            block(a,src,outA,pos,n,channels,true);
            block(isolated,other,outIso,pos,n,channels,false);
            pos+=n;
        }
        for(int n=0;n<total;n+=251)block(b,src,outB,n,std::min(251,total-n),channels,false);
        check(outA.l==outB.l,"P019 in-place/variable-block/instance left equivalence");
        if(channels==2)check(outA.r==outB.r,"P019 in-place/variable-block/instance right equivalence");
        bool changed=false;
        for(int n=kLatency;n<total;++n)if(std::abs(outB.l[n]-src.l[n-kLatency])>1.e-4f)changed=true;
        check(changed,"P027b host exercise must produce actual wet change");
        for(int n=0;n<total;++n) {
            check(outIso.l[n]==(n>=kLatency?other.l[n-kLatency]:0.f),"P019 independent dry left");
            if(channels==2)check(outIso.r[n]==(n>=kLatency?other.r[n-kLatency]:0.f),"P019 independent dry right");
        }
        check(a.setActive(false)==kResultOk&&a.setActive(true)==kResultOk,"P019 restart");
        Stereo restart(total);
        for(int n=0;n<total;n+=511)block(a,src,restart,n,std::min(511,total-n),channels,false);
        check(restart.l==outB.l,"P019 deterministic restart left");
        if(channels==2)check(restart.r==outB.r,"P019 deterministic restart right");
        for(auto* p:{&a,&b,&isolated}) {
            check(p->setActive(false)==kResultOk,"P019 deactivate");
            check(p->terminate()==kResultOk,"P019 terminate");
        }
        std::printf("P019 HOST INTEGRITY PASS channels=%d rate=%d macros=%d, variable/oversized blocks, in-place, isolated instances, exact compensated dry, restart\n",channels,rate,twelve?12:9);
    }
}
void testExtendedDry() {
    Stereo x(8);
    x.l={0.f,1.e-30f,-1.e-30f,20.f,-20.f,.25f,-.25f,0.f};
    x.r=x.l;
    for(int fx=0;fx<12;++fx) {
        check(identical(x,render(x,fx,0.f)),"P016 finite headroom 0% exact Dry");
        check(identical(x,render(x,fx,1.f,1.f)),"P016 finite headroom THERAPY exact Dry");
    }
}
// P017: exact endpoint convergence after REAL VST3 automation, not
// simply setting the state before activation. Verify 0/100% after the
// intentional smoothing interval without resetting the processor.
void testAutomatedEndpoints() {
    for(int sr:{44100,48000,96000}) {
        for(int fx=0;fx<12;++fx) {
            for(bool global:{false,true}) {
                Processor p;
                check(p.initialize(nullptr)==kResultOk,"P017 initialize");
        check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"activate audio buses");
                Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,kBlock,static_cast<double>(sr)};
                check(p.setupProcessing(setup)==kResultOk,"P017 setup");
                ResizableMemoryIBStream state;writeState(state,fx,1.f,0.f,false);
                check(p.setState(&state)==kResultOk,"P017 state");
                check(p.setActive(true)==kResultOk,"P017 activate");
                const int automationAt=sr/3;
                const int settledAt=automationAt+sr/2;
                const int end=settledAt+sr/20;
                bool wetBefore=false;
                std::size_t exact=0;
                std::array<float,kBlock> a{},b{},x{},y{};
                psychiatry::detail::FixedDelay<Processor::kLatency> refL,refR;
                for(int start=0;start<end;start+=kBlock) {
                    const int frames=std::min(kBlock,end-start);
                    for(int n=0;n<frames;++n) {
                        const int t=start+n;
                        a[n]=.31f*tone(123.,static_cast<double>(t)/sr);
                        b[n]=.29f*tone(357.,static_cast<double>(t)/sr);
                        if(t>=settledAt && t%211==0){a[n]=1.e-30f;b[n]=-1.e-30f;}
                        if(t==settledAt+31){a[n]=20.f;b[n]=-20.f;}
                    }
                    Vst::ParameterChanges changes(1);
                    if(start<=automationAt && automationAt<start+frames) {
                        int32 index=0,point=0;
                        auto* q=changes.addParameterData(global?kTherapy:kMacroIds[fx],index);
                        check(q&&q->addPoint(automationAt-start,global?1.:0.,point)==kResultTrue,
                              "P017 endpoint automation point");
                    }
                    float* ins[]{a.data(),b.data()},*outs[]{x.data(),y.data()};
                    Vst::AudioBusBuffers bi{},bo{};
                    bi.numChannels=2;bo.numChannels=2;bi.channelBuffers32=ins;bo.channelBuffers32=outs;
                    Vst::ProcessData data{};
                    data.processMode=Vst::kRealtime;data.symbolicSampleSize=Vst::kSample32;
                    data.numSamples=frames;data.numInputs=1;data.numOutputs=1;
                    data.inputs=&bi;data.outputs=&bo;data.inputParameterChanges=&changes;
                    check(p.process(data)==kResultOk,"P017 process");
                    for(int n=0;n<frames;++n) {
                        const int t=start+n;
                        check(std::isfinite(x[n])&&std::isfinite(y[n]),"P017 nonfinite");
                        const float dryL=refL.process(a[n]),dryR=refR.process(b[n]);
                        if(t<automationAt&&(std::abs(x[n]-dryL)>1.e-4f||
                                            std::abs(y[n]-dryR)>1.e-4f))wetBefore=true;
                        if(t>=settledAt+kLatency) {
                            if(x[n]!=dryL||y[n]!=dryR)
                                std::fprintf(stderr,"P017 endpoint mismatch rate=%d fx=%s control=%s t=%d dry=%g wet=%g\n",
                                             sr,kMacroNames[fx],global?"THERAPY":"macro",t,dryL,x[n]);
                            check(x[n]==dryL&&y[n]==dryR,"P017 automated endpoint not exact latency-matched dry");
                            ++exact;
                        }
                    }
                }
                check(wetBefore&&exact>0,"P017 test did not exercise wet and dry");
                check(p.setActive(false)==kResultOk,"P017 deactivate");
                check(p.terminate()==kResultOk,"P017 terminate");
                std::printf("P017 AUTOMATION PASS rate=%d fx=%s control=%s exact_frames=%zu\n",
                            sr,kMacroNames[fx],global?"THERAPY":"macro",exact);
            }
        }
    }
}
// P023: an actual host receives a nonzero tail declaration, and the
// full serial chain is exercised until that advertised bound has elapsed.
void testReportedTail() {
    for(bool twelve:{false,true})for(int sr:{44100,48000,96000}) {
        Processor p;
        check(p.initialize(nullptr)==kResultOk,"P023 init");
        check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"activate audio buses");
        Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,1024,double(sr)};
        check(p.setupProcessing(setup)==kResultOk,"P023 setup");
        ResizableMemoryIBStream state;
        IBStreamer writer(&state,kLittleEndian);
        check(writer.writeInt32(kStateMagic)&&writer.writeInt32(twelve?kStateVersion:4),"P023 state");
        for(int i=0;i<9;++i)check(writer.writeFloat(1.f),"P023 macro");
        check(writer.writeFloat(0.f)&&writer.writeInt32(0),"P023 mix");
        if(twelve)for(int i=0;i<3;++i)check(writer.writeFloat(1.f),"P026k tail extra macro");
        state.rewind();
        check(p.setState(&state)==kResultOk&&p.setActive(true)==kResultOk,"P023 activate");
        const auto reported=p.getTailSamples();
        check(reported==30u*static_cast<unsigned>(sr)+kLatency,"P023 host tail declaration");
        std::array<float,1024> l{},r{},a{},b{};
        // Exact silence at reset; followed by two seconds of input and
        // the entire stated tail duration. Bin each second independently.
        const int startSignal=sr,endSignal=3*sr;
        const int total=endSignal+static_cast<int>(reported);
        std::array<double,31> peaks{},energy{};
        std::array<unsigned,31> samples{};
        double activePeak=0.;
        for(int pos=0;pos<total;pos+=1024) {
            const int count=std::min(1024,total-pos);
            for(int i=0;i<count;++i) {
                const int n=pos+i;
                const double t=double(n)/sr;
                l[i]=n>=startSignal&&n<endSignal?.37f*tone(73.4,t)+.19f*tone(1291.,t):0.f;
                r[i]=n>=startSignal&&n<endSignal?.31f*tone(97.,t)+.17f*tone(873.,t):0.f;
            }
            float* in[]{l.data(),r.data()},*out[]{a.data(),b.data()};
            Vst::AudioBusBuffers bi{},bo{};
            bi.numChannels=2;bo.numChannels=2;bi.channelBuffers32=in;bo.channelBuffers32=out;
            if(pos+count<=startSignal||pos>=endSignal)bi.silenceFlags=3;
            Vst::ProcessData data{};data.processMode=Vst::kRealtime;data.symbolicSampleSize=Vst::kSample32;
            data.numSamples=count;data.numInputs=1;data.numOutputs=1;data.inputs=&bi;data.outputs=&bo;
            check(p.process(data)==kResultOk,"P023 actual callback");
            for(int i=0;i<count;++i) {
                const int n=pos+i;
                const double peak=std::max(std::abs(double(a[i])),std::abs(double(b[i])));
                check(std::isfinite(a[i])&&std::isfinite(b[i])&&peak<4.,"P023 finite and bounded");
                if(n<startSignal)check(peak==0.,"P023 spontaneous audio during initial silence");
                else if(n<endSignal)activePeak=std::max(activePeak,peak);
                else {
                    const auto bin=static_cast<std::size_t>((n-endSignal)/sr);
                    peaks[bin]=std::max(peaks[bin],peak);
                    energy[bin]+=double(a[i])*a[i]+double(b[i])*b[i];
                    ++samples[bin];
                }
            }
        }
        std::fprintf(stderr,"P023 excitation rate=%d macros=%d active_peak=%.9g first_tail_peak=%.9g\n",sr,twelve?12:9,activePeak,peaks[0]);
        check(activePeak>.01&&peaks[0]>.001,"P023 meaningful source and tail");
        for(std::size_t i=0;i<peaks.size();++i)
            std::printf("P023 TAIL rate=%d second=%zu peak=%.9g rms=%.9g\n",sr,i+1,peaks[i],
                        std::sqrt(energy[i]/(2.*std::max(1u,samples[i]))));
        // Retain measured tail rather than arbitrarily zeroing buffers.
        // The final complete second and extra latency must be below -100 dBFS.
        check(peaks[29]<=1.e-5&&peaks[30]<=1.e-5,"P023 declared tail too short or residual feedback");
        check(p.setActive(false)==kResultOk&&p.terminate()==kResultOk,"P023 terminate");
        std::printf("P023 TAIL PASS rate=%d macros=%d reported=%u\n",sr,twelve?12:9,reported);
    }
}

// P021: identical absolute automation timeline, radically different host
// block boundaries. Exercise all nine effects, native bypass, long silence,
// returning input and exact global Dry at the end, at three host rates.
void testDenseAutomationStress() {
    struct Event {int sample;Vst::ParamID id;double value;};
    for(int sr:{44100,48000,96000}) {
        const int total=7*sr;
        Stereo input(total);
        std::uint32_t rng=0x125A3121u;
        const auto random=[&]() {
            rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
            return double(rng&65535u)/65535.;
        };
        std::vector<Event> events;
        for(int n=0;n<3*sr;n+=113) {
            for(auto id:kMacroIds)events.push_back({n,id,random()});
            events.push_back({n,kTherapy,random()*.8});
        }
        for(auto id:kMacroIds)events.push_back({3*sr,id,1.});
        events.push_back({3*sr,kTherapy,0.});
        events.push_back({sr/2,kBypass,1.});
        events.push_back({sr,kBypass,0.});
        events.push_back({6*sr,kTherapy,1.});
        std::stable_sort(events.begin(),events.end(),[](const Event& a,const Event& b){return a.sample<b.sample;});
        // Explicit previous-sample anchors encode actual steps, not ramps.
        std::vector<Event> anchored;
        std::array<double,1000> last{};
        for(const auto& event:events){
            if(event.id!=kBypass&&event.sample>0)anchored.push_back({event.sample-1,event.id,last[event.id]});
            anchored.push_back(event);last[event.id]=event.value;
        }
        std::stable_sort(anchored.begin(),anchored.end(),[](const Event& a,const Event& b){return a.sample<b.sample;});
        events.swap(anchored);
        for(int n=0;n<total;++n) {
            if(n>=3*sr&&n<6*sr)continue; // listen for decay, not new excitation
            const double t=double(n)/sr;
            const float pulse=(n%(sr/4)<sr/100)?0.15f:0.f;
            input.l[n]=.22f*tone(73.4,t)+.11f*tone(1241.,t)+pulse*float(2.*random()-1.);
            input.r[n]=.19f*tone(97.,t)+.10f*tone(733.,t)-pulse*float(2.*random()-1.);
        }
        const auto run=[&](bool irregular) {
            Processor p;
            check(p.initialize(nullptr)==kResultOk,"P021 initialize");
        check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"activate audio buses");
            Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,1024,double(sr)};
            check(p.setupProcessing(setup)==kResultOk,"P021 setup");
            ResizableMemoryIBStream state;writeState(state,-1,0.f,0.f,false);
            check(p.setState(&state)==kResultOk&&p.setActive(true)==kResultOk,"P021 activate");
            Stereo out(total);
            std::size_t e=0;
            int start=0,step=0;
            constexpr std::array<int,7> sizes{3,17,127,1024,61,509,256};
            double peak=0.,tailEnergy=0.;std::size_t exact=0;
            while(start<total) {
                const int count=std::min(total-start,irregular?sizes[step++%sizes.size()]:257);
                Vst::ParameterChanges changes(14);
                while(e<events.size()&&events[e].sample<start+count) {
                    const auto& v=events[e++];
                    check(v.sample>=start,"P021 event timeline");
                    int32 index=0,point=0;
                    auto* q=changes.addParameterData(v.id,index);
                    check(q&&q->addPoint(v.sample-start,v.value,point)==kResultTrue,"P021 queue point");
                }
                float* ins[]{input.l.data()+start,input.r.data()+start};
                float* outs[]{out.l.data()+start,out.r.data()+start};
                Vst::AudioBusBuffers bi{},bo{};
                bi.numChannels=2;bo.numChannels=2;bi.channelBuffers32=ins;bo.channelBuffers32=outs;
                Vst::ProcessData data{};data.processMode=Vst::kRealtime;data.symbolicSampleSize=Vst::kSample32;
                data.numSamples=count;data.numInputs=1;data.numOutputs=1;data.inputs=&bi;data.outputs=&bo;
                data.inputParameterChanges=&changes;
                check(p.process(data)==kResultOk,"P021 actual callback");
                for(int n=start;n<start+count;++n) {
                    const float l=out.l[n],r=out.r[n];
                    check(std::isfinite(l)&&std::isfinite(r)&&std::abs(l)<4.f&&std::abs(r)<4.f,
                          "P021 nonfinite or uncontrolled peak");
                    peak=std::max({peak,std::abs(double(l)),std::abs(double(r))});
                    if(n>=3*sr+kLatency&&n<6*sr)tailEnergy+=double(l)*l+double(r)*r;
                    if((n>=sr/2+kLatency+static_cast<int>(std::ceil(.005*sr))+2&&n<sr+kLatency)||n>=6*sr+sr/2+kLatency) {
                        check(l==input.l[n-kLatency]&&r==input.r[n-kLatency],"P021 exact latency-matched bypass or therapy");
                        ++exact;
                    }
                }
                start+=count;
            }
            check(e==events.size()&&exact>0&&tailEnergy>0.,"P021 stress coverage");
            check(p.setActive(false)==kResultOk&&p.terminate()==kResultOk,"P021 terminate");
            std::printf("P021 STRESS rate=%d irregular=%d events=%zu frames=%d exact=%zu peak=%.6f tailEnergy=%.6f\n",
                        sr,irregular?1:0,events.size(),total,exact,peak,tailEnergy);
            return out;
        };
        const auto a=run(false),b=run(true);
        check(identical(a,b),"P021 absolute automation depends on host buffer size");
        std::printf("P021 STRESS PASS rate=%d all-twelve dense automation and silence\n",sr);
    }
}
// Prefix differences include interaction; not intrinsic single-stage gains.
void auditCombinations(const Stereo& dry,const char* sourceName,
                       const std::string& dir,std::ofstream& csv){
    constexpr std::array<int,12> order{0,2,3,4,1,5,6,7,8,9,10,11};
    using Amounts=std::array<float,12>;
    const auto run=[&](const Amounts& amounts,float therapy=0.f,bool bypass=false){
        return render(dry,-1,0.f,therapy,bypass,false,kRate,true,&amounts);
    };
    const auto record=[&](const char* type,int index,const Amounts& amounts,
                          const Stereo& wet,double prior){
        const auto m=measure(dry,wet);
        check(m.peak<4.&&m.wetRms>1.e-7,"P027c uncontrolled peak or effectively silent combination");
        csv<<sourceName<<','<<type<<','<<index;
        for(float value:amounts)csv<<','<<value;
        csv<<','<<m.changeDb<<','<<m.fullScaleDb<<','<<m.shapeResidual<<','<<m.changeDb-prior<<'\n';
        std::printf("P027c %s %s %02d gain=%+.3f dB peak=%+.3f dBFS step=%+.3f dB\n",
            sourceName,type,index,m.changeDb,m.fullScaleDb,m.changeDb-prior);
        return m.changeDb;
    };
    Amounts amounts{};
    double prior=0.;
    for(int j=0;j<12;++j){
        amounts[order[j]]=.5f;
        const auto wet=run(amounts);
        prior=record("prefix50",j+1,amounts,wet,prior);
        if(j==1){
            // Actual pinned P027c prefix 2 values, unchanged input and
            // upstream DEPRESSION. Midpoint correction should recover
            // ~1.938 dB on every source. Do not permit silently reverting.
            double baseline=0.;
            const std::string name=sourceName;
            if(name=="recorded_drums")baseline=-6.528;
            else if(name=="recorded_guitar")baseline=-2.314;
            else if(name=="recorded_vocal_foley")baseline=-4.465;
            else if(name=="recorded_machine_room")baseline=-1.332;
            else check(false,"P027d unknown pinned baseline");
            check(prior-baseline>1.85&&prior-baseline<2.03,"P027d actual schizophrenia midpoint recovery");
        }
        if(j==11&&measure(dry,wet).peak<=1.)
            writeWav(dir+"/P027c_"+sourceName+"_all50_RAW.wav",wet);
    }
    amounts.fill(.85f);
    record("all85",0,amounts,run(amounts),0.);
    constexpr std::array<Amounts,4> combinations{{
        {{.24f,0.f,.72f,.35f,.16f,.48f,.66f,.12f,.53f,.41f,.28f,.75f}},
        {{0.f,.56f,.23f,0.f,.74f,.62f,.19f,0.f,.43f,.67f,.38f,.55f}},
        {{.76f,.14f,.51f,.61f,.25f,.20f,0.f,.73f,.32f,0.f,.60f,.42f}},
        {{.32f,.78f,0.f,.48f,.55f,.33f,.12f,.62f,.70f,.17f,.53f,0.f}}
    }};
    for(int j=0;j<4;++j){
        const auto wet=run(combinations[j]);
        record("mixed",j+1,combinations[j],wet,0.);
        if(j==0){
            check(identical(dry,run(combinations[j],1.f)),"P027c combined THERAPY exact dry");
            check(identical(dry,run(combinations[j],0.f,true)),"P027c combined bypass exact dry");
            const auto half=run(combinations[j],.5f);
            double worst=0.;
            for(std::size_t i=0;i<dry.size();++i){
                worst=std::max(worst,std::abs(double(half.l[i])-(double(dry.l[i])+double(wet.l[i]))*.5));
                worst=std::max(worst,std::abs(double(half.r[i])-(double(dry.r[i])+double(wet.r[i]))*.5));
            }
            check(worst<=5.e-6,"P027c combined THERAPY midpoint");
            if(measure(dry,wet).peak<=1.)writeWav(dir+"/P027c_"+sourceName+"_mixed1_RAW.wav",wet);
        }
    }
}

// P027e: render the same generator used by the actual controller with a
// fixed test seed. Values are captured in the CSV; stdlib RNG mapping may
// differ across platforms, so no portable bitwise preset claim is made.
void auditRandomizer(const Stereo& dry,const char* sourceName,
                     const std::string& dir,std::ofstream& csv){
    std::mt19937 rng(0x125A27E);
    for(int j=0;j<8;++j){
        const auto preset=randomLossOfControl(rng);
        std::array<float,12> amounts{};
        unsigned active=0;
        for(std::size_t i=0;i<amounts.size();++i){
            amounts[i]=static_cast<float>(preset[i]);
            if(amounts[i]>0.f)++active;
        }
        check(active>=3&&active<=6,"P027e randomizer density");
        const auto wet=render(dry,-1,0.f,0.f,false,false,kRate,true,&amounts);
        const auto m=measure(dry,wet);
        check(m.peak<4.&&m.wetRms>1.e-7,"P027e randomizer finite bounded nonzero");
        csv<<sourceName<<','<<j+1<<','<<active;
        for(float v:amounts)csv<<','<<v;
        csv<<','<<m.changeDb<<','<<m.fullScaleDb<<','<<m.shapeResidual<<'\n';
        std::printf("P027e %s preset=%d active=%u gain=%+.3f peak=%+.3f residual=%.5f\n",
            sourceName,j+1,active,m.changeDb,m.fullScaleDb,m.shapeResidual);
        if(j<2&&m.peak<=1.)writeWav(dir+"/P027e_"+sourceName+"_preset"+std::to_string(j+1)+"_RAW.wav",wet);
    }
}

// P025: reference recordings decoded into fixed 48 kHz float stereo
// outside the plug-in. Keep raw measurements separate from audition gain.
void auditRecordings(const std::string& inputDir,const std::string& outputDir) {
    std::ofstream csv(outputDir+"/P025_recorded_metrics.csv");
    check(csv.good(),"P025 CSV");
    std::ofstream combo(outputDir+"/P027c_combination_metrics.csv");
    check(combo.good(),"P027c CSV");
    std::ofstream randomizer(outputDir+"/P027e_randomizer_metrics.csv");
    check(randomizer.good(),"P027e CSV");
    randomizer<<"source,preset,active_count";
    for(auto name:kMacroNames)randomizer<<','<<name;
    randomizer<<",gain_db,peak_dbfs,gain_matched_residual\n";
    combo<<"source,case,index";
    for(auto name:kMacroNames)combo<<','<<name;
    combo<<",gain_db,peak_dbfs,gain_matched_residual,delta_vs_previous_db\n";
    csv<<"source,macro,amount,gain_db,peak_dbfs,raw_null_ratio,gain_matched_residual\n";
    constexpr std::array<const char*,4> names{"recorded_drums","recorded_guitar","recorded_vocal_foley","recorded_machine_room"};
    for(const char* name:names) {
        Stereo dry(6*kRate);
        std::ifstream in(inputDir+"/"+name+".f32",std::ios::binary);
        check(in.good(),"P025 recording missing");
        for(std::size_t i=0;i<dry.size();++i){
            in.read(reinterpret_cast<char*>(&dry.l[i]),sizeof(float));
            in.read(reinterpret_cast<char*>(&dry.r[i]),sizeof(float));
        }
        check(in.good()&&in.peek()==std::char_traits<char>::eof(),"P025 recording length");
        check(identical(dry,render(dry,-1,0.f)),"P025 exact neutral");
        writeWav(outputDir+"/P025_"+name+"_DRY.wav",dry);
        auditCombinations(dry,name,outputDir,combo);
        auditRandomizer(dry,name,outputDir,randomizer);
        for(int fx=0;fx<12;++fx) {
            for(float amount:{.05f,.10f,.20f,.50f,1.f}){
                const auto wet=render(dry,fx,amount);
                const auto m=measure(dry,wet);
                check(m.peak<4.&&m.wetRms>1.e-7,"P025 uncontrolled peak or silent effect");
                csv<<name<<','<<kMacroNames[fx]<<','<<amount<<','<<m.changeDb<<','<<m.fullScaleDb<<','<<m.nullRatio<<','<<m.shapeResidual<<'\n';
                std::printf("P025 %s %s %.2f gain=%+.3f peak=%.3f residual=%.5f\n",
                    name,kMacroNames[fx],amount,m.changeDb,m.fullScaleDb,m.shapeResidual);
                if(amount==.5f||amount==1.f){
                    // Least arbitrary loudness comparison: RMS match, then
                    // one shared headroom attenuation for both halves.
                    const double gain=m.dryRms/std::max(1.e-12,m.wetRms);
                    double peak=0.;
                    for(std::size_t i=0;i<dry.size();++i)
                        peak=std::max({peak,std::abs(double(dry.l[i])),std::abs(double(dry.r[i])),
                            gain*std::abs(double(wet.l[i])),gain*std::abs(double(wet.r[i]))});
                    const double scale=std::min(1.,.90/std::max(1.e-12,peak));
                    const auto gap=static_cast<std::size_t>(kRate/2);
                    Stereo pair(dry.size()*2+gap);
                    for(std::size_t i=0;i<dry.size();++i){
                        pair.l[i]=float(scale*dry.l[i]);pair.r[i]=float(scale*dry.r[i]);
                        pair.l[i+dry.size()+gap]=float(scale*gain*wet.l[i]);
                        pair.r[i+dry.size()+gap]=float(scale*gain*wet.r[i]);
                    }
                    writeWav(outputDir+"/P025_"+name+"_"+kMacroNames[fx]+(amount==1.f?"_100_AB.wav":"_50_AB.wav"),pair);
                }
            }
        }
    }
    check(csv.good(),"P025 metric write");
    check(combo.good(),"P027c metric write");
    check(randomizer.good(),"P027e metric write");
    std::puts("P027e randomizer audit PASS: 32 recorded presets");
    std::puts("P027c COMBINATION AUDIT PASS: 68 recorded cases; exact combined THERAPY and bypass; gain flags for review");
    std::puts("P025 RECORDED AUDIO EXPORT PASS: 240 objective rows, 96 RMS-matched A/B files; human audition not claimed");
}
} // namespace
static void testAuditAudio(){
    constexpr int total=8192,block=256,at=4096;
    Stereo sourceAudio(total);
    for(int n=0;n<total;++n){
        sourceAudio.l[n]=.3f*tone(237.,double(n)/kRate);
        sourceAudio.r[n]=.27f*tone(411.,double(n)/kRate);
    }
    auto run=[&](int mode){
        Processor p;
        check(p.initialize(nullptr)==kResultOk,"audit initialize");
        check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"audit activate buses");
        Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,block,kRate};
        check(p.setupProcessing(setup)==kResultOk,"audit setup");
        ResizableMemoryIBStream state;writeState(state,8,1.f,0.f,false);
        check(p.setState(&state)==kResultOk&&p.setActive(true)==kResultOk&&p.setProcessing(true)==kResultOk,"audit activate");
        Stereo output(total);
        for(int start=0;start<total;){
            const int n=std::min(mode==2?64:block,total-start);
            Vst::ParameterChanges changes(1);
            if(start>=at&&start<at+block&&mode<3){
                int32 index=0,point=0;auto* q=changes.addParameterData(kTherapy,index);
                if(mode==1){
                    for(int i=0;i<n;++i)check(q->addPoint(i,double(start-at+i+1)/block,point)==kResultOk,"dense linear point");
                }else check(q->addPoint(n-1,double(start-at+n)/block,point)==kResultOk,"sparse linear endpoint");
            }
            if((mode==5||mode==6)&&(start==at||start==at+1024)){
                ResizableMemoryIBStream recall;
                writeState(recall,8,1.f,0.f,start==at);
                check(p.setState(&recall)==kResultOk,"R03 active recall");
                if(mode==6){
                    Vst::ProcessData flush{};flush.symbolicSampleSize=Vst::kSample32;
                    check(p.process(flush)==kResultOk,"R03 active zero-frame recall");
                }
            }
            if(mode==4&&start==at){
                int32 index=0,point=0;auto* q=changes.addParameterData(kBypass,index);
                check(q->addPoint(0,1.,point)==kResultOk,"bypass on");
            }
            if(mode==4&&start==at+1024){
                int32 index=0,point=0;auto* q=changes.addParameterData(kBypass,index);
                check(q->addPoint(0,0.,point)==kResultOk,"bypass off");
            }
            float* ins[]{sourceAudio.l.data()+start,sourceAudio.r.data()+start};
            float* outs[]{output.l.data()+start,output.r.data()+start};
            Vst::AudioBusBuffers bi{},bo{};bi.numChannels=bo.numChannels=2;
            bi.channelBuffers32=ins;bo.channelBuffers32=outs;
            Vst::ProcessData data{};data.symbolicSampleSize=Vst::kSample32;data.numSamples=n;
            data.numInputs=data.numOutputs=1;data.inputs=&bi;data.outputs=&bo;data.inputParameterChanges=&changes;
            check(p.process(data)==kResultOk,"audit render");start+=n;
        }
        check(p.setProcessing(false)==kResultOk&&p.setActive(false)==kResultOk&&p.terminate()==kResultOk,"audit terminate");
        return output;
    };
    const auto sparse=run(0),dense=run(1),partition=run(2),wet=run(3),bypass=run(4);
    const auto recall=run(5),flushRecall=run(6);
    double difference=0.,change=0.,fadeError=0.,recallError=0.;
    constexpr int ramp=240;
    for(int n=0;n<total;++n){
        difference=std::max({difference,std::abs(double(sparse.l[n])-dense.l[n]),
            std::abs(double(sparse.r[n])-dense.r[n]),std::abs(double(sparse.l[n])-partition.l[n]),
            std::abs(double(sparse.r[n])-partition.r[n])});
        change+=std::abs(double(wet.l[n])-sparse.l[n]);
        const float dl=n>=kLatency?sourceAudio.l[n-kLatency]:0.f;
        const float dr=n>=kLatency?sourceAudio.r[n-kLatency]:0.f;
        const int t=n-at-kLatency;
        const double dryMix=t<0?0.:t<ramp?double(t+1)/ramp:t<1024?1.:t<1024+ramp?1.-double(t-1024+1)/ramp:0.;
        fadeError=std::max({fadeError,std::abs(double(bypass.l[n])-(dl+(1.-dryMix)*(wet.l[n]-dl))),
            std::abs(double(bypass.r[n])-(dr+(1.-dryMix)*(wet.r[n]-dr)))});
        if(dryMix==1.)check(bypass.l[n]==dl&&bypass.r[n]==dr,"bypass exact dry plateau");
        // Recall applies at the next output frame, not as a host input event.
        const int rt=n-at;
        const double rm=rt<0?0.:rt<ramp?double(rt+1)/ramp:rt<1024?1.:rt<1024+ramp?1.-double(rt-1024+1)/ramp:0.;
        for(const auto* rendered:{&recall,&flushRecall}){
            recallError=std::max({recallError,std::abs(double(rendered->l[n])-(dl+(1.-rm)*(wet.l[n]-dl))),
                std::abs(double(rendered->r[n])-(dr+(1.-rm)*(wet.r[n]-dr)))});
            if(rm==1.)check(rendered->l[n]==dl&&rendered->r[n]==dr,"R03 recalled bypass exact dry plateau");
        }
    }
    std::printf("P028k A02/A07 sparse-dense-partition max %.9g, active delta %.9g, bypass oracle %.9g\n",difference,change,fadeError);
    check(difference<2.e-6&&change>1.,"equivalent VST3 linear automation must be identical within numeric precision");
    check(fadeError<3.e-6,"sample-aligned 5ms bypass crossfade");
    std::printf("P028l R03 active recall/zero-frame recall oracle %.9g\n",recallError);
    check(recallError<3.e-6,"R03 recall must ramp from current mix, including zero-frame recall");
    // Fresh neutral, therapy-dry and bypass modes: malformed PCM becomes zero.
    for(int mode=0;mode<3;++mode){
        Stereo malformed(512),clean(512);
        for(int n=0;n<512;++n){clean.l[n]=malformed.l[n]=.125f;clean.r[n]=malformed.r[n]=-.25f;}
        malformed.l[0]=std::numeric_limits<float>::quiet_NaN();
        malformed.r[1]=std::numeric_limits<float>::infinity();
        clean.l[0]=0.f;clean.r[1]=0.f;
        auto actual=render(malformed,mode?8:-1,mode?1.f:0.f,mode==1?1.f:0.f,mode==2);
        check(identical(actual,clean),"A06 nonfinite input escaped dry boundary");
    }
    Processor p;
    check(p.initialize(nullptr)==kResultOk,"bus init");
    Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,block,kRate};
    check(p.setupProcessing(setup)==kResultOk,"bus setup");
    check(p.activateBus(Vst::kAudio,Vst::kInput,0,false)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,false)==kResultOk,"deactivate buses");
    check(p.setActive(true)==kResultOk&&p.setProcessing(true)==kResultOk,"bus start");
    std::array<float,block> l{},r{},outL{},outR{};l.fill(.75f);r.fill(-.75f);outL.fill(.123f);outR.fill(-.321f);
    float* ins[]{l.data(),r.data()},*outs[]{outL.data(),outR.data()};
    Vst::AudioBusBuffers bi{},bo{};bi.numChannels=bo.numChannels=2;bi.channelBuffers32=ins;bo.channelBuffers32=outs;bo.silenceFlags=3;
    Vst::ProcessData d{};d.symbolicSampleSize=Vst::kSample32;d.numSamples=block;d.numInputs=d.numOutputs=1;d.inputs=&bi;d.outputs=&bo;
    check(p.process(d)==kResultOk&&bo.silenceFlags==3,"inactive output flags");
    for(int n=0;n<block;++n)check(outL[n]==.123f&&outR[n]==-.321f,"inactive output overwritten");
    check(p.setProcessing(false)==kResultOk&&p.setActive(false)==kResultOk,"stop before bus activation");
    check(p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk&&p.setActive(true)==kResultOk&&p.setProcessing(true)==kResultOk,"activate output only");
    check(p.process(d)==kResultOk&&bo.silenceFlags==3,"inactive input nonnull: output silent");
    for(int n=0;n<block;++n)check(outL[n]==0.f&&outR[n]==0.f,"inactive input sentinel read");
    check(p.setProcessing(false)==kResultOk&&p.setActive(false)==kResultOk,"stop before input activation");
    check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.setActive(true)==kResultOk&&p.setProcessing(true)==kResultOk,"reactivate input");
    l.fill(0.f);r.fill(0.f);l[0]=.5f;
    Vst::ParameterChanges split(1);int32 qi=0,pi=0;
    auto* q=split.addParameterData(kTherapy,qi);
    check(q&&q->addPoint(192,0.,pi)==kResultOk,"silent flag span split");
    d.inputParameterChanges=&split;
    check(p.process(d)==kResultOk&&bo.silenceFlags==2,"left impulse before later silent span must retain active flag");
    check(outL[kLatency]==.5f&&outR[kLatency]==0.f,"reactivated input actual PCM");
    l.fill(0.f);d.inputParameterChanges=nullptr;
    check(p.process(d)==kResultOk&&bo.silenceFlags==3,"fully silent next block");
    check(p.setProcessing(false)==kResultOk&&p.setActive(false)==kResultOk&&p.terminate()==kResultOk,"bus terminate");
    std::puts("P028k A05/A06 inactive buses and malformed PCM PASS");
}

// Invalid host queues bypass the SDK example's sorting/deduplication to reproduce
// late-invalid offsets and failed reads exactly as supplied by the host.
class AuditMalformedQueue final:public Vst::ParameterValueQueue {
public:
    explicit AuditMalformedQueue(int mode):Vst::ParameterValueQueue(kTherapy),mode_(mode){}
    int32 PLUGIN_API getPointCount() override{return 2;}
    tresult PLUGIN_API getPoint(int32 index,int32& offset,Vst::ParamValue& value) override{
        if(index==0){offset=8;value=.75;return kResultOk;}
        if(index!=1||mode_==3)return kResultFalse;
        offset=mode_==0?64:mode_==1?4:8;value=.9;
        return kResultOk;
    }
private:int mode_;
};
class AuditMalformedChanges final:public Vst::ParameterChanges{
public:
    explicit AuditMalformedChanges(int mode):Vst::ParameterChanges(0),queue_(mode){}
    int32 PLUGIN_API getParameterCount() override{return 1;}
    Vst::IParamValueQueue* PLUGIN_API getParameterData(int32 i) override{return i==0?&queue_:nullptr;}
private:AuditMalformedQueue queue_;
};
static void testRejectedQueueTransaction(){
    constexpr int block=64,total=1024;
    auto run=[](int mode){
        Processor p;
        check(p.initialize(nullptr)==kResultOk,"R02 init");
        check(p.activateBus(Vst::kAudio,Vst::kInput,0,true)==kResultOk&&p.activateBus(Vst::kAudio,Vst::kOutput,0,true)==kResultOk,"R02 buses");
        Vst::ProcessSetup setup{Vst::kRealtime,Vst::kSample32,block,48000.};
        check(p.setupProcessing(setup)==kResultOk,"R02 setup");
        ResizableMemoryIBStream state;writeState(state,8,.8f,.125f,false);
        check(p.setState(&state)==kResultOk&&p.setActive(true)==kResultOk&&p.setProcessing(true)==kResultOk,"R02 active");
        auto verifyTherapy=[&](){
            ResizableMemoryIBStream saved;
            check(p.getState(&saved)==kResultOk,"R02 saved state");saved.rewind();
            IBStreamer r(&saved,kLittleEndian);int32 magic=0,version=0;float v=0;
            check(r.readInt32(magic)&&r.readInt32(version)&&magic==kStateMagic&&version==kStateVersion,"R02 saved header");
            for(int i=0;i<10;++i)check(r.readFloat(v),"R02 saved float");
            check(v==.125f,"R02 malformed queue changed saved parameter");
        };
        Stereo result(total);
        std::array<float,block> l{},r{},ol{},orr{};
        float* ins[]{l.data(),r.data()},*outs[]{ol.data(),orr.data()};
        Vst::AudioBusBuffers bi{},bo{};bi.numChannels=bo.numChannels=2;
        bi.channelBuffers32=ins;bo.channelBuffers32=outs;
        Vst::ProcessData d{};d.symbolicSampleSize=Vst::kSample32;d.numSamples=block;
        d.numInputs=d.numOutputs=1;d.inputs=&bi;d.outputs=&bo;
        for(int start=0;start<total;start+=block){
            if(start==256){
                verifyTherapy();
                if(mode>=0){
                    AuditMalformedChanges bad(mode);
                    l.fill(.4f);r.fill(-.3f);ol.fill(.123f);orr.fill(-.321f);bo.silenceFlags=3;
                    d.inputParameterChanges=&bad;
                    check(p.process(d)==kResultFalse,"R02 malformed late queue accepted");
                    d.inputParameterChanges=nullptr;
                    check(bo.silenceFlags==3,"R02 rejected queue changed silence flags");
                    for(int i=0;i<block;++i)check(ol[i]==.123f&&orr[i]==-.321f,"R02 rejected queue wrote output");
                }
                Vst::ProcessData flush{};flush.symbolicSampleSize=Vst::kSample32;
                check(p.process(flush)==kResultOk,"R02 empty flush");
                verifyTherapy();
            }
            for(int i=0;i<block;++i){
                l[i]=(start+i==256?.5f:0.f)+.2f*tone(237.,double(start+i)/48000.);
                r[i]=.17f*tone(411.,double(start+i)/48000.);
            }
            check(p.process(d)==kResultOk,"R02 clean render");
            std::copy(ol.begin(),ol.end(),result.l.begin()+start);
            std::copy(orr.begin(),orr.end(),result.r.begin()+start);
        }
        verifyTherapy();
        check(p.setProcessing(false)==kResultOk&&p.setActive(false)==kResultOk&&p.terminate()==kResultOk,"R02 terminate");
        return result;
    };
    const auto baseline=run(-1);
    for(int mode=0;mode<4;++mode)check(identical(baseline,run(mode)),"R02 rejected queue advanced DSP or audio timeline");
    std::puts("P028l R02 malformed late queue transaction PASS");
}

int main(int argc,char** argv){
    if(argc==4&&std::string(argv[1])=="--recordings"){
        auditRecordings(argv[2],argv[3]);return 0;
    }
    std::ofstream csv;
    const bool exportWav=argc>1;
    if(exportWav){
        const std::string path=argv[1];
        csv.open(path+"/P010_actual_vst3_audio_metrics.csv");
        check(csv.good(),"create CSV");
        csv<<"source,macro,amount,therapy,dry_rms,output_rms,gain_db,peak_dbfs,raw_null_ratio,gain_matched_residual,band_shape_distance\n";
    }
    testAuditAudio();
    testRejectedQueueTransaction();
    testAutomation();
    testHighBandAliasing(exportWav?std::string(argv[1]):std::string{});
    testMultirateIntermod(exportWav?std::string(argv[1]):std::string{});
    testExtendedDry();
    testWetStartup();
    testObsessionWetLoop();
    testHostIntegrity();
    testAutomatedEndpoints();
    testDenseAutomationStress();
    testReportedTail();
    testPhaseAndTransients(exportWav?std::string(argv[1]):std::string{});
    int substantial=0,weak=0;
    for(unsigned input=0;input<kInputs.size();++input){
        const Stereo dry=source(input);
        rejectGainPlacebo(dry);
        const Stereo allZero=render(dry,-1,0.f,0.f);
        check(identical(dry,allZero),"all zero macros + THERAPY 0 not bit-exact dry");
        const Stereo bypass=render(dry,3,1.f,0.f,true);
        check(identical(dry,bypass),"native Bypass must be bit-exact dry");
        if(input==0 && exportWav)writeWav(std::string(argv[1])+"/P010_DRY_Bass.wav",dry);
        for(int effect=0;effect<12;++effect){
            const Stereo zero=render(dry,effect,0.f);
            check(identical(dry,zero),"individual 0% is not bit-exact dry");
            for(const float amount:{.20f,.50f,.75f,1.f}){
                const Stereo wet=render(dry,effect,amount,0.f);
                const Metric m=measure(dry,wet);
                const double bandDistance=spectralBandDistance(dry,wet);
                if(amount==1.f){
                    const Stereo expected=pureWetReference(dry,effect);
                    double maxReferenceError=0.;
                    for(std::size_t k=0;k<dry.size();++k){
                        maxReferenceError=std::max(maxReferenceError,
                            std::abs(static_cast<double>(wet.l[k])-expected.l[k]));
                        maxReferenceError=std::max(maxReferenceError,
                            std::abs(static_cast<double>(wet.r[k])-expected.r[k]));
                    }
                    if(maxReferenceError>3.e-6)
                        std::fprintf(stderr,
                            "P010 WET ENDPOINT FAIL %s %s: VST3 vs wet-only max error %.9g\n",
                            kInputs[input],kMacroNames[effect],maxReferenceError);
                    check(maxReferenceError<=3.e-6,
                        "100% macro output contains extra dry/mixing or mismatched wet DSP");
                }
                if(input==0 && amount==1.f && exportWav)
                    writeWav(std::string(argv[1])+"/P010_"+std::string(kMacroNames[effect])+"_WET100.wav",wet);
                if(amount==1.f) {
                    // P011 guard: a distinctive 100%-wet effect is not an
                    // excuse for massive unintentional gain swings.
                    // This is deliberately per source and per macro.
                    // Keep the existing nine-world limits; deliberate spectral hold and
                    // erasure have a separate conservative minimum-energy gate.
                    if(m.changeDb<(effect<9?-8.0:-24.0) || m.changeDb>6.0) {
                        std::fprintf(stderr,
                            "P011 FULL-WET LEVEL REGRESSION %s %s gain=%+.3f dB\n",
                            kInputs[input],kMacroNames[effect],m.changeDb);
                        check(false,"P011/P026k 100% effect gain outside declared per-world audit bounds");
                    }
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
                    <<m.fullScaleDb<<","<<m.nullRatio<<","<<m.shapeResidual<<","<<bandDistance<<"\n";
                if(amount==1.f)
                    std::printf("P010 full wet: %-13s %-16s gain=%+7.2f dB rawNull=%6.3f gainMatchedResidual=%6.3f\n",
                        kInputs[input],kMacroNames[effect],
                        m.changeDb,m.nullRatio,m.shapeResidual);
            }
            if(input==0)checkTherapy(dry,effect);
        }
    }
    std::printf("P010 VST3 audio processor endpoint invariants PASS; substantive full-wet cases=%d/60, WEAK=%d; negative gain-placebo control PASS; sample-accurate automation PASS\n",
        substantial,weak);
    // Real counterexample must fail, not be disguised by loudness. Zero effect,
    // exact dry, therapy endpoints, placebo negatives always fail CTest.
    // Weak musical transformations are reported as evidence and kept for review.
    check(weak==0,"at least one 100%-wet effect/source has insufficient gain-independent change");
    return 0;
}
