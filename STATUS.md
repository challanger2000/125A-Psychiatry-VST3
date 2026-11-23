# 125A PSYCHIATRY — Verified Development Status
Updated: **2026-11-23** · **P003 three-macro Windows VST3 prototype (NOT a release)**

## Last verified code and tests
- **Only `main` branch**; preserve permanent factory IDs and parameter IDs.
- Commit **`717941a00e9d98cde9c63e6baaf237d7e83695ad`** passed Windows GitHub Action [45191507273](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45191507273) with a real `PsychiatryPrototype.vst3` and **Steinberg validator 47/47 PASS**.
- [DSP Windows + Linux workflow 45191507243](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45191507243): SUCCESS. P001 Paranoia and P003 Depression/Obsession checks passed in Release mode.
- Newer documentation and tooling changes must receive their **own** CI verification before calling those revisions accepted.

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
- No finished installer/release; build artifacts are ephemeral engineering packages.
- No claim of musical excellence based on validator pass alone.

## Next verification and project gates
1. Listen critically to DEPRESSION, PARANOIA and OBSESSION separately on bass, drums, voice and cinematic sounds; compare with dry and level-match externally.
2. Build and test the remaining six compound DSP worlds **individually**; reject indistinct/inaudible designs. Preserve fixed order or redesign with explicit validated routing.
3. Add reliable LOSS OF CONTROL, responsive GUI and a historically sourced visual system (rights provenance and motion safety).
4. End-stage Studio Pro real-host recall/automation/CPU/audio validation and final Windows package.

## Repository maintenance
Only `main`; update this status after each fully observed Windows + Linux run. Never confuse build/Steinberg validator pass with musical acceptance.
