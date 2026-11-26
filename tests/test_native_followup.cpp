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
#include "pluginterfaces/gui/iplugviewcontentscalesupport.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "public.sdk/source/vst/utility/memoryibstream.h"
#include "base/source/fstreamer.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <utility>
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
    bool equalSizeShortcut=false;
    bool fixedParent=false;
    std::function<tresult(IPlugView*,ViewRect*)> resizeOnce;
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
        if(resizeOnce){auto callback=std::move(resizeOnce);resizeOnce={};return callback(view,size);}
        if(detachOnResize){
            const int order=detachOnResize;detachOnResize=0;
            if(order==1){REQUIRE(view->setFrame(nullptr)==kResultOk);REQUIRE(view->removed()==kResultOk);}
            else{REQUIRE(view->removed()==kResultOk);REQUIRE(view->setFrame(nullptr)==kResultOk);}
            detached=true;
            return kResultFalse;
        }
        if(!accept||!size||size->getWidth()<=0||size->getHeight()<=0)return kResultFalse;
        if(fixedParent){
            RECT actual{};REQUIRE(GetClientRect(window,&actual));
            return actual.right==size->getWidth()&&actual.bottom==size->getHeight()?kResultTrue:kResultFalse;
        }
        if(equalSizeShortcut){
            ViewRect current;REQUIRE(view->getSize(&current)==kResultOk);
            if(current.left==size->left&&current.top==size->top&&
               current.right==size->right&&current.bottom==size->bottom)return kResultTrue; // official EditorHost semantics
        }
        RECT r{0,0,size->getWidth(),size->getHeight()};
        if(!AdjustWindowRectEx(&r,WS_OVERLAPPEDWINDOW,FALSE,0))return kResultFalse;
        if(!SetWindowPos(window,nullptr,0,0,r.right-r.left,r.bottom-r.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE))return kResultFalse;
        return view->onSize(size);
    }
};
static void recall(Vst::IEditController* c,int motion){
    ResizableMemoryIBStream mem;IBStreamer s(&mem,kLittleEndian);
    REQUIRE(s.writeInt32(0x50535955)&&s.writeInt32(4)&&s.writeInt32(0)&&s.writeInt32(motion));
    mem.rewind();REQUIRE(c->setState(&mem)==kResultOk);
}
static void saved(Vst::IEditController* c,int motion){
    ResizableMemoryIBStream mem;REQUIRE(c->getState(&mem)==kResultOk);mem.rewind();
    IBStreamer s(&mem,kLittleEndian);int32 a=0,b=0,d=0,m=0;
    REQUIRE(s.readInt32(a)&&s.readInt32(b)&&s.readInt32(d)&&s.readInt32(m));
    REQUIRE(a==0x50535955&&b==4&&d==0&&m==motion);
}
static constexpr int width=960,height=640;
struct View{
    Frame frame;
    IPlugView* view=nullptr;
    tresult attach(Vst::IEditController* c,int x,int y,int parentIndex=-1){
        if(!view)view=c->createView(Vst::ViewType::kEditor);REQUIRE(view);
        frame.detached=false;
        ViewRect initial;REQUIRE(view->getSize(&initial)==kResultOk);
        RECT rect{0,0,parentIndex<0?initial.getWidth():768,parentIndex<0?initial.getHeight():512};
        REQUIRE(AdjustWindowRectEx(&rect,WS_OVERLAPPEDWINDOW,FALSE,0));
        frame.window=CreateWindowExW(0,L"125A_NativeAuditWindow",L"125A native audit",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
            x,y,rect.right-rect.left,rect.bottom-rect.top,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        REQUIRE(frame.window);
        REQUIRE(view->setFrame(&frame)==kResultOk);
        ShowWindow(frame.window,SW_SHOWNORMAL);UpdateWindow(frame.window);
        const auto result=view->attached(frame.window,kPlatformTypeHWND);
        pump(150);return result;
    }
    void open(Vst::IEditController* c,int x,int y){REQUIRE(attach(c,x,y)==kResultOk);}
    void size(double scale=1.,bool attached=true){
        ViewRect v;RECT r{};
        const int w=static_cast<int>(std::floor(width*scale)),h=static_cast<int>(std::floor(height*scale));
        REQUIRE(view->getSize(&v)==kResultOk&&GetClientRect(frame.window,&r));
        if(v.getWidth()!=w||v.getHeight()!=h||r.right!=w||r.bottom!=h)
            std::fprintf(stderr,"geometry wanted %d x %d view %d x %d hwnd %ld x %ld\n",w,h,v.getWidth(),v.getHeight(),r.right,r.bottom);
        REQUIRE(v.getWidth()==w&&v.getHeight()==h&&r.right==w&&r.bottom==h);
        HWND child=GetWindow(frame.window,GW_CHILD);RECT childRect{};
        if(attached){REQUIRE(child&&GetClientRect(child,&childRect));REQUIRE(childRect.right==w&&childRect.bottom==h);}
        ViewRect constraint(0,0,1,1);REQUIRE(view->checkSizeConstraint(&constraint)==kResultTrue);
        REQUIRE(constraint.getWidth()==w&&constraint.getHeight()==h);
    }
    void detach(){
        if(!frame.detached){REQUIRE(view->removed()==kResultOk);REQUIRE(view->setFrame(nullptr)==kResultOk);}
        DestroyWindow(frame.window);frame.window=nullptr;pump(20);
    }
    void close(){detach();view->release();view=nullptr;}
    void discardFailedAttach(){
        REQUIRE(view->setFrame(nullptr)==kResultOk);
        DestroyWindow(frame.window);frame.window=nullptr;pump(20);
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
    recall(c,1);saved(c,1);
    // Actual fixed editor, no user/host drag resize. MOTION recall must
    // leave both detached and attached geometry unchanged.
    View a;
    a.view=c->createView(Vst::ViewType::kEditor);REQUIRE(a.view);
    REQUIRE(a.view->canResize()==kResultFalse);
    for(int motion:{0,1,0,1}){
        recall(c,motion);saved(c,motion);ViewRect rect{};
        REQUIRE(a.view->getSize(&rect)==kResultOk&&rect.getWidth()==width&&rect.getHeight()==height);
    }
    a.frame.accept=false;a.open(c,15,15);a.size();
    REQUIRE(a.frame.requests==0); // matching parent needs no resize
    SetForegroundWindow(a.frame.window);pump(150);
    auto motionOn=pixels(a.frame.window,364,603,30,30);
    recall(c,0);pump(180);saved(c,0);a.size();
    auto motionOff=pixels(a.frame.window,364,603,30,30);
    REQUIRE(delta(motionOn,motionOff)>.25);
    recall(c,1);pump(180);saved(c,1);a.size();
    REQUIRE(delta(motionOn,pixels(a.frame.window,364,603,30,30))<.5);
    for(int motion:{0,1,0,1}){recall(c,motion);a.size();saved(c,motion);}
    REQUIRE(a.frame.requests==0);
    ViewRect illegal(0,0,768,512);
    REQUIRE(a.view->onSize(&illegal)!=kResultOk);a.size();
    a.close();REQUIRE(a.frame.refs==1);
    std::puts("P030 fixed geometry, MOTION state recall, MOTION and stale onSize PASS");

    // Host DPI, including fractional factors and same-view reuse. The
    // parent is allocated from getSize and refuses all redundant resizes.
    for(float factor:{1.f,1.10f,1.25f,1.33f,1.5f,1.60f,1.75f,2.f,106.f/96.f,128.f/96.f}){
        View initial;initial.view=c->createView(Vst::ViewType::kEditor);REQUIRE(initial.view);
        FUnknownPtr<IPlugViewContentScaleSupport> dpi(initial.view);REQUIRE(dpi);
        REQUIRE(dpi->setContentScaleFactor(factor)==kResultOk);
        ViewRect expected{},constrained(0,0,1,1);
        REQUIRE(initial.view->getSize(&expected)==kResultOk);
        const int wantedW=static_cast<int>(std::floor(width*double(factor))),wantedH=static_cast<int>(std::floor(height*double(factor)));
        std::printf("P030 detached DPI %.9g expected %d x %d actual %d x %d\n",double(factor),wantedW,wantedH,expected.getWidth(),expected.getHeight());
        REQUIRE(expected.getWidth()==wantedW&&expected.getHeight()==wantedH);
        REQUIRE(initial.view->checkSizeConstraint(&constrained)==kResultTrue);
        REQUIRE(expected.getWidth()==constrained.getWidth()&&expected.getHeight()==constrained.getHeight());
        REQUIRE(dpi->setContentScaleFactor(0.f)!=kResultOk);
        recall(c,1);
        initial.frame.accept=false;initial.open(c,15,15);initial.size(factor);
        REQUIRE(initial.frame.requests==0);
        initial.detach();REQUIRE(initial.frame.refs==1);
        recall(c,1);initial.open(c,15,15);initial.size(factor);
        initial.frame.accept=true;
        for(float live:{1.25f,1.33f,1.f,factor}){
            std::printf("P030 live DPI %.9g\n",double(live));
            REQUIRE(dpi->setContentScaleFactor(live)==kResultOk);initial.size(live);
        }
        dpi=nullptr;initial.close();REQUIRE(initial.frame.refs==1);
    }
    std::puts("P030 ten DPI factors, fixed-parent attach/reopen and live scaling PASS");

    // Two independent views share MOTION, never window geometry.
    {View first,second;first.open(c,15,15);second.open(c,810,20);
     FUnknownPtr<IPlugViewContentScaleSupport> dpi(first.view);REQUIRE(dpi);
     REQUIRE(dpi->setContentScaleFactor(1.25f)==kResultOk);first.size(1.25);second.size();
     first.frame.accept=false;REQUIRE(dpi->setContentScaleFactor(1.5f)!=kResultOk);
     first.size(1.25);second.size();first.frame.accept=true;
     recall(c,1);first.size(1.25);second.size();saved(c,1);
     dpi=nullptr;first.close();second.close();REQUIRE(first.frame.refs==1&&second.frame.refs==1);}
    // Before attach, with a host already assigned: accept/refuse and nested
    // attach. A rejected reentrant DPI request must not mutate the view.
    for(int mode=0;mode<3;++mode){
        View initial;initial.view=c->createView(Vst::ViewType::kEditor);REQUIRE(initial.view);
        FUnknownPtr<IPlugViewContentScaleSupport> dpi(initial.view);REQUIRE(dpi);
        RECT rect{0,0,width,height};REQUIRE(AdjustWindowRectEx(&rect,WS_OVERLAPPEDWINDOW,FALSE,0));
        initial.frame.window=CreateWindowExW(0,L"125A_NativeAuditWindow",L"125A DPI",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
            15,15,rect.right-rect.left,rect.bottom-rect.top,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        REQUIRE(initial.frame.window);initial.frame.equalSizeShortcut=true;
        REQUIRE(initial.view->setFrame(&initial.frame)==kResultOk);
        if(mode==1)initial.frame.accept=false;
        if(mode==2)initial.frame.resizeOnce=[&](IPlugView* view,ViewRect* size) -> tresult {
            REQUIRE(dpi->setContentScaleFactor(1.5f)!=kResultOk);
            recall(c,1);
            REQUIRE(initial.frame.resizeView(view,size)==kResultOk);
            REQUIRE(view->attached(initial.frame.window,kPlatformTypeHWND)==kResultOk);
            initial.size(1.25);return kResultTrue;
        };
        const auto result=dpi->setContentScaleFactor(1.25f);
        REQUIRE((result==kResultOk)==(mode!=1));
        if(mode!=2){
            initial.size(mode==1?1.:1.25,false);
            initial.frame.accept=true;
            REQUIRE(initial.view->attached(initial.frame.window,kPlatformTypeHWND)==kResultOk);
        }
        initial.size(mode==1?1.:1.25);dpi=nullptr;initial.close();REQUIRE(initial.frame.refs==1);
    }
    std::puts("P030 early DPI accept/refuse/nested attach PASS");
    // Stale host parent that genuinely differs: preserve prior refusal and
    // retry semantics, without project-driven zoom.
    for(int mode=0;mode<4;++mode){
        View initial;initial.frame.accept=mode!=1;
        initial.frame.detachOnResize=mode>=2?mode-1:0;
        const auto result=initial.attach(c,15,15,0);
        if(mode==0){REQUIRE(result==kResultOk);initial.size();}
        else{
            REQUIRE(result!=kResultOk&&GetWindow(initial.frame.window,GW_CHILD)==nullptr);
            initial.discardFailedAttach();initial.frame.accept=true;
            initial.open(c,15,15);initial.size();
        }
        initial.close();REQUIRE(initial.frame.refs==1);
    }
    // Last-view repeated native lifetime checks.
    {View last;for(int cycle=0;cycle<24;++cycle){
        last.open(c,15,15);last.size();last.detach();REQUIRE(last.frame.refs==1);
     }last.view->release();last.view=nullptr;}
    std::puts("P030 stale parent accept/refuse/detach and 24 last-view cycles PASS");
    recall(c,1);
    a.frame.accept=true;a.open(c,15,15);a.size();saved(c,1);
    SetForegroundWindow(a.frame.window);
    std::puts("P028l native testing interrupted fade");
    PixelCapture capture(150,150);
    bool measured=false;
    for(int trial=0;trial<3&&!measured;++trial){
        world(c,kDepression);pump(1600);
        auto before=capture.grab(a.frame.window,404,238);
        auto fixed=pixels(a.frame.window,63,63,75,75);
        auto last=before;
        double maxStep=0.,retarget=0.;int samples=0,validRetarget=0;
        auto lastTime=Clock::now();const auto start=lastTime;
        auto switchTime=start;bool switched=false,bracketed=false;
        world(c,kMania);
        while(Clock::now()-start<std::chrono::milliseconds(1850)){
            if(!switched&&Clock::now()-start>=std::chrono::milliseconds(310)){
                switchTime=Clock::now();world(c,kPanic);switched=true;
            }
            pump(1);auto next=capture.grab(a.frame.window,404,238);auto now=Clock::now();
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
        REQUIRE(delta(before,last)>5.&&delta(fixed,pixels(a.frame.window,63,63,75,75))<.5);
        measured=samples>=15&&validRetarget>=2&&switched&&bracketed;
        std::printf("R04 trial %d max %.4f retarget %.4f short-gap=%d retarget=%d bracketed=%d valid=%d\n",
            trial,maxStep,retarget,samples,validRetarget,int(bracketed),int(measured));
    }
    REQUIRE(measured);
    a.close();saved(c,1);
    REQUIRE(c->terminate()==kResultOk);c->release();factory->release();REQUIRE(exit());FreeLibrary(dll);timeEndPeriod(1);CoUninitialize();
    std::puts("P030 native fixed geometry, DPI and R04/R07 actual DLL probes PASS");
    return 0;
}
