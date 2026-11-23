# 125A PSYCHIATRY

**Experimental dark multi-effect for Gothic, Darkwave, EBM, Industrial and cinematic horror sound design.**

> **Current engineering checkpoint: P007 preliminary VSTGUI (factory version 0.6.0), Windows x64 VST3.** Nine FX engines, automatable parameters and a dedicated dark VSTGUI with ten knobs (nine FX plus THERAPY), numeric readouts and Bypass are present. Latest Windows build passes **47/47 Steinberg checks**, legacy-state test **1/1**, plus packaged GUI binding verification. **Not a finished product and no real DAW GUI-open/listening sign-off yet.**

## Nine equally important effect worlds

| EN | DE | Character |
|---|---|---|
| DEPRESSION | DEPRESSION | Suffocating dynamics, dark filters and decaying room |
| PARANOIA | PARANOIA | Event-driven reversals, unsettling moving feedback |
| SCHIZOPHRENIA | SCHIZOPHRENIE | Diverging grain-pitched voices and spectral clashes |
| MANIA | MANIE | Accelerating rhythmic modulation and restless movement |
| PANIC | PANIK | Input-triggered jolts, metallic resonances and tremor |
| DISSOCIATION | DISSOZIATION | Reverse-grain memory and time-smearing |
| OBSESSION | ZWANG | Micro-loop fixation, stutter and digital decay |
| HALLUCINATION | HALLUZINATION | Input-derived ghost harmonics and frequency shifting |
| PSYCHOSIS | PSYCHOSE | One dedicated multimode nonlinear/industrial sound world |

Distortion is **not** the central effect of the entire product. Each macro contains a distinctive compound audio chain. **0% = neutral**. Multiple macros can act on the same audio simultaneously.

**THERAPY / THERAPIE** is an inverted global Dry/Wet: 0% is fully processed; 100% is completely dry. VST3-native Bypass is also available.

**LOSS OF CONTROL / KONTROLLVERLUST** (randomization of the nine FX macros while THERAPY remains unchanged) and **DE/EN switching** are planned, **not implemented**. The current prototype now has a **provisional functional VSTGUI editor**, not the final historic one.

## P006 verified build and audio-quality gate

- [Windows P006 version 0.6.0 VST3 build](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296557) — **Steinberg validator 47/47 PASS**, plus the v1–v4 state regression **PASS**.
- [Windows P006 VST3 development artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296557/artifacts/14834272974) — temporary ZIP, **NOT a finished release**.
- [P006 DSP test run on Windows/Linux](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296078) — **6/6 CTest groups passed**, separately from the VST3 SDK checks.
- [P006 quality report CSV + WAV comparison package](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296078/artifacts/14834208034) — 120 synthetic source/macro/intensity combinations; **0 reported review flags and 0 measured safety-level violations** under the defined audit thresholds.
- [Technical P006 findings, fixes, test limitations and next gate](docs/P006.md).
- [Always-current project status](STATUS.md).

The 120 audit cases use **synthetic** bass, drum, voice-like and atmosphere proxies. No claim is made that automated measurements prove musical quality, that the plugin reproduces psychiatric symptoms, or that project recall works in a real Studio Pro session.

## P007 provisional GUI for first eventual user test

- [Verified P007 Windows GUI build (Actions run)](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45212735622)
- [P007 Windows VST3 preview artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45212735622/artifacts/14836343370) — engineering build only; **do not begin user testing yet**.
- Resource: [resource/psychiatry.uidesc](resource/psychiatry.uidesc) — a temporary 680×550 dark VSTGUI, nine real macro knobs plus THERAPY and Bypass.
- VST3 editor interface: [vst3/controller.cpp](vst3/controller.cpp) uses official VSTGUI 4's VST3Editor; controls attach to host automatable parameter IDs.
- The Windows build checks the packaged UI definition and all eleven control bindings. The editor has **not yet been visually inspected inside Studio Pro**.
- LOSS OF CONTROL and DE/EN remain unimplemented and are **not misrepresented as working GUI buttons**.

**User acceptance rule:** Do not ask the user to test with only the host's generic knobs; at minimum a functional provisional VSTGUI must be present.

## Historical visual direction

The eventual GUI should resemble a disturbing old clinical diagnostic instrument: worn metal, analog meters, institutional imagery, documentary-style monochrome photographs and a damaged black-and-white monitor. **Each macro drives a corresponding visual world**, and active macros may mix those behaviors. Imagery must be licensed, historically appropriate and respectful of real patients. The audiovisual display must remain outside the real-time audio callback and offer reduced-motion controls. **No historical images or custom GUI are bundled yet.**

## Development

- Windows x64 VST3, C++17, Visual Studio 2022, Steinberg VST3 SDK 3.8.1 pinned at commit `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96`.
- **One branch: `main`. Stable class IDs and parameter IDs across versions. VST3 project state v4 restores earlier v1/v2/v3 layouts; dedicated SDK-facing regression checks exist.
- The audio engine is in `src/`; VST3 integration in `vst3/`; standalone tests in `tests/`; repeatable metrics and WAV examples in `tools/`; engineering history in `docs/`.
- To run standalone DSP checks locally:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

See [docs/P002.md](docs/P002.md) for the Windows Steinberg SDK build setup.

**Release is NO-GO:** no finished historical GUI, no LOSS OF CONTROL, no DE/EN UI, no historic image license package, no real Studio Pro audible/project recall sign-off, no completed high-fidelity spectral/aliasing and CPU acceptance. A green build is never a substitute for listening.

© 125A. Final license to be determined before redistribution.
