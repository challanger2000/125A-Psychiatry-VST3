# 125A PSYCHIATRY

**Experimental dark multi-effect for Gothic, Darkwave, EBM, Industrial and cinematic horror sound design.**

> **Current engineering checkpoint: P009 / v0.9.0**, Windows x64 VST3. Nine real FX macros, inverted THERAPY, Bypass, LOSS OF CONTROL, EN/DE interface and a **functional nine-scene monochrome clinical CRT** controlled by the same nine macro parameter values as the audio engines. The official **Steinberg EditorHost opened the Windows editor and actual mouse-click tests verified that the randomized knobs change the CRT**. Current Windows build: **47/47 Steinberg tests PASS**. **Not the final photographic GUI and not yet approved in Studio Pro.**

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

**LOSS OF CONTROL / KONTROLLVERLUST** now changes the nine host-automatable macro settings as a single grouped automation gesture where supported. It never modifies THERAPY or Bypass. The **DE/EN toggle** switches between two actual UI templates and saves the language independently of sound state. This is a **functional provisional VSTGUI**, not the final historic visual concept.

## P006 verified build and audio-quality gate

- [Windows P006 version 0.6.0 VST3 build](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296557) — **Steinberg validator 47/47 PASS**, plus the v1–v4 state regression **PASS**.
- [Windows P006 VST3 development artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296557/artifacts/14834272974) — temporary ZIP, **NOT a finished release**.
- [P006 DSP test run on Windows/Linux](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296078) — **6/6 CTest groups passed**, separately from the VST3 SDK checks.
- [P006 quality report CSV + WAV comparison package](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296078/artifacts/14834208034) — 120 synthetic source/macro/intensity combinations; **0 reported review flags and 0 measured safety-level violations** under the defined audit thresholds.
- [Technical P006 findings, fixes, test limitations and next gate](docs/P006.md).
- [Always-current project status](STATUS.md).

The 120 audit cases use **synthetic** bass, drum, voice-like and atmosphere proxies. No claim is made that automated measurements prove musical quality, that the plugin reproduces psychiatric symptoms, or that project recall works in a real Studio Pro session.

## P009 — Reactive monochrome clinical CRT (working prototype)

- **Source commit `4473273ef9006d2800b11f89ac9f3092c9aa265c` tested on main.**
- [Windows VST3, Steinberg 47/47 PASS](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301904) — [P009 NOT-FINAL engineering artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301904/artifacts/14864082290).
- [Real Win32 EditorHost **click** regression](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301923) **PASS**, proving LOSS OF CONTROL changes the visible CRT and switching EN→DE keeps the editor alive.
- [Actual English/German EditorHost screenshots](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301923/artifacts/14864592486): original monochrome CRT, nine original signal diagrams, live effect-intensity meter, correct dual-language captions and an optional MOTION/BEWEGUNG checkbox.
- [Windows/Linux DSP regression](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301899): SUCCESS, original nine audio algorithms unchanged.
- **Motion is OFF by default.** Motion preference and UI language are saved in controller UI state **v2**, compatible with v1, while VST3 DSP project state remains **v4**, backward compatible with v1–v3.
- No original or third-party photographs are embedded: the **vector-generated screens are a legal-clearance-safe precursor**, not the final archival/horror image library. Display data are macro settings, not an input-audio waveform.

[Technical audit and remaining blockers](docs/P009.md) · [Current status](STATUS.md).

**The first user test in Studio Pro is still deferred until the remaining music-quality and final host tests.**

## P008 — Verified provisional bilingual VSTGUI

- [Official Windows VST3 build (47/47 Steinberg PASS)](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45262697238).
- [P008 Windows development VST3 artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45262697238/artifacts/14861920364) — **NOT FINAL; user testing is not requested yet**.
- [Official Steinberg EditorHost runtime smoke run](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45262697188) — **PASS**: actual Windows window `Editor` opened with the bundled `.vst3`.
- [Windows EditorHost screenshot artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45262697188/artifacts/14861312737). Screenshot was reviewed: button contrast, macro spacing, indicator visibility and 0–100% labels are now corrected in P008.
- [Windows/Linux DSP suite](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45262697261) — **PASS**, no change to sound engine.
- All eleven host controls are real; randomization uses nine VST3 begin/perform/end edits in one grouped host gesture when available. German and English controls are implemented; language preference is saved separately from the DSP's stable v4 project format.
- [P008 technical notes](docs/P008.md), [historic P007 checkpoint](docs/P007.md) and [current status](STATUS.md).

**The first user test remains deferred until the remaining engineering quality and final release checks.** A successful EditorHost window does not replace a Studio Pro test.

## Historical visual direction

The eventual GUI should resemble a disturbing old clinical diagnostic instrument: worn metal, analog meters, institutional imagery, documentary-style monochrome photographs and a damaged black-and-white monitor. **Each macro drives a corresponding visual world**, and active macros may mix those behaviors. Imagery must be licensed, historically appropriate and respectful of real patients. The current P009 CRT is entirely generated in the UI thread from the real macro values and has reduced-motion controls; **no photographic archive assets are bundled yet**, and the final vintage-hardware appearance is still in development.

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

**Release is NO-GO:** no finished historical GUI, no historic image license package, no real Studio Pro audible/project recall sign-off, no completed high-fidelity spectral/aliasing and CPU acceptance. A green build is never a substitute for listening.

© 125A. Final license to be determined before redistribution.
