# 125A PSYCHIATRY — Verified Development Status
Updated: **2026-11-23** · **P003 three-macro Windows VST3 prototype (NOT a release)**

## Last verified code and tests
- **Only `main` branch**; preserve permanent factory IDs and parameter IDs.
- Commit **`e40c2752976b08ccc78e182630af4fd6a0875eb8`** passed Windows GitHub Action [45192471302](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45192471302) with a real `PsychiatryPrototype.vst3` and **Steinberg validator 47/47 PASS**.
- [DSP Windows + Linux workflow 45192471228](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45192471228): SUCCESS. P001 Paranoia and P003 Depression/Obsession checks passed in Release mode. P003 renderer produced 6 synthetic WAV files and the new reverb-tail decay test passed.
- Last tested code revision additionally fixes the dark reverb feedback bound and confirms decay into silence. The next commit updates **documentation only**; no compiled DSP or VST3 source is changed.

## Actual functionality
- Real Windows x64 **VST3 audio effect** built against pinned Steinberg VST3 SDK 3.8.1 (source commit `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96`).
- **DEPRESSION**: nonlinear dynamics, dark filtering and saturating plate-like feedback delay network.
- **PARANOIA**: triggered past-audio reverse fragments and divergent stereo nonlinear feedback.
- **OBSESSION**: re-captured micro-loops, retriggering, fading and degrading digital feedback.
- All three macros are independently exposed as host-automatable controls. One inverted **THERAPY** global Dry/Wet, standard host Bypass. Processing is in defined order: DEPRESSION → PARANOIA → OBSESSION → single global mix.
- Host state **v2** stores the three macros, THERAPY and bypass. Older **P002 state v1** (`Paranoia, Therapy, Bypass`) is read without changing the plugin class identity. Note: legacy restore logic is implemented but lacks a dedicated real-host regression test.
- Host generic parameter editor only, no custom GUI. Full stereo and mono, 32-bit float only, no allocations within the DSP audio callback by design; production realtime tracing still required.

## Absolutely NOT complete
- **Six of nine macro DSPs** are missing: SCHIZOPHRENIA, MANIA, PANIC, DISSOCIATION, HALLUCINATION and PSYCHOSIS. Their parameter IDs are reserved but not exposed.
- LOSS OF CONTROL randomize and DE/EN switch are not yet implemented.
- **No actual Studio Pro listening session** or user sound quality approval, and no production CPU/stress measurements.
- No historical photographs/GUI; no image rights cleared, no render thread architecture.
- No finished installer/release; build artifacts are ephemeral engineering packages. [P003 Windows VST3 prototype artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45192471302/artifacts/14827115443) and [six dry/processed WAV probes](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45192471228/artifacts/14825783048) are temporary.
- No claim of musical excellence based on validator pass alone.

## Next verification and project gates
1. Listen critically to DEPRESSION, PARANOIA and OBSESSION separately on bass, drums, voice and cinematic sounds; compare with dry and level-match externally.
2. Build and test the remaining six compound DSP worlds **individually**; reject indistinct/inaudible designs. Preserve fixed order or redesign with explicit validated routing.
3. Add reliable LOSS OF CONTROL, responsive GUI and a historically sourced visual system (rights provenance and motion safety).
4. End-stage Studio Pro real-host recall/automation/CPU/audio validation and final Windows package.

## Repository maintenance
Only `main`; update this status after each fully observed Windows + Linux run. Never confuse build/Steinberg validator pass with musical acceptance.
