# 125A PSYCHIATRY

**Experimental dark multi-effect for Gothic, Darkwave, EBM, Industrial and cinematic horror sound design.**

> **Current verified engineering checkpoint: P016 exact endpoints, repeatable reset and phase/impulse diagnostics / factory v0.9.5**, Windows x64 VST3. Nine real FX macros, inverted THERAPY, Bypass, LOSS OF CONTROL, EN/DE interface and a **functional nine-scene monochrome clinical CRT** controlled by the same nine macro parameter values as the audio engines. The official **Steinberg EditorHost opened the Windows editor and actual mouse-click tests verified that the randomized knobs change the CRT**. Current Windows build: **47/47 Steinberg tests PASS**; independent actual-VST3 audio gate confirms 0% dry, 100% matched wet and THERAPY inversion. **Not the final photographic GUI and not yet approved in Studio Pro.**

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

## P016 — verified v0.9.5

[Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465707) and [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45290465506) passed, including exact dry for extended finite headroom/tiny floats and deterministic PARANOIA reset. The phase/impulse audit revealed a **remaining** partial-mix cancellation problem (worst observed −23.55 dB); it is not a phase acceptance. [Details](docs/P016.md).

## P015 — measured multirate VST3 quality gate

[Windows P015 run 45288772417](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772417) **SUCCESS**. All nine macro endpoints and inverse THERAPY were retested at **44.1, 48 and 96 kHz**, with actual Windows VST3 audio processing and 14+15-kHz two-tone excitation. Four nonlinear worlds yield **32 folded products** at 44.1/48 kHz; the strongest is **−52.18 dB relative to input** and all pass the new **−45 dB(input)** regression threshold. Spectral products at 96 kHz are in-band and intentionally not mistaken for aliases. The factory remains **v0.9.4**, not a release.

[75 P015 spectral/peak CSV rows, actual VST3 audio metrics and WAVs](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772417/artifacts/14873057712) · [temporary P015 Windows .vst3 developer bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772417/artifacts/14873013103). Independent [Windows/Linux DSP suite](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45288772568) also passed. Phase, realistic recordings, extended audio sessions, final GUI and Studio Pro acceptance remain open. **No user testing requested.**

## P014 — PANIC anti-alias improvement, actually measured

- **Factory 0.9.4, Windows x64**, verified tested commit `0fdf15bc45e1acc86b74653423709fd99004d9f6`. [True VST3 audio/Steinberg build 45287094009](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094009): **SUCCESS**.
- PANIC's wet nonlinear excitation now passes through 2× oversampling and sixth-order filtered reconstruction/decimation. Its 15-kHz-input/3-kHz-folded alias fell **−31.87 → −69.35 dBc**, a **37.48-dB improvement**; 14-kHz/6-kHz alias fell **−31.86 → −73.41 dBc**. A **−45-dBc regression gate** protects both frequencies.
- **47/47 Steinberg validator**, **45/45 full-Wet DSP comparisons**, nine 0%=bit-exact Dry, THERAPY inverse, anti-placebo negative test, sample-offset automation, 180 metrology source cases all pass. The official [Windows editorhost GUI](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287093826) and [Windows/Linux DSP suite](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094101) also pass.
- [P014 actual VST3 stereo WAVs, 180-row metrology and narrowband alias/CPU CSV](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094009/artifacts/14872417653); [P014 Windows VST3 developer bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45287094009/artifacts/14871169475). Not final; temporary 7-day artifacts.
- Next tasks remain realistic recordings, 44.1/96-kHz multi-tone/intermodulation tests, transient/phase matching, host CPU and final historical graphics. **No end-user test yet.**

[Full P014 method and limitations](docs/P014.md).

## P013 — Measured MANIA + DEPRESSION alias suppression

- **Verified factory v0.9.3**, source `37988d988b8180eb28c99d5f53d51e4d0ff67e2a` on `main`. [Windows build + true VST3 audio test](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249372): **SUCCESS**, Steinberg **47/47**, 45/45 full-Wet comparisons, THERAPY endpoints, native Bypass, sample-accurate automation and placebo negative.
- Added **true 2× filtered oversampling** around the MANIA feedback saturator and DEPRESSION program-dependent saturator, avoiding an extra Dry path. Their 15-kHz-source/3-kHz-alias levels improved from **−20.22 to −57.32 dBc (MANIA)** and **−20.47 to −59.03 dBc (DEPRESSION)**. Both now have a **−50 dBc regression gate** for two defined tones.
- [Actual per-macro VST3 WAV comparisons plus wet/dry, alias and CPU metrics](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249372/artifacts/14872505322); [v0.9.3 Windows .vst3 experimental bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249372/artifacts/14870953327).
- [Windows/Linux DSP suite and 120 quality cases](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249438): **SUCCESS**, 0 safety violations or review flags. [Official Windows VSTGUI EditorHost regression](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45285249313): **SUCCESS**.
- **PANIC remains at approximately −31.87 dBc** in that high-tone alias probe. Full multi-tone spectral audit, 44.1/96-kHz response, phase/transient QA, and musical acceptance with real instruments remain NO-GO items.

[Technical P013 report](docs/P013.md) · [current STATUS](STATUS.md). **No early user test requested.**

## P012 — Real PSYCHOSIS alias improvement, objectively verified

- [Verified P012 Windows 64-bit VST3 and official Steinberg 47/47](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355056), source commit `6e2acf37498ac3fc3c759e9c8501253d078d9212`; project/factory version **0.9.2**.
- **PSYCHOSIS now uses filtered 2× oversampling around the real wet-only nonlinear DSP**, not around the bypass or THERAPY.
- **15-kHz input / 3-kHz alias at 48 kHz:** −18.85 → **−59.47 dBc**; **14 kHz / 6-kHz alias:** −20.33 → **−65.25 dBc**. The mandatory PSYCHOSIS regression guard is **−45 dBc** on both reference tones.
- [Original Windows VST3 audio probes, 180-case wet/dry CSV, spectrum/CPU CSV](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355056/artifacts/14870463421); [Windows experimental VST3 bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355056/artifacts/14870123592). Artifacts are temporary engineering evidence, not a release.
- All **45/45 100%-Wet direct DSP comparisons**, zero-exact Dry, inverted THERAPY, anti-placebo reference, 180 measurement rows and sample-offset automation **PASS**. [Windows/Linux standalone 6/6 suite and 120-case quality audit](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355041) **PASS** with 0 quality flags and 0 level violations. [Official Windows EditorHost GUI probe](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45283355019) also PASS.
- **CPU**, nine FX at maximum on this CI machine: about **0.0526× realtime**; not a guarantee of real DAW performance. MANIA and DEPRESSION still show considerable folded-harmonic energy on the same high-frequency probe and need further investigation.

[Detailed P012 engineering report, spectral method and limits](docs/P012.md). **No user test yet.**

## P011 — Corrected wet-only gain staging, measured

- [P011 Windows x64 VST3 and actual-audio metrology](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035654): **PASS**; Steinberg 47/47, real audio null/mix/automation and pure-wet reference 1/1, 180-row report.
- [Latest per-effect measured audio + CSV](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035654/artifacts/14867663887) and [P011 Windows development plugin](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035654/artifacts/14867439147).
- Fixed measured excessive full-Wet attenuation in **DISSOCIATION** (synthetic drums −11.12 → **−6.02 dB**) and **HALLUCINATION** (synth bass −7.46 → **−3.11 dB**) and reduced **PANIC** gain (voice-like +4.47 → **+2.53 dB**). Gain-only compensation is confined to the **processed Wet branch**, never a hidden Dry blend.
- A new regression makes **−8 to +6 dB full-Wet RMS** an acceptance range for all 45 synthetic source/macro combinations. All pass, as do 0% exact Dry, 100% exact native DSP Wet, THERAPY reverse Dry/Wet and anti-placebo negative tests.
- [120-case DSP regression](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035612): Windows/Linux success, **0 review flags and 0 level violations**. [Windows Steinberg EditorHost GUI smoke](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45280035498): success.
- These tests use generated audio references. **Real-recording listening, anti-aliasing analysis, final historical graphics and Studio Pro acceptance are still pending.**

[P011 technical report](docs/P011.md).

## P010 — Effect audibility and 0%/100% endpoint verification through the real VST3

- [P010 Windows test run: real processor, wet/dry, nine effect worlds, automation, 47/47 Steinberg validator](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45277192703): **SUCCESS** on tested source commit `ceb57ac2dc4d16b274cd2efcec2200f2d9b37e5a`.
- [180 actual-VST3 measurement rows + 10 WAV comparisons](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45277192703/artifacts/14867677257): 5 synthetic stereo sources (synth bass, drums, guitar-like, voice-like, atmosphere) × 9 macros × 20/50/75/100%. WAVs: one bass dry and nine bass fully wet. **7-day engineering artifact, not a public release**.
- **0% each macro:** bit-for-bit identical dry output with global THERAPY=0. **100% each macro:** direct wet-only DSP reference matches the actual VST3 within 3×10⁻⁶ on all 45 macro/source combinations; no additional parallel dry mixing at full macro.
- **THERAPY 0%:** pure chain output. **THERAPY 100%:** exact dry source. **THERAPY 50%:** linear half wet/half dry to within 5×10⁻⁶. Native Bypass returns bit-exact dry.
- **Placebo / loudness control:** pure-gain-only reference is rejected. On **45/45 full-wet** macro/source cases, gain-matched waveform residual exceeds the defined audibility-screen threshold; differences are not explained merely by volume. Four normalized spectral bands are compared and exported.
- VST3 automation event at **sample 128** changes MANIA output only at/after the event, not before.
- This is an **objective synthetic reference check**, not proof of subjective quality, frequency-domain anti-aliasing quality, true vocal/guitar recordings or DAW session functionality.

[Complete P010 test method, exact thresholds and limitations](docs/P010.md).

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
