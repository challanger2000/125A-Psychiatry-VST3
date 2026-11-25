// P028l: actual DLL controller and native HWND host, not a duplicated UI model.
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#include "vst3/ids.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "public.sdk/source/vst/utility/memoryibstream.h"
#include "base/source/fstreamer.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace Steinberg;
using namespace psychiatry::vst3;
using Clock=std::chrono::steady_clock;
#define REQUIRE(x) do{if(!(x)){std::fprintf(stderr,"P028l native FAIL line %d: %s\n",__LINE__,#x);std::exit(4);}}while(0)
static void pump(int ms){
    const auto end=Clock::now()+std::chrono::milliseconds(ms);
    do{
        MSG msg{};
        while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        Sleep(1);
    }while(Clock::now()<end);
}
// The native host allows off-screen geometry; only 50% is pixel-tested.
static LRESULT CALLBACK auditWindowProc(HWND hwnd,UINT msg,WPARAM w,LPARAM l){
    if(msg==WM_GETMINMAXINFO){
        auto* info=reinterpret_cast<MINMAXINFO*>(l);
        info->ptMaxTrackSize.x=8192;info->ptMaxTrackSize.y=8192;
        return 0;
    }
    return DefWindowProcW(hwnd,msg,w,l);
}
struct Frame final:IPlugFrame{
    uint32 refs=1;
    HWND window=nullptr;
    bool accept=true;
    int requests=0;
    int detachOnResize=0;
    bool detached=false;
    tresult PLUGIN_API queryInterface(const TUID iid,void** out) override{
        if(!out)return kInvalidArgument;
        *out=nullptr;
        if(!FUnknownPrivate::iidEqual(iid,FUnknown::iid)&&!FUnknownPrivate::iidEqual(iid,IPlugFrame::iid))return kNoInterface;
        *out=static_cast<IPlugFrame*>(this);addRef();return kResultOk;
    }
    uint32 PLUGIN_API addRef() override{return ++refs;}
    uint32 PLUGIN_API release() override{return --refs;}
    tresult PLUGIN_API resizeView(IPlugView* view,ViewRect* size) override{
        ++requests;
        if(detachOnResize){
            const int order=detachOnResize;detachOnResize=0;
            if(order==1){REQUIRE(view->setFrame(nullptr)==kResultOk);REQUIRE(view->removed()==kResultOk);}
            else{REQUIRE(view->removed()==kResultOk);REQUIRE(view->setFrame(nullptr)==kResultOk);}
            detached=true;
            return kResultFalse;
        }
        if(!accept||!size||size->getWidth()<=0||size->getHeight()<=0)return kResultFalse;
        RECT r{0,0,size->getWidth(),size->getHeight()};
        if(!AdjustWindowRectEx(&r,WS_OVERLAPPEDWINDOW,FALSE,0))return kResultFalse;
        if(!SetWindowPos(window,nullptr,0,0,r.right-r.left,r.bottom-r.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE))return kResultFalse;
        return view->onSize(size);
    }
};
static void recall(Vst::IEditController* c,int motion,int zoom){
    ResizableMemoryIBStream mem;IBStreamer s(&mem,kLittleEndian);
    REQUIRE(s.writeInt32(0x50535955)&&s.writeInt32(3)&&s.writeInt32(0)&&s.writeInt32(motion)&&s.writeInt32(zoom));
    mem.rewind();REQUIRE(c->setState(&mem)==kResultOk);
}
static void saved(Vst::IEditController* c,int motion,int zoom){
    ResizableMemoryIBStream mem;REQUIRE(c->getState(&mem)==kResultOk);mem.rewind();
    IBStreamer s(&mem,kLittleEndian);int32 a=0,b=0,d=0,m=0,z=0;
    REQUIRE(s.readInt32(a)&&s.readInt32(b)&&s.readInt32(d)&&s.readInt32(m)&&s.readInt32(z));
    REQUIRE(a==0x50535955&&b==3&&m==motion&&z==zoom);
}
static constexpr std::array<int,6> widths{768,1152,1536,1920,2304,3072};
static constexpr std::array<int,6> heights{512,768,1024,1280,1536,2048};
struct View{
    Frame frame;
    IPlugView* view=nullptr;
    void open(Vst::IEditController* c,int x,int y){
        if(!view)view=c->createView(Vst::ViewType::kEditor);REQUIRE(view);
        frame.detached=false;
        ViewRect initial;REQUIRE(view->getSize(&initial)==kResultOk);
        RECT rect{0,0,initial.getWidth(),initial.getHeight()};
        REQUIRE(AdjustWindowRectEx(&rect,WS_OVERLAPPEDWINDOW,FALSE,0));
        frame.window=CreateWindowExW(0,L"125A_NativeAuditWindow",L"125A native audit",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
            x,y,rect.right-rect.left,rect.bottom-rect.top,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        REQUIRE(frame.window);
        REQUIRE(view->setFrame(&frame)==kResultOk);
        ShowWindow(frame.window,SW_SHOWNORMAL);UpdateWindow(frame.window);
        REQUIRE(view->attached(frame.window,kPlatformTypeHWND)==kResultOk);
        pump(150);
    }
    void size(int index){
        ViewRect v;RECT r{};
        REQUIRE(view->getSize(&v)==kResultOk&&GetClientRect(frame.window,&r));
        if(v.getWidth()!=widths[index]||v.getHeight()!=heights[index]||r.right!=widths[index]||r.bottom!=heights[index])
            std::fprintf(stderr,"geometry wanted %d x %d view %d x %d hwnd %ld x %ld\n",widths[index],heights[index],v.getWidth(),v.getHeight(),r.right,r.bottom);
        REQUIRE(v.getWidth()==widths[index]&&v.getHeight()==heights[index]&&r.right==widths[index]&&r.bottom==heights[index]);
    }
    void close(){
        if(!frame.detached){REQUIRE(view->removed()==kResultOk);REQUIRE(view->setFrame(nullptr)==kResultOk);}
        view->release();view=nullptr;DestroyWindow(frame.window);frame.window=nullptr;pump(20);
    }
};
// Capture only a visible client ROI from the actual composed desktop.
struct PixelCapture{
    HDC screen=nullptr,mem=nullptr;HBITMAP bmp=nullptr;HGDIOBJ old=nullptr;
    void* raw=nullptr;int width,height;
    PixelCapture(int w,int h):width(w),height(h){
        screen=GetDC(nullptr);REQUIRE(screen);
        mem=CreateCompatibleDC(screen);REQUIRE(mem);
        BITMAPINFO bi{};bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=w;
        bi.bmiHeader.biHeight=-h;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;
        bmp=CreateDIBSection(screen,&bi,DIB_RGB_COLORS,&raw,nullptr,0);REQUIRE(bmp&&raw);
        old=SelectObject(mem,bmp);REQUIRE(old);
    }
    ~PixelCapture(){SelectObject(mem,old);DeleteObject(bmp);DeleteDC(mem);ReleaseDC(nullptr,screen);}
    std::vector<unsigned char> grab(HWND window,int x,int y){
        POINT p{x,y};REQUIRE(ClientToScreen(window,&p));
        // Opaque composed desktop; no layered-window capture request or
        // per-frame creation/destruction of desktop GDI resources.
        REQUIRE(BitBlt(mem,0,0,width,height,screen,p.x,p.y,SRCCOPY));GdiFlush();
        std::vector<unsigned char> result;result.reserve(static_cast<size_t>(width*height*3));
        const auto* bytes=static_cast<const unsigned char*>(raw);
        for(int i=0;i<width*height;++i)for(int ch=0;ch<3;++ch)result.push_back(bytes[4*i+ch]);
        return result;
    }
};
static std::vector<unsigned char> pixels(HWND window,int x,int y,int w,int h){
    PixelCapture capture(w,h);return capture.grab(window,x,y);
}
static double delta(const std::vector<unsigned char>& a,const std::vector<unsigned char>& b){
    REQUIRE(a.size()==b.size()&&!a.empty());double sum=0.;
    for(size_t i=0;i<a.size();++i)sum+=std::abs(int(a[i])-int(b[i]));
    return sum/a.size();
}
static void world(Vst::IEditController* c,Vst::ParamID id){
    for(Vst::ParamID i=100;i<112;++i)REQUIRE(c->setParamNormalized(i,i==id?.5:0.)==kResultOk);
}
int wmain(int argc,wchar_t** argv){
    std::setvbuf(stdout,nullptr,_IONBF,0);
    std::setvbuf(stderr,nullptr,_IONBF,0);
    std::puts("P028l native harness entered");
    REQUIRE(argc==2);
    // The official Win32 EditorHost initializes STA COM before loading a module.
    const HRESULT com=CoInitialize(nullptr);
    REQUIRE(SUCCEEDED(com));
    SetProcessDPIAware();
    REQUIRE(timeBeginPeriod(1)==TIMERR_NOERROR);
    WNDCLASSW cls{};cls.lpfnWndProc=auditWindowProc;cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"125A_NativeAuditWindow";
    REQUIRE(RegisterClassW(&cls)!=0);
    std::puts("P028l native STA COM initialized; loading module");
    HMODULE dll=LoadLibraryExW(argv[1],nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if(!dll)std::fprintf(stderr,"LoadLibraryExW error %lu\n",GetLastError());
    REQUIRE(dll);
    std::puts("P028l native module loaded");
    auto init=reinterpret_cast<bool(*)()>(GetProcAddress(dll,"InitDll"));
    auto exit=reinterpret_cast<bool(*)()>(GetProcAddress(dll,"ExitDll"));
    auto factoryFn=reinterpret_cast<IPluginFactory*(PLUGIN_API*)()>(GetProcAddress(dll,"GetPluginFactory"));
    REQUIRE(init&&exit&&factoryFn);
    std::puts("P028l native calling InitDll");
    REQUIRE(init());
    std::puts("P028l native InitDll returned");
    auto* factory=factoryFn();REQUIRE(factory&&factory->countClasses()==2);
    Vst::IEditController* c=nullptr;
    TUID cid,iid;kControllerCID.toTUID(cid);Vst::IEditController::iid.toTUID(iid);
    REQUIRE(factory->createInstance(cid,iid,reinterpret_cast<void**>(&c))==kResultOk&&c);
    std::puts("P028l native controller created");
    REQUIRE(c->initialize(nullptr)==kResultOk);
    std::puts("P028l native controller initialized");
    recall(c,1,0);saved(c,1,0);
    std::puts("P028l native opening first editor");
    View a;a.open(c,15,15);a.size(0);saved(c,1,0);
    std::puts("P028l native first editor attached; checking MOTION");
    SetForegroundWindow(a.frame.window);pump(150);
    auto motionOn=pixels(a.frame.window,291,482,24,24);
    recall(c,0,0);pump(180);saved(c,0,0);
    auto motionOff=pixels(a.frame.window,291,482,24,24);
    REQUIRE(delta(motionOn,motionOff)>.25);
    recall(c,1,0);pump(180);saved(c,1,0);
    REQUIRE(delta(motionOn,pixels(a.frame.window,291,482,24,24))<.5);
    std::puts("P028l native testing accepted zooms");
    for(int z=0;z<6;++z){recall(c,1,z);pump(60);a.size(z);saved(c,1,z);}
    recall(c,1,0);pump(100);a.size(0);
    std::puts("P028l native testing refused zooms");
    a.frame.accept=false;
    for(int z=1;z<6;++z){recall(c,1,z);pump(40);a.size(0);saved(c,1,0);}
    a.frame.accept=true;
    std::puts("P028l native testing second view");
    View b;b.open(c,810,20);b.size(0);saved(c,1,0);
    recall(c,0,1);pump(100);a.size(1);b.size(1);saved(c,0,1);
    b.frame.accept=false;recall(c,1,0);pump(100);a.size(0);b.size(1);saved(c,1,0);
    b.close();a.close();saved(c,1,0);
    std::puts("N03 create before recall, attach after recall");
    {View pending;pending.view=c->createView(Vst::ViewType::kEditor);REQUIRE(pending.view);
     recall(c,1,2);pending.open(c,15,15);pending.size(2);saved(c,1,2);pending.close();}
    for(int order=1;order<=2;++order){
        recall(c,1,0);
        View closing,survivor;closing.open(c,15,15);survivor.open(c,810,20);
        closing.size(0);survivor.size(0);
        closing.frame.detachOnResize=order;
        recall(c,1,2);pump(100);
        REQUIRE(closing.frame.detached);survivor.size(2);saved(c,1,2);
        closing.close();survivor.close();
        View reopened;reopened.open(c,15,15);reopened.size(2);reopened.close();
        std::printf("N02 native synchronous detach order %d with survivor/reopen PASS\n",order);
    }
    // If the only view closes, do not save an unaccepted size.
    recall(c,1,0);
    {View closing;closing.open(c,15,15);closing.frame.detachOnResize=1;
     recall(c,1,2);REQUIRE(closing.frame.detached);saved(c,1,0);closing.close();}
    recall(c,1,0);
    a.open(c,15,15);a.size(0);saved(c,1,0);
    SetForegroundWindow(a.frame.window);
    std::puts("P028l native testing interrupted fade");
    PixelCapture capture(120,120);
    bool measured=false;
    for(int trial=0;trial<3&&!measured;++trial){
        world(c,kDepression);pump(1600);
        auto before=capture.grab(a.frame.window,323,190);
        auto fixed=pixels(a.frame.window,50,50,60,60);
        auto last=before;
        double maxStep=0.,retarget=0.;int samples=0,validRetarget=0;
        auto lastTime=Clock::now();const auto start=lastTime;
        auto switchTime=start;bool switched=false,bracketed=false;
        world(c,kMania);
        while(Clock::now()-start<std::chrono::milliseconds(1850)){
            if(!switched&&Clock::now()-start>=std::chrono::milliseconds(310)){
                switchTime=Clock::now();world(c,kPanic);switched=true;
            }
            pump(1);auto next=capture.grab(a.frame.window,323,190);auto now=Clock::now();
            const double t=std::chrono::duration<double,std::milli>(now-start).count();
            const double gap=std::chrono::duration<double,std::milli>(now-lastTime).count();
            double d=delta(last,next);
            const double sinceSwitch=std::chrono::duration<double,std::milli>(now-switchTime).count();
            std::printf("R04 trial=%d t=%.2f gap=%.2f delta=%.3f switched=%d\n",trial,t,gap,d,int(switched));
            if(gap<=65){
                maxStep=std::max(maxStep,d);++samples;
                if(switched&&sinceSwitch<=190){
                    retarget=std::max(retarget,d);++validRetarget;
                    if(lastTime<=switchTime)bracketed=true;
                }
                // Never retry a valid observed discontinuity.
                REQUIRE(d<15.);
            }
            last=std::move(next);lastTime=now;
        }
        REQUIRE(delta(before,last)>5.&&delta(fixed,pixels(a.frame.window,50,50,60,60))<.5);
        measured=samples>=15&&validRetarget>=2&&switched&&bracketed;
        std::printf("R04 trial %d max %.4f retarget %.4f short-gap=%d retarget=%d bracketed=%d valid=%d\n",
            trial,maxStep,retarget,samples,validRetarget,int(bracketed),int(measured));
    }
    REQUIRE(measured);
    a.close();saved(c,1,0);
    REQUIRE(c->terminate()==kResultOk);c->release();factory->release();REQUIRE(exit());FreeLibrary(dll);timeEndPeriod(1);CoUninitialize();
    std::puts("P028l R04/R05/R07 native HWND actual DLL probes PASS (six geometry levels; visible pixels at 50%)");
    return 0;
}
