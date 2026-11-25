#include "ids.h"
#include "processor.h"
#include "controller.h"
#include "public.sdk/source/main/pluginfactory.h"

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace psychiatry::vst3;

BEGIN_FACTORY_DEF("125A", "https://github.com", "")
    DEF_CLASS2(INLINE_UID_FROM_FUID(kProcessorCID), PClassInfo::kManyInstances,
               kVstAudioEffectClass, "125A PSYCHIATRY", Vst::kDistributable,
               "Fx", "0.9.26", kVstVersionString, Processor::createInstance)
    DEF_CLASS2(INLINE_UID_FROM_FUID(kControllerCID), PClassInfo::kManyInstances,
               kVstComponentControllerClass, "125A PSYCHIATRY Controller", 0,
               "", "0.9.26", kVstVersionString, Controller::createInstance)
END_FACTORY
