#include "paranoia.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace {
void u16(std::ofstream& f, std::uint16_t x) { for(int k=0;k<2;++k) f.put(static_cast<char>((x>>(8*k))&255)); }
void u32(std::ofstream& f, std::uint32_t x) { for(int k=0;k<4;++k) f.put(static_cast<char>((x>>(8*k))&255)); }
void saveWav(const std::string& path, const std::vector<float>& a, const std::vector<float>& b) {
    std::ofstream f(path,std::ios::binary);
    if(!f) return;
    const auto bytes=static_cast<std::uint32_t>(a.size()*2*sizeof(std::int16_t));
    f.write("RIFF",4); u32(f,36+bytes); f.write("WAVEfmt ",8);
    u32(f,16); u16(f,1); u16(f,2); u32(f,48000); u32(f,48000*4); u16(f,4); u16(f,16);
    f.write("data",4); u32(f,bytes);
    for(std::size_t i=0;i<a.size();++i) {
        const auto l=static_cast<std::int16_t>(std::lround(32767.f*std::clamp(a[i],-1.f,1.f)));
        const auto r=static_cast<std::int16_t>(std::lround(32767.f*std::clamp(b[i],-1.f,1.f)));
        u16(f,static_cast<std::uint16_t>(l)); u16(f,static_cast<std::uint16_t>(r));
    }
}
}
int main(int argc,char** argv) {
    if(argc!=2) { std::fprintf(stderr,"usage: render_paranoia <output_prefix>\n"); return 2; }
    constexpr int sr=48000, frames=sr*5;
    std::vector<float> l(frames), r(frames);
    for(int i=0;i<frames;++i) {
        const double t=static_cast<double>(i)/sr;
        const auto kick=(i%12000)<800 ? std::sin(2.0*3.141592653589793*90*t) * std::exp(-(i%12000)/180.0) : 0.0;
        l[i]=static_cast<float>(0.28*std::sin(2*3.141592653589793*110*t) + kick*0.5);
        r[i]=static_cast<float>(0.28*std::sin(2*3.141592653589793*111*t) + kick*0.5);
    }
    saveWav(std::string(argv[1])+"_dry.wav",l,r);
    psychiatry::Paranoia fx; fx.prepare(sr); fx.setSeed(987654321);
    psychiatry::Paranoia::Settings settings;
    settings.amount=0.85f;
    settings.therapy=0.0f;
    settings.intervalMs=220.0f;
    settings.pattern=psychiatry::Paranoia::Pattern::Every;
    settings.trigger=psychiatry::Paranoia::Trigger::Hybrid;
    fx.setSettings(settings);fx.reset();
    for(int i=0;i<frames;i+=256){ float* channels[]={l.data()+i,r.data()+i}; fx.process(channels,2,std::min(256,frames-i)); }
    saveWav(std::string(argv[1])+"_wet.wav",l,r);
    std::printf("Rendered %zu reverse events (zero added latency; previous audio only)\n",
        static_cast<std::size_t>(fx.stats().reverseEvents));
}
