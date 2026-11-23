# 125A PSYCHIATRY — Verified Project Status

Updated: **2026-11-23**. Latest audited Windows build: **P007 preliminary VSTGUI / VST3 factory v0.6.0**, **NOT a product release**. **Only `main`**; do not create temporary development branches.

## Latest verified build and fixed references

- **Source commit tested:** `d39f3faa68b101531954463d4100fe8f8a2237ba` (P007 temporary VSTGUI, VST3 factory currently 0.6.0).
- **Windows/MSVC x64 real VST3 workflow:** [run 45212735622](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45212735622), **SUCCESS**.
  - Steinberg VST3 SDK 3.8.1 validator: **47/47 PASS, 0 FAIL**.
  - Actual VST3 processor/controller state regression: **1/1 PASS** (historical state v1, v2, v3, v4, plus NaN, truncation and future-version rejection). **Not a DAW project reopen test**.
  - [P006 Windows VST3 bundle artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45212735622/artifacts/14836343370): `125A-PSYCHIATRY-P007-Preview-VSTGUI-NOT-FINAL` (7-day retention, expires 2026-11-30).
- **Windows + Linux standalone audio workflow:** [run 45207296078](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296078), **SUCCESS**.
  - **6/6 CTest groups passed on each platform**. Includes all nine concurrent DSP engines and callback no-heap allocation test, plus precomputed Hilbert tap coefficient regression.
  - **P006 objective quality audit:** 120 sound/source/intensity cases, **0 review flags, 0 safety-level violations** after addressing measured issues.
  - [Quality audit CSV and synthetic WAV archive](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296078/artifacts/14834208034); metrics include wet/dry RMS ratio, dBFS peak, normalized delta, DC, maximum single-sample step, and runner-local render time.
- All Windows Actions artefacts are temporary engineering packages, not release installers.

## Actual audio code in the VST3

**All nine sound-world prototypes are implemented**: DEPRESSION, PARANOIA, SCHIZOPHRENIA, MANIA, PANIC, DISSOCIATION, OBSESSION, HALLUCINATION and PSYCHOSIS. Each is a dedicated compound DSP engine, not an inert parameter. Distortion is **one ingredient inside relevant effect worlds**, especially PSYCHOSIS, rather than the universal plugin core.

Processing order: DEPRESSION → SCHIZOPHRENIA → MANIA → PANIC → PARANOIA → DISSOCIATION → OBSESSION → HALLUCINATION → PSYCHOSIS → **one global inverse THERAPY Dry/Wet mix**.

- **P007 verified temporary GUI:** actual Steinberg **VSTGUI** editor backed by `resource/psychiatry.uidesc`, displayed as a dark 680×550 preliminary front panel. Nine bound knobs, one bound THERAPY knob, numeric displays and bound native Bypass checkbox. The XML and all eleven control bindings were verified packaged **inside the .vst3 bundle** on Windows Actions.
- VST3 `createView(kEditor)` now constructs `VSTGUI::VST3Editor`, rather than returning null and falling back to host generic controls.
- **No physical Windows DAW editor-open test yet**. CMake compilation + XML/packaging verification do not substitute for observing a GUI actually open in Studio Pro.
- Stereo/mono, 32-bit floating point input/output, deterministic state per instance; sample-rate dependent buffers allocated in preparation. CPU and realtime allocation *suite* exists, but actual end-user DAW latency/CPU stress still needed.
- VST3 factory version now **0.6.0**, actual component/controller IDs and ParamIDs unchanged. Project state **v4**, with decoding of v1/v2/v3.

## P007 preliminary VSTGUI: user test prerequisite

- The first user test is **not requested until a real VSTGUI is available and internally verified**. Do not ask the user to assess the effect using Studio Pro's generic parameter view.
- Implemented a preliminary VSTGUI with a consistently sized nine-knob grid, one inverted THERAPY control, readable values and Bypass. The temporary look is dark metal/clinical-inspired, not the definitive historic reactive monitor.
- No inert LOSS OF CONTROL button has been exposed; the interface deliberately marks the planned feature as not available, and DE/EN switching remains a later product task.
- [Verified P007 Windows build](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45212735622): 47/47 Steinberg checks; VST3 state-regression 1/1; exact GUI XML resource and all eleven tags packaged in the Windows VST3.
- This check **does not** claim an editor-on-screen test in Studio Pro; user audition remains deferred.

## Actual changes in P006

- The HALLUCINATION Hilbert stage now uses constant FIR coefficients rather than performing eight cosine calculations for every sample on every channel; a coefficient-equivalence regression test was added.
- Objective audit showed **MANIA at 85% amplified some test sources by +9 to +10 dB and crossed 0 dBFS**. Added gradual output gain correction independent of modulation speed/texture.
- Audit identified a near-inaudible **DEPRESSION** response at 20% on some sources; its macro curve now rises earlier. PANIC's marginal 20% response was strengthened with a smooth bounded curve.
- Further audit identified **PSYCHOSIS at 85% with +8.25 dB RMS on synthetic voice and +6.05 dB on atmosphere**; corrected with a gradual level compensation stage.
- The audit now **fails CI** on any measured >0 dBFS peak or >+6 dB unintentional gain rise on its reference sources. Final validated run: **0 flagged cases / 120**.
- Independent VST3 state test instantiates the actual processor and controller with SDK IBStream interfaces; verifies saved v1-v4 contents and invalid-state rejection.

[Full P006 method, findings and limitations](docs/P006.md).

## Still not implemented / NO-GO

- **A temporary VSTGUI exists, but no final historic psychiatric equipment design, dynamic monochrome photo/video monitor, approved image rights or asset provenance.**
- **LOSS OF CONTROL** randomizes nine macro parameters but has **not been coded**. **DE/EN UI language toggle not coded**.
- **No real Studio Pro listening test or project save/reopen, no user listening acceptance, no real musical/vocal audio comparison.** The audit uses generated *proxies*, not recorded bass, guitar, drums and speech.
- **No production high-fidelity assurance** for wavefolder aliasing, spectral/Hilbert sidebands, grain clicks, transient smoothing, long automation and long-session CPU behavior. The VST3 47/47 tests do not validate subjective quality.
- No release installer or final VST3 product. Do not call P007 or P006 a finished product.

## Next engineering gates

1. **Next audio acceptance:** source-specific recordings (bass, guitar, drums, vocals, atmospheres), properly level-matched listening, anti-alias/spectral/CPU investigation and revision of weak FX where necessary. Real DAW testing **at the end** per project policy.
2. **P008 final custom GUI:** dark historical psychiatric examination apparatus, nine thematic image pools, responsive small damaged monochrome monitor, no work in audio callback, accessibility/reduced motion, verified asset licensing, DE/EN labels, LOSS OF CONTROL.
3. **P009 final QA:** actual Windows Studio Pro audible/protected realtime validation, project recall/automation/installer verification, final release readiness only on proof.

## Project rules

Keep **only `main`**, keep every verified last-tested commit and Actions links in this file; distinguish synthetic objective QA from actual audible approval. Release UI, packages and product metadata use the neutral **125A** brand; do not surface personal developer identity.
