#include "spectral_worlds.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
namespace {
constexpr unsigned kRate=48000;
constexpr std::size_t kFrames=kRate*6;
using Audio=std::vector<float>;
struct Stereo {Audio l,r;};
void write16(std::ofstream& f,std::uint16_t v){
    f.put(static_cast<char>(v&255));f.put(static_cast<char>((v>>8)&255));
}
void write32(std::ofstream& f,std::uint32_t v){
    for(int j=0;j<4;++j)f.put(static_cast<char>((v>>(j*8))&255));
}
bool save(const std::string& path,const Stereo& s) {
    std::ofstream f(path,std::ios::binary);
    if(!f)return false;
    const auto size=static_cast<std::uint32_t>(4*s.l.size());
    f.write("RIFF",4);write32(f,size+36);f.write("WAVEfmt ",8);
    write32(f,16);write16(f,1);write16(f,2);write32(f,kRate);
    write32(f,kRate*4);write16(f,4);write16(f,16);
    f.write("data",4);write32(f,size);
    auto sample=[](float f)->std::uint16_t {
        const auto v=static_cast<std::int16_t>(
            std::lround(std::clamp(f,-1.f,1.f)*32767.f));
        return static_cast<std::uint16_t>(v);
    };
    for(std::size_t i=0;i<s.l.size();++i){
        write16(f,sample(s.l[i]));write16(f,sample(s.r[i]));
    }
    return f.good();
}
Stereo source() {
    Stereo s{Audio(kFrames),Audio(kFrames)};
    std::uint32_t rnd=0x125A7C23u;
    for(std::size_t i=0;i<kFrames;++i){
        const double t=static_cast<double>(i)/kRate;
        const auto beat=i%(kRate/4);
        const float onset=beat<3800?0.52f*
              std::exp(-beat/720.f)*std::sin(6.28318530718f*
                    (64.f+130.f*std::exp(-beat/300.f))*beat/kRate):0.f;
        rnd^=rnd<<13;rnd^=rnd>>17;rnd^=rnd<<5;
        const float hiss=(static_cast<int>(rnd&0xffff)-32768)/32768.f;
        s.l[i]=static_cast<float>(0.28*std::sin(6.28318530718*87.*t)
           +0.10*std::sin(6.28318530718*1180.*t)+onset+0.025*hiss);
        s.r[i]=static_cast<float>(0.26*std::sin(6.28318530718*89.*t)
           +0.12*std::sin(6.28318530718*1160.*t)+onset+0.021*hiss);
    }
    return s;
}
template<class FX>
void process(FX& fx,Stereo& s) {
    for(std::size_t i=0;i<s.l.size();i+=333){
        float* data[]{s.l.data()+i,s.r.data()+i};
        fx.process(data,2,std::min<std::size_t>(333,s.l.size()-i));
    }
}
}
int main(int argc,char** argv) {
    if(argc!=2) {std::fprintf(stderr,"render_three_worlds <output_prefix>\n");return 2;}
    const auto dry=source();
    auto schizophrenia=dry,dissociation=dry,hallucination=dry,combined=dry;
    psychiatry::Schizophrenia m;m.prepare(kRate);m.setAmount(0.86f);m.reset();
    psychiatry::Dissociation p;p.prepare(kRate);p.setAmount(0.86f);p.reset();
    psychiatry::Hallucination z;z.prepare(kRate);z.setAmount(0.86f);z.reset();
    process(m,schizophrenia);process(p,dissociation);process(z,hallucination);
    psychiatry::Schizophrenia m2;m2.prepare(kRate);m2.setAmount(0.65f);m2.reset();
    psychiatry::Dissociation p2;p2.prepare(kRate);p2.setAmount(0.57f);p2.reset();
    psychiatry::Hallucination z2;z2.prepare(kRate);z2.setAmount(0.58f);z2.reset();
    process(m2,combined);process(p2,combined);process(z2,combined);
    const std::string prefix=argv[1];
    if(!save(prefix+"_DRY.wav",dry)||
       !save(prefix+"_SCHIZOPHRENIA_86.wav",schizophrenia)||
       !save(prefix+"_DISSOCIATION_86.wav",dissociation)||
       !save(prefix+"_HALLUCINATION_86.wav",hallucination)||
       !save(prefix+"_COMBINED.wav",combined)) {
        std::fprintf(stderr,"Audio example export failed\n");return 1;
    }
    std::printf("P005: five stereo comparisons created\n");
}
