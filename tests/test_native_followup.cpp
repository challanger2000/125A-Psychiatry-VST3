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
    tresult attach(Vst::IEditController* c,int x,int y,int parentIndex=-1){
        if(!view)view=c->createView(Vst::ViewType::kEditor);REQUIRE(view);
        frame.detached=false;
        ViewRect initial;REQUIRE(view->getSize(&initial)==kResultOk);
        RECT rect{0,0,parentIndex<0?initial.getWidth():widths[parentIndex],parentIndex<0?initial.getHeight():heights[parentIndex]};
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
    void size(int index,int scale=1){
        ViewRect v;RECT r{};
        const int w=widths[index]*scale,h=heights[index]*scale;
        REQUIRE(view->getSize(&v)==kResultOk&&GetClientRect(frame.window,&r));
        if(v.getWidth()!=w||v.getHeight()!=h||r.right!=w||r.bottom!=h)
            std::fprintf(stderr,"geometry wanted %d x %d view %d x %d hwnd %ld x %ld\n",w,h,v.getWidth(),v.getHeight(),r.right,r.bottom);
        REQUIRE(v.getWidth()==w&&v.getHeight()==h&&r.right==w&&r.bottom==h);
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
    recall(c,1,0);saved(c,1,0);
    // A normal host starts with the advertised getSize() and may skip a
    // resize when it equals the requested size. Never publish the requested
    // geometry before asking the host to apply it.
    {View official;official.frame.equalSizeShortcut=true;official.open(c,15,15);
     official.size(0);recall(c,1,1);pump(60);official.size(1);official.close();}
    recall(c,1,0);
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
        // Reattach the SAME IPlugView, not a newly created editor.
        closing.detach();closing.open(c,15,15);closing.size(2);saved(c,1,2);
        recall(c,1,1);pump(100);closing.size(1);survivor.size(1);saved(c,1,1);
        closing.close();survivor.close();
        View reopened;reopened.open(c,15,15);reopened.size(1);reopened.close();
        std::printf("N02 native synchronous detach order %d with survivor/reopen PASS\n",order);
    }
    // Initial attach: stale 50% parent, recalled 100% preference.
    // Accept, refuse, and detach synchronously in both orders. A failed
    // attach must be retryable on the same view, without a ghost child.
    for(int mode=0;mode<4;++mode){
        recall(c,1,0);
        View initial;initial.view=c->createView(Vst::ViewType::kEditor);REQUIRE(initial.view);
        recall(c,1,2);
        initial.frame.accept=mode!=1;
        initial.frame.detachOnResize=mode>=2?mode-1:0;
        const int before=initial.frame.requests;
        const auto result=initial.attach(c,15,15,0);
        REQUIRE(initial.frame.requests==before+1);
        if(mode==0){REQUIRE(result==kResultOk);initial.size(2);}
        else{
            REQUIRE(result!=kResultOk);
            if(mode>=2)REQUIRE(initial.frame.detached);
            REQUIRE(GetWindow(initial.frame.window,GW_CHILD)==nullptr);
            saved(c,1,2);
            initial.discardFailedAttach();
            initial.frame.accept=true;
            REQUIRE(initial.attach(c,15,15,0)==kResultOk);initial.size(2);
        }
        recall(c,1,1);pump(100);initial.size(1);saved(c,1,1);
        initial.close();
        std::printf("N03 initial attach mode %d, same-view retry/next zoom PASS\n",mode);
    }
    // Supersede an initial attachment from *inside* the host resize
    // callback. Returning from the older transaction must not close the
    // replacement or overwrite its accepted rectangle.
    for(int order=1;order<=2;++order)for(int acceptOld=0;acceptOld<=1;++acceptOld){
        recall(c,1,0);
        View initial;initial.view=c->createView(Vst::ViewType::kEditor);REQUIRE(initial.view);
        recall(c,1,2);
        bool nested=false;
        initial.frame.resizeOnce=[&](IPlugView* view,ViewRect*){
            if(order==1){REQUIRE(view->setFrame(nullptr)==kResultOk);REQUIRE(view->removed()==kResultOk);}
            else{REQUIRE(view->removed()==kResultOk);REQUIRE(view->setFrame(nullptr)==kResultOk);}
            REQUIRE(GetWindow(initial.frame.window,GW_CHILD)==nullptr);
            REQUIRE(view->setFrame(&initial.frame)==kResultOk);
            REQUIRE(view->attached(initial.frame.window,kPlatformTypeHWND)==kResultOk);
            nested=true;initial.size(2);
            return acceptOld?kResultTrue:kResultFalse;
        };
        REQUIRE(initial.attach(c,15,15,0)!=kResultOk);
        REQUIRE(nested&&GetWindow(initial.frame.window,GW_CHILD)!=nullptr);
        initial.size(2);saved(c,1,2);
        recall(c,1,1);pump(100);initial.size(1);saved(c,1,1);
        initial.close();REQUIRE(initial.frame.refs==1);
        std::printf("P029 nested reattach order=%d outer-result=%d PASS\n",order,acceptOld);
    }
    // Recall while initial resize is in progress, with and without
    // another attached view. Do not need a second external recall.
    for(int multiple=0;multiple<=1;++multiple){
        recall(c,1,0);
        View survivor;if(multiple)survivor.open(c,810,20);
        View initial;initial.frame.resizeOnce=[&](IPlugView* view,ViewRect* size){
            recall(c,1,1);
            return initial.frame.resizeView(view,size);
        };
        initial.open(c,15,15);initial.size(1);saved(c,1,1);
        if(multiple){survivor.size(1);survivor.close();}
        recall(c,1,2);pump(60);initial.size(2);saved(c,1,2);
        initial.close();REQUIRE(initial.frame.refs==1);
        std::printf("P029 in-attach recall multiple=%d PASS\n",multiple);
    }
    // A later negotiation can itself be refused; abort without a ghost
    // window, retain the recalled preference, and allow same-view retry.
    recall(c,1,0);
    {View initial;initial.frame.resizeOnce=[&](IPlugView* view,ViewRect* size){
        recall(c,1,1);
        const auto result=initial.frame.resizeView(view,size);
        initial.frame.accept=false;return result;
     };
     REQUIRE(initial.attach(c,15,15,0)!=kResultOk);
     REQUIRE(GetWindow(initial.frame.window,GW_CHILD)==nullptr);saved(c,1,1);
     initial.discardFailedAttach();initial.frame.accept=true;
     initial.open(c,15,15);initial.size(1);initial.close();REQUIRE(initial.frame.refs==1);}
    // Recall during an already-attached editor's resize: the previous
    // applyingUi_ early return silently discarded the later zoom request.
    // Test latest-wins, cancellation back to in-flight target, and host
    // refusal of the follow-up, with one or two native editors.
    for(int multiple=0;multiple<=1;++multiple)for(int mode=0;mode<3;++mode){
        recall(c,1,0);
        View first,second;first.open(c,15,15);if(multiple)second.open(c,810,20);
        first.frame.resizeOnce=[&](IPlugView* view,ViewRect* size){
            recall(c,1,3);recall(c,1,mode==1?2:1);
            const auto result=first.frame.resizeView(view,size);
            if(mode==2)first.frame.accept=false;
            return result;
        };
        if(mode==2&&multiple){
            second.frame.resizeOnce=[&](IPlugView* view,ViewRect* size){
                const auto result=second.frame.resizeView(view,size);
                second.frame.accept=false;return result;
            };
        }
        recall(c,1,2);pump(80);
        const int expected=mode==0?1:2;
        first.size(expected);saved(c,1,expected);
        if(multiple)second.size(expected);
        first.frame.accept=true;second.frame.accept=true;
        recall(c,1,0);pump(80);first.size(0);saved(c,1,0);
        if(multiple){second.size(0);second.close();REQUIRE(second.frame.refs==1);}
        first.close();REQUIRE(first.frame.refs==1);
        std::printf("P029a live-resize recall multiple=%d mode=%d PASS\n",multiple,mode);
    }
    // DPI and zoom must be one serialized geometry transaction.
    // Test both directions of callback nesting, including DPI refusal.
    for(int mode=0;mode<3;++mode){
        recall(c,1,0);
        View first;first.open(c,15,15);
        FUnknownPtr<IPlugViewContentScaleSupport> dpi(first.view);REQUIRE(dpi);
        if(mode==0){
            first.frame.resizeOnce=[&](IPlugView* view,ViewRect* size){
                recall(c,1,1);return first.frame.resizeView(view,size);
            };
            REQUIRE(dpi->setContentScaleFactor(2.f)==kResultOk);
            first.size(1,2);saved(c,1,1);
        }else if(mode==1){
            first.frame.resizeOnce=[&](IPlugView* view,ViewRect* size){
                REQUIRE(dpi->setContentScaleFactor(2.f)==kResultOk);
                return first.frame.resizeView(view,size);
            };
            recall(c,1,1);first.size(1,2);saved(c,1,1);
        }else{
            first.frame.accept=false;
            REQUIRE(dpi->setContentScaleFactor(2.f)!=kResultOk);
            first.size(0);saved(c,1,0);first.frame.accept=true;
            REQUIRE(dpi->setContentScaleFactor(2.f)==kResultOk);first.size(0,2);
        }
        recall(c,1,2);first.size(2,2);saved(c,1,2);
        REQUIRE(dpi->setContentScaleFactor(1.f)==kResultOk);first.size(2);
        dpi=nullptr;first.close();REQUIRE(first.frame.refs==1);
        std::printf("P029b DPI/zoom mode=%d PASS\n",mode);
    }
    // A registration made inside another view's resize callback must
    // receive the committed zoom after that callback completes.
    for(int reuse=0;reuse<=1;++reuse)for(int acceptOld=0;acceptOld<=1;++acceptOld){
        recall(c,1,0);
        View first,second;first.open(c,15,15);
        if(reuse)second.open(c,810,20);
        first.frame.resizeOnce=[&](IPlugView* view,ViewRect* size){
            if(reuse)second.detach();
            second.open(c,810,20);
            return acceptOld?first.frame.resizeView(view,size):kResultFalse;
        };
        recall(c,1,2);
        first.size(acceptOld?2:0);
        second.size(acceptOld?2:0);
        // If the old first view refused but an old second attachment
        // was superseded, neither old registration accepted the request.
        // The new registration follows the committed old zoom instead.
        saved(c,1,acceptOld?2:0);
        recall(c,1,1);first.size(1);second.size(1);saved(c,1,1);
        first.close();second.close();REQUIRE(first.frame.refs==1&&second.frame.refs==1);
        std::printf("P029b nested registration reuse=%d accept=%d PASS\n",reuse,acceptOld);
    }
    // Repeated destruction of the last native editor (Win32 scope).
    recall(c,1,0);
    {View last;for(int cycle=0;cycle<24;++cycle){
        last.open(c,15,15);last.size(0);last.detach();REQUIRE(last.frame.refs==1);
     }last.view->release();last.view=nullptr;
     std::puts("P029b last-view Win32 reopen 24 cycles PASS");}
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
