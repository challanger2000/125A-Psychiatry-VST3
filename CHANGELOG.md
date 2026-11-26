# Change history — 125A PSYCHIATRY

Current candidate: **0.9.29 / P031**. Only `main`. Current evidence: [STATUS.md](STATUS.md). Earlier entries are historical development checkpoints, not current instructions.

## Documentation cleanup — 2026-11-26
- Consolidated README, current status and handoff; historical documentation retained by immutable Git links.
- No source, DSP, GUI, state, build or binary changes. No V1.0 release claimed.

## P031 — 0.9.29 candidate, owner-requested usability
- First-aid box resets all 13 rotary controls through grouped host edits; BYPASS and MOTION unchanged.
- Left double-click resets the individual knob, retaining native drag, wheel and keyboard behavior.
- MOTION defaults on for new or empty controller state; explicit saved off is preserved.
- DSP, latency, artwork and audio state unchanged. Native and recall tests passed on exact P031 build; synthetic hosted timing still FAIL. See STATUS.md for measured evidence.

## P030a — remove development-only zoom state
- UI state v4 is 16 bytes, containing only magic, version, language and MOTION. No legacy zoom field or migration. Unreleased v1–v3 UI states rejected; audio processor state unchanged. Restore original artwork README.txt to preserve the manifest; current editor documentation lives in README.md. P030 configure stopped at README.txt SHA256 mismatch, no Windows runtime evidence yet.

## P030 — fixed-size editor (0.9.28 candidate)
- Owner removed user zoom; fixed 960 × 640 at DPI 1, automatic host DPI only. Removed zoom control, context menu choices and controller zoom state/resize synchronization. Legacy UI state v1–v3 readable; v3 reserved zoom slot saved as zero. DSP, artwork and audio parameter IDs unchanged. Native and state verification pending.

## P028k / 0.9.27 (2026-11-25) — re-audit candidate, not final

## 0.9.28 — P028l audit-correction candidate (not verified)

- R01–R03 per-stage automation curves, prevalidated host queues, active bypass recall ramp including zero-frame flush.
- R04–R07 bounded fade retarget composite, live editor preferences, exact ZIP/native dependency checks and MOTION initialization.
- New targeted native and audio regression probes; CI and independent audit pending. No creative changes or final release.


- A01–A10 source corrections, consolidated regressions, centralized factory version and silence flags. Four workflows PASS at `502227585a874dccd84cd5031bbae2092f354ae7`; independent follow-up audit and Studio Pro acceptance pending. See STATUS.md and docs/P028k-AUDIT-CORRECTIONS.md.

## 0.9.26 — P028h scene library candidate

- 40 unique new historical-fiction scene textures with twelve-world mapping, bounded GUI texture loading, slow timed crossfade/rotation, MOTION hold and native zoom probe. No DSP changes. P028j native GUI PASS, P028i Windows/DSP PASS and P028h recorded audio PASS; final DAW acceptance pending.

## 0.9.25 — P028d raster editor candidate

- Approved P028c artwork, thirteen native animated controls, momentary LOSS, circular photographic observation, English-only, motion, bypass, zoom and value displays. P028g all four CI workflows passed on exact source `87be28cb8f4d5c34a9a0bc164e4eda06ec6b03f8`; verified raster candidate, final acceptance pending; DSP unchanged.

## P027g — integrated functionality gate (factory v0.9.24 unchanged)

- Actual controller edits via SDK parameter queues into real processor; live audio change and new-instance recall with bit-exact restarted audio.
- No production changes; native CI pending. After green freeze functional baseline for final GUI.

## 0.9.24 — P027f verified twelve-world visual state

- Monitor logic observes all twelve parameters, stable primary/secondary, effective THERAPY and bypass, existing diagnostic resource indices safely mapped.
- Actual controller/model regressions; no sound or final graphic redesign. All four CI workflows passed.

## 0.9.23 — P027e verified randomizer density

- Randomly choose 3–6 active worlds from all twelve, retain .12–.78 amounts, explicitly reset others. Preserve THERAPY/bypass/UI and manual range.
- Native host density regression, 32 actual recorded generator presets. No DSP or GUI changes; all four CI workflows passed.

## 0.9.22 — P027d verified spectral midpoint correction

- Partial power compensation for SCHIZOPHRENIA and HALLUCINATION crossfades only, midpoint +1.938 dB; endpoints unchanged.
- Analytic and actual recorded prefix regression; prior safety gates unchanged. P027c passed three triggered workflows. P027d passed all four.

## P027c — recorded combination gain audit (factory v0.9.21 unchanged)

- Actual VST3 prefix and mixed-macro measurements (68 additional recorded cases); combined THERAPY/bypass invariants.
- P027b passed three triggered workflows; full-chain attenuation flagged for review. No product code changes; new CI pending.

## P027b — expanded functional audit (factory v0.9.21 unchanged)

- Host integrity now covers all 12 effects as well as legacy 9, mono/stereo and 44.1/48/96 kHz; no production changes.
- Objective matrix 168 cases; retain old nine-chain results and include new twelve-chain and three singles. Recorded QA triggers on actual processor/test changes.
- Three triggered workflows passed; no final listening approval.

## 0.9.21 — P027a verified twelve-world randomizer (2026-11-24)

- LOSS OF CONTROL now targets all twelve automatable sound IDs; retains existing random ranges and group edits.
- Expanded actual controller/host-spy regression, 64 presses and 36 callbacks per press. Global and UI-only values unchanged.
- Existing diagnostic GUI remains provisional. DSP unchanged. All three triggered workflows passed; recorded workflow not triggered.

## 0.9.20 — P026l verified freeze acquisition correction (2026-11-24)

- Preserve a deliberate equal-level freeze but reacquire on a substantially stronger source frame, preventing an almost silent leading edge from monopolizing a hold.
- Add quiet-onset regression and all-twelve P023 excitation diagnostics. Existing thresholds unchanged.
- P026k Windows P023 failed; DSP/recorded/EditorHost passed. P026l all four CI workflows passed (see STATUS.md).
- Document owner-approved vintage horror GUI concept in P027-GUI-DESIGN.md; no GUI code change.

## 0.9.19 — P026k twelve-world sound candidate (2026-11-24)

- HALLUCINATION adds parallel detuned resonances, source-dependent cross-modulation and a low-level displaced grain branch.
- DELIRIUM, CATATONIA, AMNESIA implemented and wired to stable new IDs 109–111, project state v5 with legacy prefix and defaults.
- Twelve-world actual processor/recorded-source matrix and focused new-world tests. Fixed existing 96 sample alignment retained; creative history/freeze onset is intentionally delayed. Existing nine-knob custom interface/monitor/randomizer still provisional.
- Candidate verification pending; no listening acceptance claimed.

## 0.9.18 — P026j candidate (2026-11-24)

- DISSOCIATION: continuous overlapping causal forward/reverse grains, independent stereo source schedules, short diffusion; replaces static reverse loop.
- New causal immutable-grain, continuous-overlap, recapture, silence and tail regressions. Previous fixed-loop assertion intentionally superseded by the approved flowing-grain behavior.
- Recorded-source workflow now watches all src/ files (MANIA's earlier recorded test did not run). Full validation pending.

## 0.9.17 — P026i candidate (2026-11-24)

- MANIA: input-reactive stereo-linked clock, real tapered two-repeat source fragment, moving resonant bandpass and retained oversampled feedback/chopper. New regression for retrigger density and replay after source stops. No fixed latency, state or ID change.
- Isolated C++ probes passed; actual repository CI pending. P026h remains verified.

## P025 — recorded-source verification (factory 0.9.12)

- [Recorded audio QA](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45312252550) SUCCESS, 180 measured rows and 72 RMS-matched A/B WAVs. Peak ≤ −1.943 dBFS; no changes to product source or bundle. Final DAW acceptance candidate remains v0.9.12.

## 0.9.12 — P024 (2026-11-24)

- Nine original procedural grayscale CRT images, dominant/secondary scene behavior and local displacement; no external photography.
- [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769352), [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769415), [GUI](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769462) SUCCESS. Actual raster visibility and bypass/restore verified. No DSP change.

## 0.9.11 — P023 (2026-11-24)

- Host tail declaration corrected: 30 seconds plus latency, measured all-nine decay at 44.1/48/96 kHz. [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45307623931), [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45307623916), [GUI](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45307623955) SUCCESS. No DSP algorithm change.

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
