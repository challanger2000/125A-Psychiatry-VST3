# Change history — 125A PSYCHIATRY

Only the `main` branch is used. The most recent **verified code commit** and test links are always in [STATUS.md](STATUS.md).

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
