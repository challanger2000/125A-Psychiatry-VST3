#include "paranoia.hpp"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>

// Checks remain active in Release builds (unlike assert with NDEBUG).
#define CHECK(expr) do { if (!(expr)) { \
    std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #expr, __FILE__, __LINE__); \
    std::abort(); } } while (0)

using psychiatry::Paranoia;
namespace {
using Audio = std::vector<float>;
void run(Paranoia& fx, Audio& l, Audio& r, std::size_t block) {
    CHECK(l.size() == r.size());
    for (std::size_t i = 0; i < l.size(); i += block) {
        float* ch[] = {l.data()+i, r.data()+i};
        fx.process(ch, 2, std::min(block, l.size()-i));
    }
}
Audio makeSignal(std::size_t n, double sr) {
    Audio out(n);
    for (std::size_t i=0;i<n;++i) {
        const double t = static_cast<double>(i) / sr;
        const float burst = (i % static_cast<std::size_t>(sr/3)) < 80 ? 0.9f : 0.0f;
        out[i] = 0.35f * std::sin(6.283185307179586 * 163.0 * t) + burst;
    }
    return out;
}
}
int main() {
    constexpr double sr = 48000.0;
    constexpr std::size_t n = 96000;
    const auto original = makeSignal(n, sr);
    auto r0 = original;
    {
        Paranoia fx;
        fx.prepare(sr);
        fx.setSettings({});
        auto l = original;
        run(fx, l, r0, 1);
        CHECK(l == original && r0 == original && "0 macro must pass through exactly");
        CHECK(fx.stats().reverseEvents == 0);
    }
    {
        Paranoia fx;
        fx.prepare(sr);
        Paranoia::Settings s;
        s.amount = 1.0f; s.therapy = 1.0f;
        s.trigger = Paranoia::Trigger::Timed;
        s.pattern = Paranoia::Pattern::Every;
        s.intervalMs = 67.0f;
        fx.setSettings(s);
        fx.reset();
        auto l = original, r = original;
        run(fx, l, r, 277);
        CHECK(l == original && r == original && "100% therapy must be exactly dry");
        CHECK(fx.stats().reverseEvents > 5);
        CHECK(fx.latencySamples() == 0);
    }
    {
        Paranoia fx;
        fx.prepare(sr);
        fx.setSeed(12345);
        Paranoia::Settings s;
        s.amount=0.90f; s.trigger=Paranoia::Trigger::Timed;
        s.pattern=Paranoia::Pattern::Every; s.intervalMs=78.0f;
        fx.setSettings(s);
        fx.reset();
        auto l=original,r=original;
        run(fx,l,r,137);
        const auto st=fx.stats();
        CHECK(st.triggers > 8 && st.reverseEvents > 8 && st.reversedSamples > 1000);
        CHECK(st.acousticEvents > 0 && "P026c real acoustic sample events");
        double difference=0,energy=0;
        for(std::size_t i=0;i<n;++i){
            CHECK(std::isfinite(l[i])&&std::isfinite(r[i]));
            CHECK(std::abs(l[i]) < 2.0f && std::abs(r[i]) < 2.0f);
            difference += std::abs(l[i]-original[i]);
            energy += std::abs(original[i]);
        }
        CHECK(difference/energy > 0.09 && "effect should be clearly distinguishable");
        // Audio processing is independent of host buffer segmentation.
        Paranoia other; other.prepare(sr); other.setSeed(12345); other.setSettings(s); other.reset();
        auto x=original,y=original;
        run(other,x,y,1024);
        CHECK(l==x && r==y && "DSP must be block-size invariant");
        std::printf("P001: %llu reverse events, wet difference %.3f\n",
            static_cast<unsigned long long>(st.reverseEvents), difference/energy);
    }
    {
        Paranoia fx; fx.prepare(44100.0);
        Paranoia::Settings s; s.amount=1; s.trigger=Paranoia::Trigger::Timed;
        s.clock=Paranoia::Clock::BeatDivision; s.bpm=120.0f;
        s.division=4; s.pattern=Paranoia::Pattern::Alternating;
        fx.setSettings(s); fx.reset();
        Audio l(44100,0.2f),r=l;
        run(fx,l,r,64);
        CHECK(fx.stats().triggers == 8);
        CHECK(fx.stats().reverseEvents == 4);
    }
    {
        Paranoia fx; fx.prepare(48000.0);
        Paranoia::Settings s; s.amount=1; s.trigger=Paranoia::Trigger::Transient;
        s.pattern=Paranoia::Pattern::Every;
        fx.setSettings(s); fx.reset();
        Audio l(96000,0.0f),r=l;
        for(std::size_t i=4000;i<96000;i+=14000) {
            for(std::size_t j=0;j<500;++j) l[i+j]=r[i+j]=1.0f;
        }
        run(fx,l,r,128);
        CHECK(fx.stats().reverseEvents>=1 && "audio-level triggering must work");
    }
    {
        Paranoia fx; fx.prepare(192000.0);
        Paranoia::Settings s; s.amount=1; s.therapy=0;
        fx.setSettings(s); fx.reset();
        Audio l(192000,0.5f),r=l;
        l[500]=std::numeric_limits<float>::infinity();
        r[900]=std::numeric_limits<float>::quiet_NaN();
        run(fx,l,r,3);
        for(std::size_t i=0;i<l.size();++i)
            CHECK(std::isfinite(l[i]) && std::isfinite(r[i]));
    }
    {
        const Audio extended{0.f,1.e-30f,-1.e-30f,20.f,-20.f,0.3f};
        for(float therapy:{0.f,1.f}) {
            Paranoia fx; fx.prepare(sr);
            Paranoia::Settings s;
            s.amount=therapy==0.f?0.f:1.f; s.therapy=therapy;
            fx.setSettings(s);fx.reset();
            auto l=extended,r=extended; run(fx,l,r,3);
            CHECK(l==extended && r==extended && "P016 extended-range dry");
        }
        Paranoia fx;fx.prepare(sr);fx.setSeed(125);
        Paranoia::Settings s;
        s.amount=.9f;s.trigger=Paranoia::Trigger::Timed;
        s.pattern=Paranoia::Pattern::Random;s.intervalMs=78.f;
        fx.setSettings(s);fx.reset();
        auto l=original,r=original;run(fx,l,r,137);
        fx.reset();
        auto x=original,y=original;run(fx,x,y,1024);
        CHECK(l==x&&r==y&&"P016 reset must reproduce seeded render");
    }
    {
        // Separate source-player contract: real archived PCM must emit
        // nonzero output when driven, never during zero-input startup.
        psychiatry::ReactiveAcoustic a,b;
        for(double rate:{44100.,48000.,96000.}) {
            a.prepare(rate);b.prepare(rate);
            for(int n=0;n<static_cast<int>(rate);++n) {
                const auto z=a.tick(0.f,1.f);CHECK(z.l==0.f&&z.r==0.f);
            }
            CHECK(a.events()==0);
            // Same input from reset, despite a different history of silence.
            a.reset();b.reset();
            double energy=0.,diff=0.;
            for(int n=0;n<static_cast<int>(rate*4.);++n) {
                const auto l=a.tick(.2f,1.f),r=b.tick(.2f,1.f);
                CHECK(l.l==r.l&&l.r==r.r);
                CHECK(std::isfinite(l.l)&&std::abs(l.l)<=.5f);
                energy+=l.l*l.l+l.r*l.r;diff+=std::abs(l.l-l.r);
            }
            CHECK(a.events()>=2&&energy>1.&&diff>1.);
            for(int n=0;n<200;++n) {
                const auto z=a.tick(.2f,0.f);CHECK(z.l==0.f&&z.r==0.f);
            }
            std::printf("P026c rate=%.0f events=%llu energy=%.4f stereo_delta=%.4f\n",
              rate,static_cast<unsigned long long>(a.events()),energy,diff);
        }
    }
    std::puts("P001 PASS: dry, therapy, finite outputs, reverse triggers, tempo, transient, block size, P016 reset and extended-range endpoints");
}
