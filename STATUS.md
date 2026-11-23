# 125A PSYCHIATRY — Verified Project Status

Updated: **2026-11-24**. Latest verified gate: **P016 exact endpoints/reset and phase/transient audit / factory v0.9.5**, **NOT a product release**. **Only `main`**; do not create temporary development branches.

## Latest verified build and fixed references

- **Source commit tested:** `de2fbe43c9752d0aecd1d0eea464416d52c49646` on **main** (P016 exact dry/wet endpoints, seeded reset, phase/impulse metrology; factory v0.9.5)..
- **Windows/MSVC x64 real VST3 workflow:** [run 45290465707](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465707), **SUCCESS**.
  - Steinberg VST3 SDK 3.8.1 validator: **47/47 PASS, 0 FAIL**.
  - Actual VST3 processor/controller state regression: **1/1 PASS** (historical state v1, v2, v3, v4, plus NaN, truncation and future-version rejection). **Not a DAW project reopen test**.
  - [P016 Windows VST3 bundle artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465707/artifacts/14873847457): `125A-PSYCHIATRY-P016-Phase-Transient-NOT-FINAL` (7-day retention, expires 2026-11-30).
- **Windows + Linux standalone audio workflow:** [run 45290465506](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465506), **SUCCESS**.
  - **6/6 CTest groups passed on each platform**. Includes all nine concurrent DSP engines and callback no-heap allocation test, plus precomputed Hilbert tap coefficient regression.
  - **P006 objective quality audit:** 120 sound/source/intensity cases, **0 review flags, 0 safety-level violations** after addressing measured issues. **P010 adds a separate 180-case test of the actual VST3 processor, not merely standalone DSP**.
  - [P011-quality and CPU metric methodology](docs/P011.md); metrics include wet/dry RMS ratio, dBFS peak, normalized delta, DC, maximum single-sample step, and runner-local render time.
- All Windows Actions artefacts are temporary engineering packages, not release installers.

## Actual audio code in the VST3

**All nine sound-world prototypes are implemented**: DEPRESSION, PARANOIA, SCHIZOPHRENIA, MANIA, PANIC, DISSOCIATION, OBSESSION, HALLUCINATION and PSYCHOSIS. Each is a dedicated compound DSP engine, not an inert parameter. Distortion is **one ingredient inside relevant effect worlds**, especially PSYCHOSIS, rather than the universal plugin core.

Processing order: DEPRESSION → SCHIZOPHRENIA → MANIA → PANIC → PARANOIA → DISSOCIATION → OBSESSION → HALLUCINATION → PSYCHOSIS → **one global inverse THERAPY Dry/Wet mix**.

- **P009 verified CRT interface:** Steinberg **VSTGUI**, dark 1004×570 panel, two EN/DE templates, nine live FX knobs, THERAPY, Bypass, LOSS OF CONTROL, percent readouts, language switch and original software-drawn **monochrome diagnostic CRT**. CRT scenes and cross-macro signal corruption are calculated on the UI thread from the actual controller parameters. No patient photos or third-party graphics are used.
- VST3 `createView(kEditor)` now constructs `VSTGUI::VST3Editor`, rather than returning null and falling back to host generic controls.
- **Official Steinberg EditorHost GUI interaction PASSED:** [run 45269301923](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301923). An actual Win32 GUI was created, a mouse click on LOSS OF CONTROL changed nine effect values and the CRT image (482 sampled pixels differed, RGB delta 81,702), and the subsequent DE language switch retained a working editor. [Three run screenshots](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301923/artifacts/14864592486) were visually inspected. **This is still not Studio Pro or an audio listening test.**
- Stereo/mono, 32-bit floating point input/output, deterministic state per instance; sample-rate dependent buffers allocated in preparation. CPU and realtime allocation *suite* exists, but actual end-user DAW latency/CPU stress still needed.
- VST3 factory version now **0.9.5**, actual component/controller IDs and ParamIDs unchanged. Project state **v4**, with decoding of v1/v2/v3.


## P016 — verified exact endpoints, reset and phase/transient audit

**Windows/MSVC run [45290465707](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465707) SUCCESS; Windows/Linux DSP run [45290465506](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465506) SUCCESS**, tested `de2fbe43c9752d0aecd1d0eea464416d52c49646`, factory **0.9.5**. Existing single-tone/two-tone, actual processor wet/dry, state and allocation gates remain passing.

- PARANOIA no longer substitutes sanitized samples at 0%/dry. Both tiny finite floats and >16.0 input samples pass unchanged at exact endpoints. RNG reset restores the original seed, reproducing the event schedule. THERAPY uses direct exact Dry/Wet endpoints.
- 144 per-channel six-tone phase observations and 24 impulse observations across 44.1/48/96 kHz. No pre-impulse audio, finite impulse outputs, initial peaks below unity; MANIA/PANIC/PSYCHOSIS peak 3 samples after input onset, DEPRESSION at sample 0.
- **Open audio quality defect:** 50% THERAPY cancellation relative to a coherent half-sum reaches −23.55 dB at PSYCHOSIS/96 kHz/16 kHz, −19.34 dB at PANIC/44.1 kHz/8 kHz, and −14.79 dB at PSYCHOSIS/48 kHz/8 kHz. The half-sum itself is mathematically correct; wet-only causal oversampling phase is a plausible contributor, not an excuse to ignore the audible loss. This is a diagnostic result, **not a phase pass**.
- [Metrics and audio](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465707/artifacts/14874226720) · [development bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465707/artifacts/14873847457). Remaining: isolate filter-induced versus creative phase, improve unwanted cancellation without violating dry/wet contracts, musical source acceptance, final GUI, long-session stress and Studio Pro recall.

## P015 — verified 44.1 / 48 / 96 kHz and two-tone intermodulation

**Verified test-only source commit:** `a806b9980877ff327547f27532fab111add2cc92`, **main**, Windows/MSVC workflow [45288772417](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772417) **SUCCESS**; factory **v0.9.4 unchanged**, VST3 IDs and state v4 unchanged. [Standalone Windows+Linux run 45288772568](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772568): **SUCCESS**. This is not a release or Studio Pro audition.

- Actual `Processor::process` is tested at **44,100 / 48,000 / 96,000 Hz**, with two simultaneous 14+15-kHz tones and a one-second exact-bin spectral window after 12,000 warm-up samples. A negative-control Dry render has no measurable folded spur.
- **Every one of nine macros** at each of three rates passes **0%=bit-identical Dry** and **THERAPY=100%=bit-identical Dry**. All 27 full-Wet macro/rate outputs are finite and have measured absolute peak below 4.0. The earlier exact full-Wet/DSP reference and inverse THERAPY 50/50 at 48k remain covered separately.
- Four nonlinear worlds (**DEPRESSION, MANIA, PANIC, PSYCHOSIS**) yield **32 folded third-order output lines** at 44.1/48k: 42, 43, 44, 45 kHz products fold into 0.1–6 kHz. Worst measured line is **−52.18 dB relative to the input two-tone reference** (PSYCHOSIS at 48k/5k). The new hard regression rejects any line above **−45 dB(input)**; it passes.
- At **96 kHz**, those 42–45-kHz lines are **in-band harmonic/intermodulation products, not aliases**. No misleading alias-pass threshold is applied to them. These measurements are **input-referenced dB**, not dBc relative to each effect's attenuated carrier.
- [P015 75-line CSV plus P012 original metric/WAV exports](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772417/artifacts/14873057712) and [experimental Windows VST3 bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772417/artifacts/14873013103) (CI artifacts, seven-day retention).
- **Windows Steinberg validator 47/47 PASS; VST3 state 1/1; real-VST3 wet/dry test 1/1; Windows/Linux six DSP tests per OS PASS.** The new 32 folded lines are *not* a full-spectrum/all-world/phase/noise certification.

Remaining: recorded musical material, phase/group delay and transient quality after wet-only 2× stages, other five worlds' nonlinear aliases, long-session many-instance CPU and actual Studio Pro project recall. **No user-side testing before finishing.**

## P014 — PANIC oversampling and actual-VST3 alias regression

**Verified tested code:** `0fdf15bc45e1acc86b74653423709fd99004d9f6` on `main`; factory **v0.9.4**. The official [Windows VST3/Steinberg workflow 45287094009](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094009) is **SUCCESS**: **47/47 Steinberg tests**, v1–v4 state test 1/1, actual-processor audio test 1/1 and 180 measurement rows exported.

The P013 alias audit identified a remaining **PANIC** third-harmonic foldback, about **−31.87 dBc** for a 15-kHz tone (48-kHz host, folded energy at 3 kHz). P014 introduces the same proven **per-instance, zero-allocation 2× interpolator + sixth-order Butterworth anti-alias decimator** around PANIC's *wet-only excitation and tanh saturation*. Existing trigger, impulse, envelope, siren, MIDI/audio parameter IDs, global THERAPY and dry bypass remain unchanged.

| Actual Windows-VST3 sine probe, PANIC 100%, THERAPY 0% | Before P014 | After P014 | Improvement |
|---|---:|---:|---:|
| 15 kHz → 3 kHz folded third harmonic | −31.87 dBc | **−69.35 dBc** | **37.48 dB** |
| 14 kHz → 6 kHz folded third harmonic | −31.86 dBc | **−73.41 dBc** | **41.55 dB** |

- P014 regression now **rejects PANIC greater than −45 dBc** in both single-tone probes. Older **MANIA/DEPRESSION −50 dBc** and **PSYCHOSIS −45 dBc** thresholds stay active; all pass on the final build.
- **Actual-processor audio**: nine macros 0% = bit-identical Dry, 100% = matching dedicated wet DSP without a separate original Dry mixer, 45/45 full-Wet comparisons, THERAPY 100% exact Dry, THERAPY 50% half Dry/half Wet, sample-accurate MANIA automation, explicit gain-only placebo rejection, and 180 source/macro/intensity rows **PASS**.
- **Windows/Linux DSP**, [run 45287094101](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094101), **SUCCESS**: 6/6 CTests per OS, 120-case audit **0 review flags and 0 safety-level violations**.
- **Actual official Steinberg Win32 EditorHost**, [GUI run 45287093826](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287093826), **SUCCESS** after the P014 DSP change.
- **Reference CI CPU**: at all nine macros 100%, **0.0864 s** for 1.25 s 48-kHz stereo audio (**0.0691× realtime**), maximum sampled callback **0.4413 ms**. The prior P013 run was **0.0526×** on a different ephemeral CI machine; do not infer the user's Studio Pro CPU from this.
- [P014 actual-VST3 180-row CSV, stereo WAV examples and alias/CPU CSV](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094009/artifacts/14872417653), [Windows VST3 development bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094009/artifacts/14871169475), both expiring 2026-11-30.

**Still not finished:** multi-tone alias and intermodulation tests at 44.1/48/96 kHz, other worlds' spectral artifacts, transient/phase and delay response of the Butterworth chain, critical listening with actual recorded guitars/drums/voice, long-session realtime/CPU tests in Studio Pro, and rights-cleared finished historical GUI assets. **No user test requested.**

[Detailed P014 engineering notes](docs/P014.md).

## P013 — wet-only alias suppression now also proven in MANIA + DEPRESSION

**Latest tested commit:** `37988d988b8180eb28c99d5f53d51e4d0ff67e2a` — factory **v0.9.3** — branch `main` only. [Windows P013 VST3 build and real audio test 45285249372](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249372): **SUCCESS**; 47/47 Steinberg validator, state 1/1, actual VST3 metrology 1/1 and 180-row export.

P012 had correctly identified additional 14/15-kHz folded-harmonic aliases in **MANIA** and **DEPRESSION**, although PSYCHOSIS was already corrected. P013 introduced a reusable, per-instance `detail::Oversample2x` sixth-order Butterworth **interpolator plus independent decimator**. Only the real nonlinear Wet shaper runs at double host rate. The user's 0% Dry / 100% processed-Wet and reverse THERAPY contracts are unaffected.

| Effect, 48-kHz real VST3 / 15-kHz source → 3-kHz folded alias | Before P013 | After P013 | Suppression |
|---|---:|---:|---:|
| **MANIA** | −20.22 dBc | **−57.32 dBc** | **37.10 dB** |
| **DEPRESSION** | −20.47 dBc | **−59.03 dBc** | **38.56 dB** |
| PSYCHOSIS (P012, unchanged) | −18.85 dBc before P012 | **−59.47 dBc** | 40.62 dB |

The 14-kHz source → 6-kHz folded result is **MANIA −61.72 dBc**, **DEPRESSION −73.31 dBc**. Both now have hard P013 tests rejecting values above **−50 dBc** for both tones; the earlier PSYCHOSIS **−45 dBc** guard remains.

- **Actual VST3 audio:** 45/45 full-Wet direct-DSP matches, all nine 0%=exact original Dry, 100%=pure engine Wet, inverted THERAPY 100%=exact Dry and 50%=half Dry/half Wet, sample-offset automation, pure-volume placebo negative, 180-row metrology **PASS**.
- **Windows/Linux DSP and 120-case quality audit:** [run 45285249438](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249438) **SUCCESS**; 6/6 test groups each platform, 0 quality flags and 0 level violations.
- **Official Win32 Steinberg EditorHost GUI smoke:** [run 45285249313](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249313) **SUCCESS**.
- **CPU on final P013 Windows Actions runner:** all nine macros at 100% took **0.0658 s** to process **1.25 s** stereo at 48 kHz (**0.0526× realtime**); this is a runner-dependent measurement, not a certified DAW CPU load.
- [P013 measured VST3 audio CSV, 10 stereo WAVs and alias/CPU CSV](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249372/artifacts/14872505322) (temporary, 7-day CI retention); [P013 preview VST3 bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249372/artifacts/14870953327).
- **Still open:** PANIC's 15-kHz probe remains at about **−31.87 dBc** (intentional transient excitation and tanh), other worlds are not alias-certified. Two-times oversampling can affect wet-only phase, attack and tonal character; musical listening, multi-tone/44.1/96-kHz alias tests, real-recorded source acceptance, and final Studio Pro project recall remain unverified.

[Detailed P013 analysis](docs/P013.md).

## P012 — anti-aliasing proof from the real VST3, not an assumed filter improvement

**Verified source:** `6e2acf37498ac3fc3c759e9c8501253d078d9212` on `main`, factory **v0.9.2**. [Windows real VST3/Steinberg validator 45283355056](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355056) **SUCCESS: 47/47**, state 1/1, actual VST3 audio 1/1, and exported 180 sound measurements plus P012 spectral/CPU CSV.

**P012 measured problem on the old DSP:** high 14- and 15-kHz tones at 48 kHz generated folded harmonic energy at 6/3 kHz. PSYCHOSIS, the plugin's primary nonlinear macro, had particularly strong aliases. P012 implements **true streamed 2× oversampling for PSYCHOSIS's wet-only nonlinear shaping path** with sixth-order Butterworth anti-image/anti-alias IIR filters **before and after** the shapers. This is not a lookalike processor or offline-only filter.

| Single-tone real-VST3 probe, 100% PSYCHOSIS | Pre-oversampling | P012 verified | Improvement |
|---|---:|---:|---:|
| 15 kHz fundamental → 3 kHz alias | −18.85 dBc | **−59.47 dBc** | **40.62 dB** |
| 14 kHz fundamental → 6 kHz alias | −20.33 dBc | **−65.25 dBc** | **44.92 dB** |

- The regression now **fails** if either monitored PSYCHOSIS folded-harmonic ratio exceeds **−45 dBc**. Both probes pass on the real Windows VST3.
- 0% each macro remains **bit-for-bit Dry**; 100% each still agrees with the actual dedicated DSP wet output (**45/45 source/macro combinations**), with no additional Dry blend. THERAPY 0% fully Wet / 100% bit-perfect Dry, 50% expected equal mix; Bypass, pure-gain negative control, sample-accurate automation and reference gain limits are all still passing.
- **Runner-local CPU probe, all nine macros 100%:** 60,000 stereo samples at 48 kHz processed in **0.0657 seconds**, i.e. **0.0526× realtime** on the P012 Windows CI machine. All nine at 0%: 0.0315× realtime. These are measurements on different CI runners from older baselines, **not guaranteed CPU percentages on a user PC**.
- **Still not alias-clean:** MANIA has −20.22 dBc and DEPRESSION −20.47 dBc in the 15-kHz/3-kHz probe, unchanged from the prior code. PANIC was −31.87 dBc. **No release-ready anti-alias certification of all effects** is claimed.
- [P012 actual-VST3 180-row wet/dry audio CSV, 10 WAV examples, alias and CPU CSV](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355056/artifacts/14870463421) and [actual Windows .vst3 developer package](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355056/artifacts/14870123592), retained temporarily until 2026-11-30.
- [Windows/Linux DSP regression 45283355041](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355041): **6/6 CTests per OS, 120-case quality audit 0 review flags / 0 safety violations**. [Windows Steinberg GUI smoke 45283355019](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355019): **SUCCESS**.
- **Remaining:** actual recorded instruments/voices, high-resolution multi-frequency anti-alias/true-peak measurements, musical transient/phase response and plugin delay/phase matching (2× IIR stages have phase shift), long-session CPU/denormal tests and Studio Pro final sign-off. The final historical photographic GUI is still incomplete.

[Detailed P012 method and limitations](docs/P012.md).

## P011 — wet-only gain correction measured by real VST3

**Latest validated code:** `ca3325614d1868539ab34deaa0fc72e5d70ef44d`, Windows workflow [45280035654](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035654), **SUCCESS**. 47/47 Steinberg validator, v1–v4 state test 1/1, actual VST3 audio-path regression 1/1, full 180-row CSV export PASS. [Latest measured CSV + 10 actual VST3 WAVs](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035654/artifacts/14867663887).

P010 identified excessive full-Wet loss of DISSOCIATION and HALLUCINATION and undue full-Wet gain in PANIC. P011 changes **only the transformed wet branch** of those three engines, never injects an additional Dry path. Each makeup/trim multiplier is smoothly dependent on the respective macro `m` and is unity at 0%.

| Measurement at macro 100%, THERAPY 0% | P010 pre-fix gain vs Dry | P011 verified gain vs Dry |
|---|---:|---:|
| DISSOCIATION – synthetic drums | −11.12 dB | **−6.02 dB** |
| DISSOCIATION – synth bass | −4.14 dB | **+0.97 dB** |
| HALLUCINATION – synth bass | −7.46 dB | **−3.11 dB** |
| HALLUCINATION – synthetic guitar | −7.40 dB | **−3.05 dB** |
| PANIC – synthetic voice-like | +4.47 dB | **+2.53 dB** |
| PANIC – synthetic drums | +3.05 dB | **+1.11 dB** |

- **All 45/45 full-Wet combinations** still match the pure, directly processed DSP output (maximum allowed sample error 3×10⁻⁶); **0% remains exact Dry**, THERAPY 100% exact Dry, THERAPY 50% remains equal-wet/dry, and VST3 automation and pure-gain placebo negatives pass.
- The real VST3 audio-test CI now **fails** if *any* full-Wet source/macro RMS gain exits **−8.0 to +6.0 dB**; P011 passes all 45. This is only a reference-signal gain check, not normalization of real-world material.
- [Standalone Windows/Linux DSP run 45280035612](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035612) **SUCCESS**; 120-case P006 quality audit reports **0 review flags / 0 safety violations**. [Official Steinberg EditorHost GUI smoke run 45280035498](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035498) **SUCCESS**: no interface regressions after P011.
- Factory version incremented to **0.9.1**, while stable IDs, macro ordering, VST3 component project-state v4 and controller-only UI state v2 remain unchanged.
- **Not proven:** subjective usefulness, anti-aliasing of nonlinear paths, loudness across real recorded tracks, final Studio Pro test, final photo-based horror design. None should be described as passed yet.

[Full P011 record](docs/P011.md) · [Original P010 measurement contract](docs/P010.md).

## P010 — real VST3 signal-path verification, not a placebo

The major new evidence is **actual audio passing through `psychiatry::vst3::Processor::process`**, with real Steinberg `ProcessData`, stereo float32 buffers, and `IBStream` v4 state. Previous P006 tests isolated the standalone DSP; P010 exercises the plugin's complete output path.

- **Verified Windows Actions run [45277192703](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45277192703), SUCCESS:** Steinberg validator **47/47**, VST3 state **1/1**, real VST3 audio gate **1/1**. [180-row CSV and 10 WAV probes](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45277192703/artifacts/14867677257), expires 2026-11-30.
- Source signals: deterministic synthetic **bass, drums, guitar-like, voice-like, atmospheric stereo**, 48 kHz, each 1.25 seconds. Every macro individually tested at **0%, 20%, 50%, 75%, 100%**, while others are 0 and global THERAPY is 0.
- **0% each of nine macros:** bit-identical Dry input across all five test sources. Native Bypass: bit-identical Dry. **100% macro:** actual VST3 sample output matches its corresponding raw DSP wet output, max error ≤ **3×10⁻⁶**, for all **45** full-wet macro/source combinations; no extra dry path is added by the VST3 mixer.
- **THERAPY inverted by specification:** 0% is the wet engine result; 100% returns bit-identical original Dry; 50% is the expected 50/50 Dry/Wet mix with max sample error ≤ **5×10⁻⁶**. Verified per macro on synth-bass source.
- **45/45** full-wet macro/source results pass both raw-change and **gain-matched** waveform-residual thresholds, so it is not merely a volume effect. Minimum measured gain-matched residual among the 45 was **0.328** (PANIC on guitar-like material), above acceptance threshold **0.035**. A reference consisting solely of 0.41× Dry is deliberately detected as a **placebo-like pure-gain** counterexample; a normalized 4-band spectral descriptor is exported for tonal difference review.
- **Real VST3 parameter automation:** change MANIA at audio sample **128** of a 256-sample block. Before sample 128 audio is bit-exact dry; thereafter it changes. No duplicate/misplaced processing.
- Level observations remain relevant: DISSOCIATION at full wet is around **−11.12 dB RMS on synthetic drums** and HALLUCINATION around **−7.46 dB on synth bass**; PANIC voice-like rises **+4.47 dB**. These are effects that are demonstrably different, **not proof of musically good gain staging**. Human listening, aliasing and proper loudness normalization remain NO-GO items.
- [Full repeatable P010 method, direct wet reference and scope](docs/P010.md). The final `.vst3` binary still reports **v0.9.0**; P010 changed tests and CI, not audio processing algorithms or IDs.

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
- **No real Studio Pro listening test or project save/reopen, no user listening acceptance, no captured real musical/vocal audio comparison.** P010 proves significant transformations using generated bass, drum, guitar-like, voice-like and atmosphere *proxies*, not recordings.
- **No production high-fidelity assurance** for wavefolder aliasing, spectral/Hilbert sidebands, grain clicks, transient smoothing, long automation and long-session CPU behavior. The VST3 47/47 tests do not validate subjective quality.
- No release installer or final VST3 product. P014 corrects a further narrowband PANIC alias; release and musical acceptance are still NO-GO.

## Next engineering gates

1. **Next audio acceptance:** multi-tone, 44.1/48/96-kHz alias/intermodulation measurements of all nine worlds, phase/latency and transient benchmarks, real recorded bass/guitar/drums/vocals, and revisions where musical quality is poor. Real DAW testing **at the end** per project policy.
2. **Next final art phase:** establish rights-cleared original historic-clinic photographic motifs and richer worn-instrument styling for the already functioning monochrome CRT, keeping UI rendering off the audio thread; retain motion-off accessibility and bilingual labels.
3. **Final release QA:** actual Windows Studio Pro audible/protected realtime validation, project recall/automation/installer verification, final release readiness only on proof.

## Project rules

Keep **only `main`**, keep every verified last-tested commit and Actions links in this file; distinguish synthetic objective QA from actual audible approval. Release UI, packages and product metadata use the neutral **125A** brand; do not surface personal developer identity.
