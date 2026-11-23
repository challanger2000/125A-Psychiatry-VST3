# 125A PSYCHIATRY — Verified Project Status

Updated: **2026-11-23**. Latest audited Windows build: **P009 interactive diagnostic monitor / VST3 factory v0.9.0**, **NOT a product release**. **Only `main`**; do not create temporary development branches.

## Latest verified build and fixed references

- **Source commit tested:** `4473273ef9006d2800b11f89ac9f3092c9aa265c` on **main** (P009 monochrome diagnostic CRT, P009 click regression, fully bilingual labels and motion-off default).
- **Windows/MSVC x64 real VST3 workflow:** [run 45269301904](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301904), **SUCCESS**.
  - Steinberg VST3 SDK 3.8.1 validator: **47/47 PASS, 0 FAIL**.
  - Actual VST3 processor/controller state regression: **1/1 PASS** (historical state v1, v2, v3, v4, plus NaN, truncation and future-version rejection). **Not a DAW project reopen test**.
  - [P009 Windows VST3 bundle artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301904/artifacts/14864082290): `125A-PSYCHIATRY-P009-DiagnosticCRT-NOT-FINAL` (7-day retention, expires 2026-11-30).
- **Windows + Linux standalone audio workflow:** [run 45269301899](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301899), **SUCCESS**.
  - **6/6 CTest groups passed on each platform**. Includes all nine concurrent DSP engines and callback no-heap allocation test, plus precomputed Hilbert tap coefficient regression.
  - **P006 objective quality audit:** 120 sound/source/intensity cases, **0 review flags, 0 safety-level violations** after addressing measured issues.
  - [Quality audit CSV and synthetic WAV archive](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45207296078/artifacts/14834208034); metrics include wet/dry RMS ratio, dBFS peak, normalized delta, DC, maximum single-sample step, and runner-local render time.
- All Windows Actions artefacts are temporary engineering packages, not release installers.

## Actual audio code in the VST3

**All nine sound-world prototypes are implemented**: DEPRESSION, PARANOIA, SCHIZOPHRENIA, MANIA, PANIC, DISSOCIATION, OBSESSION, HALLUCINATION and PSYCHOSIS. Each is a dedicated compound DSP engine, not an inert parameter. Distortion is **one ingredient inside relevant effect worlds**, especially PSYCHOSIS, rather than the universal plugin core.

Processing order: DEPRESSION → SCHIZOPHRENIA → MANIA → PANIC → PARANOIA → DISSOCIATION → OBSESSION → HALLUCINATION → PSYCHOSIS → **one global inverse THERAPY Dry/Wet mix**.

- **P009 verified CRT interface:** Steinberg **VSTGUI**, dark 1004×570 panel, two EN/DE templates, nine live FX knobs, THERAPY, Bypass, LOSS OF CONTROL, percent readouts, language switch and original software-drawn **monochrome diagnostic CRT**. CRT scenes and cross-macro signal corruption are calculated on the UI thread from the actual controller parameters. No patient photos or third-party graphics are used.
- VST3 `createView(kEditor)` now constructs `VSTGUI::VST3Editor`, rather than returning null and falling back to host generic controls.
- **Official Steinberg EditorHost GUI interaction PASSED:** [run 45269301923](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301923). An actual Win32 GUI was created, a mouse click on LOSS OF CONTROL changed nine effect values and the CRT image (482 sampled pixels differed, RGB delta 81,702), and the subsequent DE language switch retained a working editor. [Three run screenshots](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301923/artifacts/14864592486) were visually inspected. **This is still not Studio Pro or an audio listening test.**
- Stereo/mono, 32-bit floating point input/output, deterministic state per instance; sample-rate dependent buffers allocated in preparation. CPU and realtime allocation *suite* exists, but actual end-user DAW latency/CPU stress still needed.
- VST3 factory version now **0.9.0**, actual component/controller IDs and ParamIDs unchanged. Project state **v4**, with decoding of v1/v2/v3.


## P009 — proven CRT implementation and honest scope

- **Custom view `PSYCHIATRY_DIAGNOSTIC`** is constructed through `VST3EditorDelegate::createCustomView` and VSTGUI `CView` drawing. Both GUI layouts embed the same 288×325 original drawing surface. Nine different institutional/diagnostic patterns are displayed depending on the **dominant live macro**; the remaining active macros add visible cross-effect interference.
- **Zero audio-thread work added.** Monitor samples the existing nine parameter values on the GUI idle callback, smooths transitions, and draws with no copied photographs or visual assets. **Actual audio amplitude or FFT data are not transmitted to the display.**
- **Actual GUI interaction test:** a Windows EditorHost-generated left click triggers the working LOSS OF CONTROL randomizer, the nine macro readouts visibly update and the CRT responds. In the final P009 reference, 482 sampled CRT pixels changed, summed RGB delta 81,702. Subsequent EN→DE view switch completed without window crash; screenshots are retained.
- **Accessibility:** full-screen strobing is deliberately absent; the **MOTION / BEWEGUNG** checkbox is off by default, so values update the screen without continuous movement. Enabled mode adds only gentle scan motion. The setting is persisted in controller-only UI state v2 with v1 state backward decoding. DSP component state remains v4.
- **Quality fixes revealed by screenshots and UI tests:** initially DEPRESSION could leave the base corridor largely unchanged after randomization despite correct knob movement, so an always-visible CRT intensity strip was added. The English/German templates now translate the diagnostic labels and shortened German THERAPIE / MIX label. The monitor was changed from green-tinted to true grayscale.
- **Verified commit:** `4473273ef9006d2800b11f89ac9f3092c9aa265c`. [Windows 47/47 API+1/1 SDK state](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301904); [Windows/Linux DSP suite](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301899); [real Win32 GUI/click test](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301923), all **SUCCESS**.
- [Complete P009 design and verification log](docs/P009.md). This is an early visual implementation and **not final historic photography**.

## P008 — implemented controls and real Windows evidence

- **LOSS OF CONTROL** is active in both VSTGUI templates. A momentary button on UI tag `9000` changes **exactly the nine macro ParamIDs 100–108**, not THERAPY (200), Bypass (999), project state format, or audio callbacks. There is an 18% per-world chance of turning off; otherwise it selects 12–78% effect amount.
- The actual VST3 controller calls `beginEdit/setParamNormalized/performEdit/endEdit` for all nine macro IDs, with `startGroupEdit/finishGroupEdit` for hosts supporting IComponentHandler2. A simulated host spy regression verifies 9×3 notifications and grouped undo.
- **EN/DE** uses two `.uidesc` templates and a momentary language button on private tag `9001`. This preference persists in **controller-only UI state** (`PSYU`), independent of processor project state v4; earlier projects default to English.
- **Percent readouts** use actual 0–100% values. Visual screenshot review caught cramped rows, light buttons with poor contrast and missing THERAPY pointer; corrected and re-tested.
- **Windows official validation:** [run 45269301904](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301904) **SUCCESS**, 47/47 official validator, 1/1 VST3 controller/legacy-state regression and packaged both bilingual GUIs.
- **Runtime Win32 proof:** [Steinberg EditorHost run 45262697188](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45262697188) **SUCCESS**, actual GUI window with HWND and captured screenshot; manually inspected. Does NOT prove operation within Studio Pro.
- [Detailed engineering notes](docs/P008.md). The first user test is **not requested yet**.

## Actual changes in P006

- The HALLUCINATION Hilbert stage now uses constant FIR coefficients rather than performing eight cosine calculations for every sample on every channel; a coefficient-equivalence regression test was added.
- Objective audit showed **MANIA at 85% amplified some test sources by +9 to +10 dB and crossed 0 dBFS**. Added gradual output gain correction independent of modulation speed/texture.
- Audit identified a near-inaudible **DEPRESSION** response at 20% on some sources; its macro curve now rises earlier. PANIC's marginal 20% response was strengthened with a smooth bounded curve.
- Further audit identified **PSYCHOSIS at 85% with +8.25 dB RMS on synthetic voice and +6.05 dB on atmosphere**; corrected with a gradual level compensation stage.
- The audit now **fails CI** on any measured >0 dBFS peak or >+6 dB unintentional gain rise on its reference sources. Final validated run: **0 flagged cases / 120**.
- Independent VST3 state test instantiates the actual processor and controller with SDK IBStream interfaces; verifies saved v1-v4 contents and invalid-state rejection.

[Full P006 method, findings and limitations](docs/P006.md).

## Still not implemented / NO-GO

- **The functional P009 monochrome vector CRT exists, but the final historic photographic GUI, photo/asset rights review and the full old-clinic atmosphere are not yet finished.**
- **LOSS OF CONTROL and DE/EN toggles are implemented**, but their physical click-to-audio behavior in Studio Pro has not yet been user-verified.
- **No real Studio Pro listening test or project save/reopen, no user listening acceptance, no real musical/vocal audio comparison.** The audit uses generated *proxies*, not recorded bass, guitar, drums and speech.
- **No production high-fidelity assurance** for wavefolder aliasing, spectral/Hilbert sidebands, grain clicks, transient smoothing, long automation and long-session CPU behavior. The VST3 47/47 tests do not validate subjective quality.
- No release installer or final VST3 product. Do not call P009 a finished product.

## Next engineering gates

1. **Next audio acceptance:** source-specific recordings (bass, guitar, drums, vocals, atmospheres), properly level-matched listening, anti-alias/spectral/CPU investigation and revision of weak FX where necessary. Real DAW testing **at the end** per project policy.
2. **Next final art phase:** establish rights-cleared original historic-clinic photographic motifs and richer worn-instrument styling for the already functioning monochrome CRT, keeping UI rendering off the audio thread; retain motion-off accessibility and bilingual labels.
3. **Final release QA:** actual Windows Studio Pro audible/protected realtime validation, project recall/automation/installer verification, final release readiness only on proof.

## Project rules

Keep **only `main`**, keep every verified last-tested commit and Actions links in this file; distinguish synthetic objective QA from actual audible approval. Release UI, packages and product metadata use the neutral **125A** brand; do not surface personal developer identity.
