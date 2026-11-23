# 125A PSYCHIATRY — Verified Project Status

Updated: **2026-11-23** · Current code phase: **P005 / v0.5.0 (nine-macro Windows VST3 prototype, NOT a finished release)**

## Last verified tested code
- Source commit: **`7f54e3802faa8192e31a2680aded07ead1055c1f`** on **main only**.
- Official Windows 2022/MSVC VST3 build and Steinberg API validator [45198889548](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45198889548): **SUCCESS; 47/47 tests passed, zero failures**.
- P005 standalone DSP workflow [45198889541](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45198889541): **SUCCESS on both Windows and Linux**, all **5/5 CTest test groups**.
- [Windows prototype artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45198889548/artifacts/14829987368) — `125A-PSYCHIATRY-P005-Windows-VST3-NOT-FINAL` (temporary artifact, 7-day retention; **not a final installer**).
- **P005 concurrent-chain regression:** nine DSP engines ran together, dry zero-amount output bit-exact, output finite and peak-bounded, results invariant across host block partitions, and **zero heap allocations detected inside the audio-processing call** on Windows and Linux. This is not proof of a production CPU budget.
- [Nine-world synthetic WAV probe bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45197686369/artifacts/14828402907) (temporary, includes grouped examples).

## Implemented and present in the Windows VST3
1. **DEPRESSION**: dynamics crush + dark filtering + saturating reverberation delay network.
2. **PARANOIA**: transient/clock-driven reverse slices + stereo nonlinear comb feedback.
3. **SCHIZOPHRENIA**: twin resampled/grain-pitch voices, opposing spectral motion, cross-modulation (fictional sound-world name, **not a clinical portrayal**).
4. **MANIA**: envelope-responsive, accelerating modulation + feedback-based ring coloration.
5. **PANIC**: transient detector, bounded tone/noise shock and restless gated/resonant body.
6. **DISSOCIATION**: stored source-derived backwards grains + temporally displaced smear/freeze texture (not a true FFT spectral freezer).
7. **OBSESSION**: micro-loop capture, repeats, retriggering and intentional bit-reduction.
8. **HALLUCINATION**: FIR Hilbert approximation frequency shift, dual formant-colored branches and eerie time feedback.
9. **PSYCHOSIS**: multi-band nonlinear shaping, wavefolding and signal interaction; **this is one distinct FX of nine, distortion is not central to all nine**.

These are **early playable DSP algorithms, not yet user-approved sound designs**. All macros are automatable in the host's generic UI. They are processed in a defined serial path: DEPRESSION → SCHIZOPHRENIA → MANIA → PANIC → PARANOIA → DISSOCIATION → OBSESSION → HALLUCINATION → PSYCHOSIS → once-only global THERAPY mix.

- **THERAPY:** inverse global Dry/Wet (0% processed; 100% dry), state/automation; **Bypass:** native.
- VST3 class IDs and macro parameter IDs are stable. Project state **v4** persists the nine macro values, therapy and bypass; **v1, v2, v3** old state representations are parsed to preserve earlier projects. **Actual host recall across old project files remains to be tested.**
- Mono/stereo, 32-bit float, C++17, Steinberg VST3 SDK 3.8.1 pinned at `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96`.
- DSP callbacks perform **no dynamic memory allocation by design**; memory is reserved at preparation. Actual audio-thread allocation tracing and CPU testing is still required.

## Outstanding / NO-GO for release
- **No custom GUI, no historical photography, no reactive clinical video monitor, no asset license/provenance audit.**
- **LOSS OF CONTROL randomizer not implemented; DE/EN language toggle not implemented.**
- **No real Studio Pro/Studio One audio test, no actual project recall/reopen acceptance, no user musical quality approval**. Listening must compare bass, guitars, drums, voice, cinematic sounds, level-matched and at varying intensities.
- Pitch/formant shifter quality, spectral approximation, anti-aliasing for nonlinear/folded processing, wet/dry phase response, extreme automation and latency/CPU budget require engineering review.
- GitHub Actions 47/47 validator verifies API behavior, not that the nine FX are distinct or commercially competitive.
- No finished installer, release, product screenshots, accessibility/motion settings or tested photo rights.

## Next phases
- **P006 DSP quality pass:** listening tests on real sources, measured output levels / oversampling / CPU; improve or replace weak macro algorithms rather than hiding flaws behind GUI. Verify all nine interacting and state recall v1–v4.
- **P007 GUI:** convincing antique psychiatric diagnostic instrument, thematic photo pools per knob, controlled analog display response, flicker accessibility, off-audio-thread video renderer. Clear rights for images.
- **P008 finish:** LOSS OF CONTROL on the actual nine host parameters; user-recognizable 125A visuals, DE/EN, actual Studio Pro end-stage audio QA and packaging.

## Documentation / repository policy
`main` only; stay precise about what is **verified**. Always update this status and relevant docs after actual changes, including tested commit and full Windows Actions run IDs. Use brand-only metadata in the released product; do not embed personal developer identity.
