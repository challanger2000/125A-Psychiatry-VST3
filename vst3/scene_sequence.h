#pragma once
#include <algorithm>
#include <array>
#include <cstddef>

namespace psychiatry::vst3 {
// P028h manifest order. Index zero is the existing stationary standby.
inline constexpr std::array<const char*,41> kSceneFiles{
    "scene-corridor-bw.png",
    "scene-p028h-01-depression-isolation.png",
    "scene-p028h-02-panic-medical-room.png",
    "scene-p028h-03-dissociation-reflection.png",
    "scene-p028h-04-mania-scribbled-walls.png",
    "scene-p028h-05-corridor-wheelchair.png",
    "scene-p028h-06-padded-room-bed.png",
    "scene-p028h-07-treatment-room-straitjacket.png",
    "scene-p028h-08-eeg-mannequin.png",
    "scene-p028h-09-empty-ward-bed.png",
    "scene-p028h-10-mania-motion-blur.png",
    "scene-p028h-11-paranoia-observer.png",
    "scene-p028h-12-isolation-padded-room.png",
    "scene-p028h-13-fragmented-reflections.png",
    "scene-p028h-14-hallucination-double-exposure.png",
    "scene-p028h-15-corridor-straitjacket.png",
    "scene-p028h-16-hydrotherapy-tub.png",
    "scene-p028h-17-treatment-room-shadow.png",
    "scene-p028h-18-electrotherapy-console.png",
    "scene-p028h-19-historical-treatment-nurse.png",
    "scene-p028h-20-historical-restraint-chair.png",
    "scene-p028h-21-empty-electrotherapy-chair.png",
    "scene-p028h-22-historical-electrode-portrait.png",
    "scene-p028h-23-historical-treatment-seated.png",
    "scene-p028h-24-historical-doctor-and-nurse.png",
    "scene-p028h-25-ward-hanging-straitjacket.png",
    "scene-p028h-26-patient-portrait-young.png",
    "scene-p028h-27-matron-with-keys.png",
    "scene-p028h-28-open-medicine-cabinet.png",
    "scene-p028h-29-patient-records-archive.png",
    "scene-p028h-30-antique-instruments.png",
    "scene-p028h-31-abandoned-common-room.png",
    "scene-p028h-32-institution-staircase.png",
    "scene-p028h-33-basement-corridor.png",
    "scene-p028h-34-long-ward.png",
    "scene-p028h-35-double-exposure-portrait.png",
    "scene-p028h-36-elderly-patient-portrait.png",
    "scene-p028h-37-nurse-portrait.png",
    "scene-p028h-38-neurological-instruments.png",
    "scene-p028h-39-closed-medicine-cabinet.png",
    "scene-p028h-40-disordered-common-room.png"
};
inline constexpr std::array<std::array<int,4>,12> kWorldScenes{{
    {{1,31,26,36}},
    {{11,27,33,37}},
    {{8,32,22,0}},
    {{4,40,10,0}},
    {{2,38,20,0}},
    {{3,35,13,0}},
    {{28,30,39,0}},
    {{14,5,17,0}},
    {{7,19,15,24}},
    {{16,23,18,0}},
    {{6,25,12,21}},
    {{29,9,34,0}}
}};
inline constexpr std::array<int,12> kWorldSceneCounts{4,4,3,3,3,3,3,3,4,3,4,3};

class SceneSequence {
public:
    int forWorld(int world) const noexcept {
        const auto w=static_cast<std::size_t>(std::clamp(world,0,11));
        return kWorldScenes[w][cursor_[w]];
    }
    void update(int activeWorld,bool motion,double seconds) noexcept {
        if(activeWorld!=world_) {world_=activeWorld; elapsed_=0.;}
        if(!motion||world_<0) {elapsed_=0.;return;}
        elapsed_+=std::clamp(seconds,0.,1.);
        // Deliberate, non-repetitive rotation; no rapid cuts or random repeats.
        const auto w=static_cast<std::size_t>(world_);
        const double dwell=7.+double((world_+cursor_[w])%3);
        if(elapsed_>=dwell) {
            elapsed_=0.;
            cursor_[w]=(cursor_[w]+1)%kWorldSceneCounts[w];
        }
    }
    int current() const noexcept {return world_<0?0:forWorld(world_);}
private:
    std::array<int,12> cursor_{};
    int world_=-1;
    double elapsed_=0.;
};
}
