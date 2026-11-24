#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace psychiatry::vst3 {
// Stable component/controller IDs: never change once user projects exist.
static const Steinberg::FUID kProcessorCID(0x125A20D2, 0xA3D04F91, 0x9B1074B7, 0x0119CA26);
static const Steinberg::FUID kControllerCID(0x125A20D3, 0xA3D04F91, 0x9B1074B7, 0x0119CA26);

// Reserve macro IDs for the later nine-effect release. Do NOT expose inert controls.
enum Param : Steinberg::Vst::ParamID {
    kDepression=100, kParanoia=101, kSchizophrenia=102,
    kMania=103, kPanic=104, kDissociation=105, kObsession=106,
    kHallucination=107, kPsychosis=108,
    kDelirium=109, kCatatonia=110, kAmnesia=111,
    kTherapy=200, kBypass=999
};
static constexpr Steinberg::int32 kStateMagic=0x50535932; // 'PSY2'
static constexpr Steinberg::int32 kStateVersion=5;
} // namespace psychiatry::vst3
