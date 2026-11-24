#include "linear_phase_2x.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <complex>
#include <algorithm>
using namespace psychiatry::detail;
constexpr double pi=3.14159265358979323846;
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"P018 filter FAIL line %d: %s\n",__LINE__,#x);std::abort();}}while(0)
int main(){
 LinearPhaseOversample2x os;
 std::vector<float> impulse(128);
 for(int i=0;i<128;i++)impulse[i]=os.process(i==0?1.f:0.f,[](float x){return x;});
 double asym=0,sum=0;
 for(int i=0;i<49;i++){asym=std::max(asym,std::abs(double(impulse[i])-impulse[48-i]));sum+=impulse[i];}
 CHECK(asym<1.e-7&&std::abs(sum-1)<1.e-6);
 CHECK(std::max_element(impulse.begin(),impulse.end())-impulse.begin()==24);
 FixedDelay<24> delay;
 for(int n=0;n<128;n++)CHECK(delay.process(n==0?1.f:0.f)==(n==24?1.f:0.f));
 std::printf("P018 FIR impulse symmetry %.9g DC %.9g center %.9g\n",asym,sum,impulse[24]);
 for(int sr:{44100,48000,96000}){
  for(double hz:{100.,1000.,4000.,8000.,12000.,16000.}){
   os.reset();std::complex<double>d{},w{};
   for(int n=0;n<sr+1000;n++){
    float x=.1f*std::sin(2*pi*hz*n/sr);float y=os.process(x,[](float a){return a;});
    if(n>=1000){auto p=std::exp(std::complex<double>(0,-2*pi*hz*(n-24)/sr));w+=double(y)*p;d+=double(.1f*std::sin(2*pi*hz*(n-24)/sr))*p;}
   }
   auto H=w/d;double loss=20*std::log10(std::abs(1.+H)/(1.+std::abs(H)));
   std::printf("P018 FIR sr=%d hz=%.0f gain=%+.5f phase=%+.5f cancellation=%+.5f\n",sr,hz,20*log10(abs(H)),arg(H)*180/pi,loss);
   CHECK(std::abs(arg(H))<1.e-4&&std::abs(20*log10(abs(H)))<.005);
  }
  if(sr==96000)continue;
  for(double hz:{14000.,15000.}){
   os.reset();std::complex<double>carrier{},alias{};double f=std::abs(3*hz-sr);
   for(int n=0;n<sr+1000;n++){float x=.35f*sin(2*pi*hz*n/sr);float y=os.process(x,[](float a){return std::tanh(3.5f*a);});if(n>=1000){carrier+=double(y)*std::exp(std::complex<double>(0,-2*pi*hz*n/sr));alias+=double(y)*std::exp(std::complex<double>(0,-2*pi*f*n/sr));}}
   double db=20*log10(abs(alias)/abs(carrier));
   std::printf("P018 FIR tanh sr=%d %.0f->%.0f alias %.2f dBc\n",sr,hz,f,db);CHECK(db< -50.);
  }
 }
}
