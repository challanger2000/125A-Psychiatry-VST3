# Change history — 125A PSYCHIATRY

Only the `main` branch is used. The most recent **verified code commit** and test links are always in [STATUS.md](STATUS.md).

## 0.9.10 — P022 (2026-11-24)

- Monitor follows actual macro intensity, THERAPY, Bypass; truly static reduced-motion mode. Real bypass/restore screenshot proof 2340/240/2340.
- Branded x64 bundle and upstream license notices verified. [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45306570917), [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45306570973), [GUI](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45306571192) SUCCESS. No DSP changes.

## P021 — stress verification (factory 0.9.9)

- [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45305211996) and [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45305212003) passed. Six actual processor renders with dense automation, silent input, native bypass and exact compensated THERAPY; bit-identical results across buffer layouts at each of three rates. No DSP change.

## 0.9.9 — P020 (2026-11-24)

- OBSESSION complete grain capture, immutable repeat lifetime, sample-rate-scaled wet-only fade, exact wet endpoint.
- [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45300506720), [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45300506694), [GUI](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45300506798) SUCCESS; all existing gates and three-rate frozen loop test pass.

## 0.9.8 — P019 (2026-11-24)

- Corrected SCHIZOPHRENIA startup, DISSOCIATION capture isolation, PANIC oscillator; atmosphere full-wet gain −7.56 dB.
- [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45297895778), [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45297895678), [GUI](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45297895606) SUCCESS; all prior guards and new actual processor mono/stereo, variable-block, in-place, isolation and restart tests pass.

## 0.9.7 — P018 (2026-11-24)

- Linear-phase FIR in four nonlinear worlds, matched per-stage and global mix delay, fixed 96-sample host PDC, input-time parameter event alignment.
- Worst observed 50% cancellation improved −23.55 to −2.28 dB; three other oversampled worlds no worse than −0.47 dB. All prior alias, automation and quality gates pass.
- [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45295639573), [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45295639454), [GUI](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45295639417) SUCCESS; validator 47/47, 7/7 DSP groups per platform, 0/120 quality flags.

## 0.9.6 — P017 (2026-11-24)

- All nine macro and THERAPY smoothers settle to exact 0/100% without abrupt movement.
- 54 automated wet-to-dry endpoint cases at three rates passed; [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45293157519), [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45293157516) SUCCESS, Steinberg 47/47.
- P018 phase alignment still required.

## 0.9.5 — P016 (2026-11-24)

- Preserve raw finite samples at PARANOIA/THERAPY exact endpoints; restore the selected RNG seed at reset.
- [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465707) and [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465506): SUCCESS. 144 phase and 24 impulse measurements; exact THERAPY half-sum tested. Worst observed cancellation −23.55 dB; phase correction remains open. No release or Studio Pro sign-off.

## P015 — actual VST3 multirate and two-tone alias audit (2026-11-24; factory remains 0.9.4)

- Real `Processor::process` now probes **44.1/48/96 kHz** with 14+15-kHz two-tone input. All nine macro 0% Dry and inverse THERAPY 100% Dry endpoints are bit-identical at each rate; all 27 full-Wet outputs are finite and below 4.0 peak.
- Four nonlinear macros contribute **32 folded third-order two-tone lines** at 44.1/48 kHz. Worst measured is **−52.18 dB relative to input**; an explicit **−45 dB(input)** CI gate guards all 32. At 96 kHz 42–45-kHz products are correctly classified as in-band, **not** aliases.
- [Final Windows VST3 run 45288772417](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772417): **SUCCESS**, Steinberg 47/47, existing P010 180-row audio and sample automation PASS, P015 75-row export PASS. [Windows/Linux DSP run 45288772568](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772568): **SUCCESS**.
- Pure testing/documentation extension: **no DSP rework, no changed plugin IDs, no factory version change**. This does not certify all other nonlinearities, transient/phase correctness, real-world listening or Studio Pro recall.

## 0.9.4 — P014 measured PANIC alias reduction (2026-11-23)

- Oversampled PANIC's actual wet-only excitation/tanh nonlinear branch with the reusable 2× streamed sixth-order Butterworth interpolator and decimator. Existing trigger and transient/impulse behavior preserved; 0% Dry, 100% wet and inverse THERAPY unchanged.
- Actual Windows-VST3 15-kHz→3-kHz folded alias improved from **−31.87 to −69.35 dBc** (**37.48 dB**); 14-kHz→6-kHz from **−31.86 to −73.41 dBc** (**41.55 dB**). Strict **−45 dBc PANIC alias threshold** added for both probes.
- [Windows 45287094009](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094009): **SUCCESS**, Steinberg **47/47**, full-Wet 45/45, 180 audio measurements, inverse THERAPY, gain-placebo negative, sample-accurate automation; [Windows/Linux 45287094101](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094101) DSP suite 6/6 each OS, 0/120 flags; [Win32 EditorHost 45287093826](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287093826) GUI PASS.
- All nine FX at 100% processed 1.25 seconds of audio in **0.0864 s (0.0691× realtime)** on this specific CI runner. The new oversampling can change phase/group delay; no real DAW CPU or listening claims.
- Remain NO-GO: full-spectrum/multi-tone 44.1/48/96-kHz alias tests, realistic audio/phase and transient quality, finished historical GUI and Studio Pro final tests.

## 0.9.3 — P013 measured MANIA and DEPRESSION anti-aliasing (2026-11-23)

- Added reusable `src/oversample2x.hpp` sixth-order Butterworth wet-only oversampling for the two nonlinear shapers, preserving the existing plugin contract and state.
- **MANIA:** 15-kHz source alias at 3 kHz improved from −20.22 to **−57.32 dBc**. **DEPRESSION:** from −20.47 to **−59.03 dBc**. 14-kHz tests similarly improved to −61.72 and −73.31 dBc. Explicit **−50 dBc** CI gates now protect both on both tones; PSYCHOSIS −45 dBc remains.
- [Real Windows VST3 run 45285249372](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249372): **47/47 Steinberg PASS**, VST3 state 1/1, actual 180-row audio/45 wet-only comparisons, sample-accurate automation and negative placebo test PASS.
- [Windows/Linux DSP 45285249438](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249438): 6/6 tests per platform, 120-case audit **0 flags and 0 safety-level violations**. [Win32 EditorHost 45285249313](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249313): PASS.
- P013 full-chain CPU on its CI host: 0.0526× realtime (1.25-second 48-kHz stereo block series); no guaranteed host-performance claim.
- **Still open:** PANIC high-frequency alias (~−31.87 dBc), other untested effects, group delay and transient behavior, realistic musical recordings, final photographic historical GUI and Studio Pro sign-off.

## 0.9.2 — P012 measured PSYCHOSIS oversampling (2026-11-23)

- Discovered unwanted folded 3rd-harmonic aliases using **actual VST3 Processor::process**: 15-kHz tone aliased to 3 kHz at **−18.85 dBc** under PSYCHOSIS 100%; 14 kHz folded to 6 kHz at **−20.33 dBc**.
- Replaced preliminary single-pole PSYCHOSIS postfilter with **2× reconstructed wet-only nonlinear shaping**, three 2nd-order Butterworth sections (sixth-order) **before** each of the three shaper band paths and **after** combined wet output. No heap allocation in processing.
- Verified −59.47 dBc (15k →3k) and −65.25 dBc (14k→6k), roughly **40–45 dB alias reduction** on the two specified probes. Added **−45 dBc P012 regression guard** on both.
- Kept the 0%=bit-perfect Dry / 100%=pure DSP Wet contract, inverted global THERAPY, VST3 IDs and project state intact. Actual Windows DSP 45/45 full-Wet and 180-row reference metrology still pass.
- Actual Windows 9-macro 100% CPU probe **0.0526× realtime** on its Actions runner; runner-dependent and not a DAW CPU guarantee.
- [Windows VST3/Steinberg 47/47, alias and CPU build 45283355056](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355056), [Windows/Linux 6/6 and 120-case audit 45283355041](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355041), [Win32 GUI smoke 45283355019](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355019): all **SUCCESS**.
- **Not yet fixed:** MANIA and DEPRESSION folded aliases, 44.1/96-kHz spectrum benchmarks, realistic musical/transient/phase acceptance and finished historical interface.

## 0.9.1 — P011 measurable wet-only gain staging (2026-11-23)

- Corrected sound level only **inside the effected branch** of DISSOCIATION (gradual 1 + 0.80 m² makeup), HALLUCINATION (1 + 0.65 m² makeup) and PANIC (1 / (1 + 0.25 m²) attenuation), preserving 0% Dry and 100% processed Wet semantics.
- Actual VST3 measurements: DISSOCIATION on synthetic drums −11.12→−6.02 dB; HALLUCINATION on synthetic bass −7.46→−3.11 dB; PANIC on voice-like source +4.47→+2.53 dB.
- Strengthened 180-row actual-VST3 metrology: **45/45 full-Wet** combinations now fail if any RMS gain differs more than −8.0/+6.0 dB from Dry. Maintains pure-Wet DSP reference, sample-accurate automation and gain-placebo negative controls.
- [Windows VST3 45280035654](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035654) **SUCCESS**, 47/47 Steinberg; [Windows/Linux DSP and P006 120-case audit 45280035612](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035612) **SUCCESS**, 0 warnings/0 level violations; [GUI run 45280035498](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035498) **SUCCESS**.
- Pure-Wet/0-Dry endpoint and VST3 component IDs, automation IDs and project-state v4 remain unchanged; no real-source musical acceptance claimed.

## P010 — real VST3 audio metrology (2026-11-23; plugin factory remains 0.9.0)

- Introduced `tests/test_vst3_audio_path.cpp`, compiling the actual Windows VST3 Processor into a host-style audio rendering executable. No parallel fake DSP abstraction.
- Proven 0% macro = bit-perfect Dry and 100% macro = its raw full-wet DSP output within 3×10⁻⁶ (45 full-wet macro/source runs). THERAPY 100% = exact Dry, THERAPY 50% = half Wet, and Bypass = Dry.
- 180 objective metrology rows: 5 deterministic synthetic sound categories × 9 macros × 4 effect intensities. All 45 full-wet source/macro combinations pass gain-invariant residual gate; a gain-only counterexample is rejected.
- Added 4-band gain-normalized spectral descriptor and sample-accurate MANIA automation regression at sample 128. Exported 10 actual-VST3 audio WAVs and full CSV in CI.
- MSVC Windows [run 45277192703](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45277192703): SUCCESS, Steinberg 47/47 plus state and real audio gates; no DSP binary algorithms or project-state IDs changed.
- Musical listening, aliasing/transient analysis, CPU under Studio Pro and final historical GUI remain pending.

## 0.9.0 — P009 (2026-11-23)

- Added original VSTGUI vector CRT with **nine effect-specific diagnostic worlds**, 288×325 pixel area in a 1004×570 dark dual-language front panel. Dominant macro selects scene, other macros introduce interference; UI sampling only, no audio-thread graphics.
- Actual Steinberg Windows **EditorHost mouse-click test** now proves LOSS OF CONTROL changes the live visible CRT, and switching EN→DE survives in the same session.
- Fixed visually subtle DEPRESSION response with an intensity meter. Reduced-motion (MOTION/BEWEGUNG) **off by default**, controller UI state v2 with v1 compatibility; DSP state v4 unchanged.
- Converted the CRT to actual **black and white**, completed German monitor labels and shortened THERAPIE / MIX to prevent clipping.
- QA: Steinberg VST3 47/47, 1/1 state/regression, P006 Windows/Linux DSP suite, P009 Win32 click/screenshot proof: **all PASS**.
- Historic photo assets, high-fidelity audio listening, aliasing/CPU in Studio Pro and final acceptance still pending.

## 0.8.0 — P008 (2026-11-23)
- Implemented real LOSS OF CONTROL GUI button: 9 host-automatable VST3 parameter edits, optional grouped undo, no THERAPY or Bypass mutation.
- Added mock DAW host check for each begin/perform/end edit and group boundaries; two successive presses yield changing macro values.
- Functional DE/EN layouts and UI-only persistent language preference, no change to the v4 DSP state or stable ParamIDs.
- Screenshot-driven GUI correction: spacious nine-knob layout, hardware-style dark buttons, readable percent displays, THERAPY pointer.
- Windows official Steinberg VST3 API validator 47/47; controller/host-state regression 1/1; actual VST3 editor opened in Steinberg EditorHost Win32 runtime. Latest P008 CI details in [STATUS.md](STATUS.md).
- Historical reactive CRT photos, critical listening, aliasing/CPU acceptance and Studio Pro project QA still outstanding.

## P007 — provisional VSTGUI checkpoint (factory version remains 0.6.0; 2026-11-23)
- Added real VSTGUI 4 editor using a bundled source-controlled .uidesc. Includes 9 macro knobs, THERAPY, Bypass and bound numeric parameter displays; no bitmap third-party assets.
- Windows CI compiled the GUI-enabled VST3, passed Steinberg 47/47 and actual SDK state test 1/1, confirmed all control tags packaged.
- Studio Pro editor-open and hearing test not yet performed; first user testing remains deferred.
- Historical animated diagnostic-monitor UI, image licenses, LOSS OF CONTROL and DE/EN still outstanding.

## 0.6.0 — P006 (2026-11-23)
- P006 objective 120-case synthetic audio quality/CPU audit and CSV artifact.
- Original reference measurements revealed excess gain in MANIA (clipping and >+9 dB), weak DEPRESSION/PANIC onset, and excess PSYCHOSIS gain. Applied corrective DSP mappings and compensation.
- Audit now gates clipping and >+6 dB gain; the final documented reference run reports 0/120 warnings.
- Reduced HALLUCINATION frequency-shift DSP cost by precomputing Hilbert FIR coefficients; equivalence regression added.
- Added SDK-facing VST3 processor/controller state v1–v4 roundtrip and malformed-state checks.
- Windows VST3 factory version and package name aligned to 0.6.0. Validation: Steinberg 47/47; CTest 6/6 standalone on Windows/Linux; VST3 state 1/1 Windows.
- No GUI, no LOSS OF CONTROL, no Studio Pro musical sign-off yet.

## 0.5.0 — P005 (2026-11-23)
- Completed nine audio **prototypes**: SCHIZOPHRENIA, DISSOCIATION, HALLUCINATION joined six earlier chains.
- DAW-automatable nine macros and THERAPY, VST3 state v4 with backward decode v1–v3.
- Nine-chain audio invariants and no-allocation callback checks.

## 0.4.0 — P004 (2026-11-23)
- MANIA, PANIC, PSYCHOSIS compound audio prototypes; six-macro Windows VST3.

## 0.3.0 — P003 (2026-11-23)
- DEPRESSION and OBSESSION compound DSP; PARANOIA retained, shared global THERAPY, state v2.

## 0.2.0 — P002 (2026-11-23)
- First working Windows VST3 host wrapper for PARANOIA, generic host editor and state v1.

## 0.1.0 — P001 (2026-11-23)
- Specification, event-triggered past-audio reverse prototype, WAV renderer, CI groundwork.
