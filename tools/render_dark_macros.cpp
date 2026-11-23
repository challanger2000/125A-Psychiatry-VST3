#include "depression.hpp"
#include "obsession.hpp"
#include "paranoia.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace {
constexpr unsigned kRate=48000;
constexpr std::size_t kSamples=kRate*6;
using Audio=std::vector<float>;
struct Stereo {Audio l,r;};
void u16(std::ofstream& f,std::uint16_t v) {
    f.put(static_cast<char>(v&255));
    f.put(static_cast<char>((v>>8)&255));
}
void u32(std::ofstream& f,std::uint32_t v) {
    for(int i=0;i<4;++i) f.put(static_cast<char>((v>>(8*i))&255));
}
bool save(const std::string& file,const Stereo& s) {
    if(s.l.size()!=s.r.size()||s.l.size()>0x3fffffffU) return false;
    std::ofstream f(file,std::ios::binary);
    if(!f) return false;
    const auto bytes=static_cast<std::uint32_t>(s.l.size()*4);
    f.write("RIFF",4);u32(f,36+bytes);f.write("WAVEfmt ",8);
    u32(f,16);u16(f,1);u16(f,2);u32(f,kRate);u32(f,kRate*4);
    u16(f,4);u16(f,16);f.write("data",4);u32(f,bytes);
    for(std::size_t i=0;i<s.l.size();++i) {
        const auto toInt=[](float x)->std::uint16_t {
            const auto sample=static_cast<std::int16_t>(std::lround(
                32767.f*std::clamp(std::isfinite(x)?x:0.f,-1.f,1.f)));
            return static_cast<std::uint16_t>(sample);
        };
        u16(f,toInt(s.l[i]));u16(f,toInt(s.r[i]));
    }
    return f.good();
}
Stereo generate() {
    Stereo s{Audio(kSamples),Audio(kSamples)};
    std::uint32_t seed=0xA125F003U;
    for(std::size_t i=0;i<kSamples;++i) {
        const double t=static_cast<double>(i)/kRate;
        const auto note=55.*std::pow(2.,(static_cast<int>(t*2)%4)*2./12.);
        const auto phase=6.283185307179586*note*t;
        const auto bass=0.23*std::sin(phase)+0.08*std::sin(2.*phase);
        const auto pulse=static_cast<int>(i%12000);
        const auto drum=pulse<400 ? 0.35*std::sin(6.283185307179586*(
            50.+140.*std::exp(-pulse/110.))*pulse/kRate)
            *std::exp(-pulse/120.) : 0.;
        seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
        const auto noise=(static_cast<double>(seed)/4294967295.0-.5);
        const auto breath=0.04*noise*(0.5+0.5*std::sin(t*3.4));
        s.l[i]=static_cast<float>(bass+drum+breath);
        s.r[i]=static_cast<float>(bass+drum-breath*0.8+0.014*std::sin(t*122.));
    }
    return s;
}
template<class Processor>
void apply(Processor& fx,Stereo& s) {
    for(std::size_t i=0;i<s.l.size();i+=256) {
        float* ptrs[]{s.l.data()+i,s.r.data()+i};
        fx.process(ptrs,2,std::min<std::size_t>(256,s.l.size()-i));
    }
}
double diff(const Stereo& dry,const Stereo& processed) {
    double difference=0,level=0;
    for(std::size_t i=0;i<dry.l.size();++i) {
        difference+=std::abs(dry.l[i]-processed.l[i])+
            std::abs(dry.r[i]-processed.r[i]);
        level+=std::abs(dry.l[i])+std::abs(dry.r[i]);
    }
    return level>0?difference/level:0;
}
}
int main(int argc,char** argv) {
    if(argc!=2) {std::fprintf(stderr,"usage: render_dark_macros <prefix>\n");return 2;}
    const auto dry=generate();
    auto depression=dry,obsession=dry,combined=dry;
    psychiatry::Depression d;d.prepare(kRate);d.setAmount(0.88f);d.reset();
    apply(d,depression);
    psychiatry::Obsession o;o.prepare(kRate);o.setAmount(0.88f);o.reset();
    apply(o,obsession);
    psychiatry::Depression dc;dc.prepare(kRate);dc.setAmount(0.75f);dc.reset();
    psychiatry::Paranoia p;p.prepare(kRate);
    psychiatry::Paranoia::Settings ps;
    ps.amount=0.70f;ps.therapy=0.f;ps.pattern=psychiatry::Paranoia::Pattern::Every;
    p.setSettings(ps);p.reset();
    psychiatry::Obsession oc;oc.prepare(kRate);oc.setAmount(0.65f);oc.reset();
    apply(dc,combined);apply(p,combined);apply(oc,combined);
    const std::string base=argv[1];
    if(!save(base+"_DRY.wav",dry)||
       !save(base+"_DEPRESSION_88.wav",depression)||
       !save(base+"_OBSESSION_88.wav",obsession)||
       !save(base+"_COMBINED.wav",combined)) {
        std::fputs("could not write WAV examples\n",stderr);return 1;
    }
    std::printf("P003 sample differences vs dry: DEPRESSION=%.3f, OBSESSION=%.3f, COMBINED=%.3f\n",
        diff(dry,depression),diff(dry,obsession),diff(dry,combined));
    std::printf("PARANOIA reverse events in COMBINED: %llu\n",
        static_cast<unsigned long long>(p.stats().reverseEvents));
    return 0;
}
