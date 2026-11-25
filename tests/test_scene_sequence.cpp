#include "../vst3/scene_sequence.h"
#include <iostream>
#include <set>
using namespace psychiatry::vst3;
int main() {
    SceneSequence s;
    std::set<int> seen;
    if(s.current()!=0)return 1;
    for(int w=0;w<12;++w) {
        s.update(w,false,0.);
        const int first=s.current();
        for(int n=0;n<20;++n)s.update(w,false,1.);
        if(s.current()!=first)return 2;
        for(int v=0;v<kWorldSceneCounts[w];++v){
            if(!seen.insert(s.current()).second)return 3;
            int before=s.current();
            for(int n=0;n<10&&s.current()==before;++n)s.update(w,true,1.);
            if(s.current()==before)return 4;
        }
        if(s.current()!=first)return 5;
        s.update(-1,true,1.);
        if(s.current()!=0)return 6;
        s.update(w,false,0.);
        if(s.current()!=first)return 7;
    }
    if(seen.size()!=40||*seen.begin()!=1||*seen.rbegin()!=40)return 8;
    SceneSequence t;
    t.update(0,true,0.);
    for(int i=0;i<7;++i)t.update(0,true,1.);
    const int held=t.current();
    if(held==kWorldScenes[0][0])return 9;
    t.update(-1,true,1.);
    t.update(0,false,0.);
    if(t.current()!=held)return 10;
    std::cout<<"P028h: all 40 scenes reachable, no duplicate within cycle, static hold and standby/restore PASS\n";
}
