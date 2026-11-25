#pragma once
#include "parameter_smoothing.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace psychiatry {
namespace creative {
constexpr double pi=3.14159265358979323846;
inline double rate(double sr) noexcept {return std::isfinite(sr)&&sr>=1000.&&sr<=2000000.?sr:48000.;}
inline float norm(float x) noexcept {return std::isfinite(x)?std::clamp(x,0.f,1.f):0.f;}
inline float sane(float x) noexcept {return std::isfinite(x)?std::clamp(x,-4.f,4.f):0.f;}
inline float pole(double hz,double sr) noexcept {return float(1.-std::exp(-2.*pi*hz/sr));}
inline bool valid(float* const* p,unsigned n) noexcept {return p&&p[0]&&(n==1||(n==2&&p[1]));}
inline float mix(float raw,float wet,float m) noexcept {
    return m==0.f?raw:m==1.f?wet:sane(raw)+m*(wet-sane(raw));
}
inline float random(std::uint32_t& r) noexcept {
    r^=r<<13;r^=r>>17;r^=r<<5;return float(r>>8)*(1.f/16777216.f);
}
inline float read(const std::vector<float>& h,std::size_t w,double lag) noexcept {
    lag=std::clamp(lag,1.,double(h.size()-3));
    const auto k=static_cast<std::size_t>(lag);
    const auto size=h.size();
    const auto a=w>=k?w-k:w+size-k,b=a?a-1:size-1;
    return h[a]+float(lag-double(k))*(h[b]-h[a]);
}
}
// Whole-source tape-like speed trajectories, not parallel harmony voices.
class Delirium final {
public:
    void prepare(double sr) {
        sr_=creative::rate(sr);step_=creative::pole(13.,sr_);
        for(auto& h:history_)h.assign(static_cast<std::size_t>(std::ceil(.48*sr_))+8,0.f);
        fadeLength_=std::max<std::size_t>(1,static_cast<std::size_t>(.028*sr_));
        reset();
    }
    void setAmount(float x) noexcept {amount_=creative::norm(x);}
    void reset() noexcept {
        for(auto& h:history_)std::fill(h.begin(),h.end(),0.f);
        write_=fade_=segment_=segmentLength_=0;lag_=nextLag_=.12*sr_;
        speedFrom_=speedTo_=1.;rng_=0x125A91E3u;m_=amount_;
    }
    void process(float* const* p,unsigned channels,std::size_t n,const float* curve=nullptr) noexcept {
        if(!creative::valid(p,channels)||history_[0].empty())return;
        for(std::size_t i=0;i<n;++i) {
            if(curve)amount_=curve[i];
            m_=detail::smoothToward(m_,amount_,step_);
            if(segment_>=segmentLength_) {
                speedFrom_=speedTo_;
                speedTo_=.4+1.2*creative::random(rng_);
                segmentLength_=std::max<std::size_t>(1,static_cast<std::size_t>(sr_*(.23+.47*creative::random(rng_))));
                segment_=0;
            }
            const double u=double(segment_++)/double(segmentLength_);
            const double eased=u*u*(3.-2.*u);
            const double speed=1.+m_*(speedFrom_+eased*(speedTo_-speedFrom_)-1.);
            if(!fade_&&(lag_<.04*sr_||lag_>.38*sr_)) {
                nextLag_=.17*sr_;fade_=1;
            }
            const float w=fade_?float(.5-.5*std::cos(creative::pi*double(fade_)/double(fadeLength_))):0.f;
            for(unsigned ch=0;ch<channels;++ch) {
                const float raw=p[ch][i];auto& h=history_[ch];
                const float old=creative::read(h,write_,lag_);
                const float wet=fade_?old+w*(creative::read(h,write_,nextLag_)-old):old;
                h[write_]=creative::sane(raw);
                p[ch][i]=creative::mix(raw,wet,m_);
            }
            lag_+=1.-speed;
            if(fade_) {
                nextLag_+=1.-speed;
                if(++fade_>fadeLength_){lag_=nextLag_;fade_=0;}
            }
            if(++write_==history_[0].size())write_=0;
        }
    }
private:
    std::array<std::vector<float>,2> history_{};
    double sr_=48000.,lag_=0.,nextLag_=0.,speedFrom_=1.,speedTo_=1.;
    std::uint32_t rng_=0x125A91E3u;
    std::size_t write_=0,fade_=0,fadeLength_=1,segment_=0,segmentLength_=0;
    float amount_=0.f,m_=0.f,step_=.001f;
};

// STFT freeze. Capture a complex spectrum, estimate phase advance from
// consecutive real input frames, propagate that phase independently of input.
// Four-way overlap/add uses Hann analysis and Hann synthesis windows.
template<bool Distributed=true> class CatatoniaImpl final {
    using C=std::complex<float>;
public:
    void prepare(double sr) {
        sr_=creative::rate(sr);step_=creative::pole(13.,sr_);
        size_=256;while(size_<4096&&double(size_)<.021*sr_)size_*=2;
        hop_=size_/4;
        slices_=0;for(auto n=size_;n>1;n/=2)++slices_;
        // One previous-frame transform over the next hop, in 16-sample slices.
        butterfliesPerSlice_=32*slices_;
        window_.resize(size_);twiddle_.resize(size_/2);
        bitReverse_.resize(size_);decay_.resize(size_/2+1);fallbackRotation_.resize(size_/2+1);
        for(std::size_t k=0;k<size_;++k)window_[k]=float(.5-.5*std::cos(2.*creative::pi*double(k)/double(size_)));
        for(std::size_t k=0;k<twiddle_.size();++k)twiddle_[k]=std::polar(1.f,float(-2.*creative::pi*double(k)/double(size_)));
        bitReverse_[0]=0;
        for(std::size_t i=1,j=0;i<size_;++i){
            std::size_t bit=size_>>1;for(;j&bit;bit>>=1)j^=bit;
            j^=bit;bitReverse_[i]=j;
        }
        for(std::size_t k=0;k<decay_.size();++k){
            const double f=2.*double(k)/double(size_);
            decay_[k]=float(std::exp(-double(hop_)*(1.+3.*f*f)/(sr_*1.6)));
            fallbackRotation_[k]=std::polar(1.f,float(2.*creative::pi*double(k*hop_)/double(size_)));
        }
        for(auto& c:state_) {
            c.history.assign(size_,0.f);c.ola.assign(size_,0.f);
            c.work.assign(size_,C{});c.previous.assign(size_,C{});
            c.held.assign(size_/2+1,C{});c.rotation.assign(size_/2+1,C{});
            c.future.assign(size_,C{});c.predicted.assign(size_/2+1,C{});
        }
        reset();
    }
    void setAmount(float x) noexcept {amount_=creative::norm(x);}
    void reset() noexcept {
        write_=ticks_=filled_=0;m_=amount_;
        for(auto& c:state_) {
            std::fill(c.history.begin(),c.history.end(),0.f);std::fill(c.ola.begin(),c.ola.end(),0.f);
            std::fill(c.previous.begin(),c.previous.end(),C{});std::fill(c.held.begin(),c.held.end(),C{});
            c.age=0;c.captureEnergy=0.;c.active=false;c.previousValid=false;
            c.analysisJob={};c.synthesisJob={};c.futureValid=false;
        }
    }
    void process(float* const* p,unsigned channels,std::size_t n,const float* curve=nullptr) noexcept {
        if(!creative::valid(p,channels)||window_.empty())return;
        for(std::size_t i=0;i<n;++i) {
            if(curve)amount_=curve[i];
            m_=detail::smoothToward(m_,amount_,step_);
            for(unsigned ch=0;ch<channels;++ch) {
                auto& c=state_[ch];const float raw=p[ch][i],wet=c.ola[write_];
                c.ola[write_]=0.f;c.history[write_]=creative::sane(raw);
                p[ch][i]=creative::mix(raw,wet,m_);
            }
            if(++write_==size_)write_=0;
            if(filled_<size_)++filled_;
            if constexpr(Distributed){
                if((ticks_&15u)==15u)for(unsigned ch=0;ch<channels;++ch)
                {
                    auto& c=state_[ch];
                    advance(c.previous,c.analysisJob,false,butterfliesPerSlice_);
                    advance(c.future,c.synthesisJob,true,butterfliesPerSlice_);
                }
            }
            if(++ticks_==hop_) {
                ticks_=0;
                for(unsigned ch=0;ch<channels;++ch) {
                    auto& c=state_[ch];
                    if(m_==0.f){c.active=false;c.previousValid=false;c.analysisJob={};c.synthesisJob={};c.futureValid=false;c.age=0;c.captureEnergy=0.;continue;}
                    frame(c);
                }
            }
        }
    }
private:
    struct TransformJob {bool pending=false;std::size_t half=1,base=0,k=0;};
    struct Channel {
        std::vector<float> history,ola;
        std::vector<C> work,previous,held,rotation,future,predicted;
        std::size_t age=0;
        double captureEnergy=0.;
        bool active=false,previousValid=false,futureValid=false;
        TransformJob analysisJob{},synthesisJob{};
    };
    void fft(std::vector<C>& a,bool inverse) noexcept {
        for(std::size_t i=1;i<size_;++i)if(i<bitReverse_[i])std::swap(a[i],a[bitReverse_[i]]);
        for(std::size_t len=2;len<=size_;len*=2) {
            const auto half=len/2,stride=size_/len;
            for(std::size_t base=0;base<size_;base+=len)
                for(std::size_t k=0;k<half;++k) {
                    const C tw=inverse?std::conj(twiddle_[k*stride]):twiddle_[k*stride];
                    const C u=a[base+k],x=a[base+k+half];
                    // Bounded finite frame values: avoid generic complex recovery.
                    const float vr=x.real()*tw.real()-x.imag()*tw.imag();
                    const float vi=x.real()*tw.imag()+x.imag()*tw.real();
                    a[base+k]=C(u.real()+vr,u.imag()+vi);
                    a[base+k+half]=C(u.real()-vr,u.imag()-vi);
                }
        }
        if(inverse){const float gain=1.f/float(size_);for(auto& x:a)x=C(x.real()*gain,x.imag()*gain);}
    }
    // Previous analysis is only consumed when a new frame is captured.
    // Keep the exact radix-2 order and twiddles, distributing the transform
    // over the following hop. Current-frame FFT is required only at capture.
    void advance(std::vector<C>& buffer,TransformJob& job,bool inverse,std::size_t budget) noexcept {
        // Run contiguous butterflies in batches. No variable divide per
        // butterfly and no additional precision/rounding changes.
        while(job.pending&&budget){
            const auto half=job.half,base=job.base,stride=size_/(2*half);
            const auto count=std::min(budget,half-job.k);
            const auto end=job.k+count;
            for(auto k=job.k;k<end;++k){
                const C tw=inverse?std::conj(twiddle_[k*stride]):twiddle_[k*stride];
                const C u=buffer[base+k],x=buffer[base+k+half];
                const float vr=x.real()*tw.real()-x.imag()*tw.imag();
                const float vi=x.real()*tw.imag()+x.imag()*tw.real();
                buffer[base+k]=C(u.real()+vr,u.imag()+vi);
                buffer[base+k+half]=C(u.real()-vr,u.imag()-vi);
            }
            budget-=count;job.k=end;
            if(job.k==half){
                job.k=0;job.base+=2*half;
                if(job.base==size_){
                    job.base=0;job.half*=2;
                    if(job.half==size_)job.pending=false;
                }
            }
        }
    }
    // During a hold the next decayed spectrum is completely determined.
    // Predict only that case. A capture always discards the prediction.
    // Output remains at the original hop boundary, with no added latency.
    void prepareFuture(Channel& c) noexcept {
        c.futureValid=true;
        for(std::size_t k=0;k<c.held.size();++k){
            C value=c.held[k];value*=c.rotation[k]*decay_[k];
            if(std::abs(value)<1.e-12f)value=C{};
            c.predicted[k]=value;
        }
        c.future[bitReverse_[0]]=C(c.predicted[0].real(),0.f);
        c.future[bitReverse_[size_/2]]=C(c.predicted[size_/2].real(),0.f);
        for(std::size_t k=1;k<size_/2;++k){
            c.future[bitReverse_[k]]=c.predicted[k];
            c.future[bitReverse_[size_-k]]=std::conj(c.predicted[k]);
        }
        c.synthesisJob={true,1,0,0};
    }
    void frame(Channel& c) noexcept {
        double energy=0.;
        for(std::size_t k=0;k<size_;++k) {
            const float x=c.history[(write_+k)&(size_-1)];
            const float tapered=x*window_[k];
            energy+=double(tapered)*tapered;c.work[k]=C(tapered,0.f);
        }
        if constexpr(!Distributed)fft(c.work,false);
        const auto hold=std::max<std::size_t>(1,static_cast<std::size_t>((.15+2.65*m_*m_)*sr_/double(hop_)));
        // A nearly silent leading edge (e.g. after other causal time effects)
        // must not monopolize a 2.8 s hold. Reacquire only when the incoming
        // frame has materially more energy than the original captured frame.
        // Compare to capture energy, NOT the decaying current held energy;
        // otherwise an equal-level new tone would break a deliberate freeze.
        const bool stronger=c.active&&energy>8.*c.captureEnergy;
        const bool capture=filled_==size_&&(!c.active||c.age>=hold||stronger)
            &&energy>double(size_)*1.e-14;
        if constexpr(Distributed){
            if(capture){
                // Normally finished before this boundary. Complete a partial
                // channel's prior analysis if the host changed active channels.
                if(c.previousValid&&c.analysisJob.pending)advance(c.previous,c.analysisJob,false,size_*slices_);
                fft(c.work,false);
            }
        }
        const bool useFuture=Distributed&&!capture&&c.active&&c.futureValid;
        if(useFuture&&c.synthesisJob.pending)advance(c.future,c.synthesisJob,true,size_*slices_);
        for(std::size_t k=0;k<c.held.size();++k) {
            if(capture) {
                const C now=c.work[k];
                const C cross=now*std::conj(c.previous[k]);
                const float magnitude=std::abs(cross);
                c.rotation[k]=c.previousValid&&magnitude>1.e-12f?cross/magnitude:fallbackRotation_[k];
                c.held[k]=now;
            } else if(c.active) {
                if(useFuture)c.held[k]=c.predicted[k];
                else{
                    c.held[k]*=c.rotation[k]*decay_[k];
                    if(std::abs(c.held[k])<1.e-12f)c.held[k]=C{};
                }
            }
            if constexpr(!Distributed)c.previous[k]=c.work[k];
        }
        c.previousValid=energy>double(size_)*1.e-14;
        if constexpr(Distributed){
            c.analysisJob={};
            if(c.previousValid){
                if(capture){
                    // Already transformed; reuse this exact spectrum.
                    std::copy(c.work.begin(),c.work.end(),c.previous.begin());
                }else{
                    // Fuse the previous transform's bit reversal with snapshot.
                    for(std::size_t k=0;k<size_;++k)c.previous[bitReverse_[k]]=c.work[k];
                    c.analysisJob={true,1,0,0};
                }
            }
        }
        if(capture){c.active=true;c.age=0;c.captureEnergy=energy;}
        else if(c.age<1000000u)++c.age;
        if(!c.active){c.futureValid=false;c.synthesisJob={};return;}
        if(useFuture){
            const float gain=1.f/float(size_);
            for(std::size_t k=0;k<size_;++k)
                c.ola[(write_+k)&(size_-1)]+=(c.future[k].real()*gain)*window_[k]*(2.f/3.f);
        }else{
            c.work[0]=C(c.held[0].real(),0.f);
            c.work[size_/2]=C(c.held[size_/2].real(),0.f);
            for(std::size_t k=1;k<size_/2;++k) {
                c.work[k]=c.held[k];c.work[size_-k]=std::conj(c.held[k]);
            }
            fft(c.work,true);
            for(std::size_t k=0;k<size_;++k)
                c.ola[(write_+k)&(size_-1)]+=c.work[k].real()*window_[k]*(2.f/3.f);
        }
        if constexpr(Distributed)prepareFuture(c);
    }
    std::array<Channel,2> state_{};
    std::vector<float> window_;
    std::vector<C> twiddle_,fallbackRotation_;
    std::vector<float> decay_;
    std::vector<std::size_t> bitReverse_;
    std::size_t size_=0,hop_=0,write_=0,ticks_=0,filled_=0,slices_=0,butterfliesPerSlice_=0;
    double sr_=48000.;
    float amount_=0.f,m_=0.f,step_=.001f;
};

using Catatonia=CatatoniaImpl<true>;

// Erase information rather than add shock sounds: stereo-linked smooth
// time and complementary-band masks, occasional one-shot past fragment.
class Amnesia final {
public:
    void prepare(double sr) {
        sr_=creative::rate(sr);step_=creative::pole(13.,sr_);
        low_=creative::pole(220.,sr_);high_=creative::pole(2200.,sr_);
        gateStep_=creative::pole(85.,sr_);
        for(auto& c:state_)c.history.assign(static_cast<std::size_t>(std::ceil(.32*sr_))+8,0.f);
        reset();
    }
    void setAmount(float x) noexcept {amount_=creative::norm(x);}
    void reset() noexcept {
        for(auto& c:state_){std::fill(c.history.begin(),c.history.end(),0.f);c.l=c.h=0.f;}
        write_=left_=age_=length_=0;rng_=0x125A92E1u;target_=gain_={1.f,1.f,1.f,1.f};
        fragment_=false;lag_=0.;m_=amount_;
    }
    void process(float* const* p,unsigned channels,std::size_t n,const float* curve=nullptr) noexcept {
        if(!creative::valid(p,channels)||state_[0].history.empty())return;
        for(std::size_t i=0;i<n;++i) {
            if(curve)amount_=curve[i];
            m_=detail::smoothToward(m_,amount_,step_);
            if(left_==0) {
                const float event=creative::random(rng_);
                length_=std::max<std::size_t>(1,static_cast<std::size_t>(sr_*(.075+.14*creative::random(rng_))));
                left_=length_;age_=0;target_={1.f,1.f,1.f,1.f};
                if(event<.36f)target_[3]=0.f;
                else if(event<.83f) {
                    const auto band=std::min(2u,static_cast<unsigned>(3.f*creative::random(rng_)));
                    target_[band]=0.f;
                    if(event>.68f)target_[(band+1)%3]=0.f;
                }
                fragment_=event<.13f;
                lag_=sr_*(.09+.16*creative::random(rng_));
            }
            for(std::size_t k=0;k<gain_.size();++k)
                gain_[k]+=gateStep_*(target_[k]-gain_[k]);
            const float u=float(age_)/float(length_);
            const float window=std::min({1.f,u*10.f,(1.f-u)*10.f});
            for(unsigned ch=0;ch<channels;++ch) {
                auto& c=state_[ch];const float raw=p[ch][i],x=creative::sane(raw);
                c.l+=low_*(x-c.l);c.h+=high_*(x-c.h);
                const float cut=gain_[3]*(gain_[0]*c.l+gain_[1]*(c.h-c.l)+gain_[2]*(x-c.h));
                const float resurfaced=fragment_?.22f*window*creative::read(c.history,write_,lag_):0.f;
                c.history[write_]=x;
                p[ch][i]=creative::mix(raw,cut+resurfaced,m_);
            }
            ++age_;--left_;if(++write_==state_[0].history.size())write_=0;
        }
    }
private:
    struct Channel {std::vector<float> history;float l=0.f,h=0.f;};
    std::array<Channel,2> state_{};
    std::array<float,4> target_{1.f,1.f,1.f,1.f},gain_{1.f,1.f,1.f,1.f};
    std::uint32_t rng_=0x125A92E1u;
    std::size_t write_=0,left_=0,age_=0,length_=0;
    double sr_=48000.,lag_=0.;
    float amount_=0.f,m_=0.f,step_=.001f,low_=0.f,high_=0.f,gateStep_=0.f;
    bool fragment_=false;
};
} // namespace psychiatry
