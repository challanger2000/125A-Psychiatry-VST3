# 125A PSYCHIATRY — Product and DSP Specification
Version: **concept baseline** · Updated: **2026-11-24** · Implementation: **v0.9.10, P022 verified; NOT a product release**. See [STATUS](../STATUS.md) for authoritative progress.

## Intent
Windows x64 VST3 **insert audio effect**, not an instrument. Dark, extreme, purposeful sound design for Gothic, Darkwave, EBM, Industrial, Thriller and Horror. It should create powerful sounds from ordinary source material. Not a conventional multi-effect with theatrical labels: every knob is a **curated multi-algorithm macro**, whose processors and routing evolve together.

The nine macros are **peers**. Distortion is not the universal center of the plugin; it is used where appropriate, especially in PSYCHOSIS. A knob at zero must leave the signal unchanged. A small nonzero amount should already be audible; at high values it should be radical without uncontrolled output level.

## Controls
- Nine independent macros, 0–100%, DAW-automatable and recallable.
- **THERAPY / THERAPIE**: inverted global Dry/Wet. 0% therapy = 100% processed, 50% = equal dry/processed, 100% = dry. The processed branch must preserve clean passthrough when all macros are zero.
- **LOSS OF CONTROL / KONTROLLVERLUST**: trigger randomizes **only nine macros**; does not change THERAPY, preset identity, language, or output level. Random values must be bounded and transitions smoothed. Preset/project state includes final knob values, not an unreproducible implicit random state.
- DE/EN switch changes GUI captions only; no DSP changes.
- Consistent semantics: 0% = bypass/neutral; 1–20% clearly perceptible onset; 20–50% characterful; 50–75% extreme; 75–100% destructive/experimental.
- No gratuitous secondary settings. Optional hidden global safety configuration only if shown necessary by testing.

## Nine macro sound identities
These are **candidate processor recipes**, not a promise that every stage will be audible or active at every setting. Stage thresholds, topology, and gains require listening tests. Prefer 3 excellent algorithms over 7 mediocre ones.

### 1. DEPRESSION / DEPRESSION
**Goal:** heavy, muffled, suffocating decay, not just a low-pass.
- Candidate structure: asymmetric saturation → program-dependent transient/dynamic compression → drifting dark filter; parallel large dark plate → re-distortion of reverb tail → at high macro values gradual tail fragmentation.
- **Macro evolution:** compressed/darkened body → spacious but oppressive tail → unstable, crumbling room.
- **Visual vocabulary:** empty historical corridors, dimming light, faded photographic emulsion, sluggish image movement.

### 2. PARANOIA / PARANOIA
**Goal:** sound is untrustworthy; fragments seem to arrive unexpectedly from elsewhere.
- Candidate structure: short lookback buffer with occasional reverse slices + dynamic comb/feedback filter + spatial movement + saturation **inside the feedback loop**.
- **Macro evolution:** subtle threatening afterimages → irregular displaced echoes → brief reverse apparitions and distorted feedback events.
- **Visual vocabulary:** abrupt reframing, watchful angles, lateral picture slips, surveillance-like flashes.

### 3. SCHIZOPHRENIE / SCHIZOPHRENIA
**Goal:** one input splits into contrasting, dissonant identities. **Metaphorical name, not a medical portrayal.**
- Candidate structure: 2–4 parallel layers with pitch/formant offsets, independent drift, cross-modulation and a controlled ring-mod branch; rejoin nonlinearly.
- **Macro evolution:** distinct separation → conflicting timbres → harsh spectral collisions.
- **Visual vocabulary:** double exposure, split faces/images, diverging channels.
- **Caution:** avoid implying the effect reproduces symptoms or the experience of a diagnosis.

### 4. MANIE / MANIA
**Goal:** restless acceleration and mechanical escalation.
- Candidate structure: transient-aware rhythmic amplitude/frequency modulation + continuously densifying retriggers + envelope-driven waveshaping + feedback saturation.
- **Macro evolution:** agitated pulse → accelerated motion → feverish distorted outbreaks.
- **Visual vocabulary:** advancing measurement traces, accelerated frame cadence, rising instability.

### 5. PANIK / PANIC
**Goal:** sudden startling impulses embedded in usable audio, not random damaging peaks.
- Candidate structure: transient detection → short pitch-jolt gestures + narrow resonant sweeps + gated noise/electrical crackles + discontinuity-safe micro-breaks.
- **Macro evolution:** tremor → aggressive spasms → sharp but level-bounded shock gestures.
- **Visual vocabulary:** unstable electrical meters, brief desynchronization, camera shakes.
- **Safety:** no full-scale unbounded spikes; use controlled transients and anti-click ramps.

### 6. DISSOZIATION / DISSOCIATION
**Goal:** source loses continuity and sense of physical space.
- Candidate structure: granular slicing + sparse reverse grains + spectral freeze + diffuse decaying space; parallel dry fragments for intelligibility.
- **Macro evolution:** smearing → time dislocation → floating, frozen remnants.
- **Visual vocabulary:** partially frozen frames, fractured tiles, drifting image regions.

### 7. ZWANG / OBSESSION
**Goal:** fixation, repeated micro-motifs and escalating mechanical looping.
- Candidate structure: sample-accurate micro-loop capture + stepwise shrinking loop size + retrigger/gate + degraded-feedback or bit-reduction path.
- **Macro evolution:** insistent repetition → stutter rhythm → self-destructive loop texture.
- **Visual vocabulary:** repeating image section, locked frame, narrowing visual loop.

### 8. HALLUZINATION / HALLUCINATION
**Goal:** phantom harmonics, alien whispers and voices *derived from the user's signal*.
- Candidate structure: frequency shift + tuned/detuned spectral resonators + formant-like filtering / spectral resynthesis + reverse diffuse reverb.
- **Macro evolution:** spectral ghosts → disembodied harmonics → unnatural resonant presences.
- **Visual vocabulary:** faint apparitions, trailing afterimages, ghosted photographs.
- **Rule:** not an independent voice sample player by default. Input-derived transformations first.

### 9. PSYCHOSE / PSYCHOSIS
**Goal:** intense controlled distortion as one distinct world, **not the plugin's central engine**.
- Candidate structure: dynamic multi-band nonlinear shaper + asymmetric clipping + selective wavefolding + intermodulation branch; controlled low-frequency retention and automatic gain management.
- **Macro evolution:** dirty harmonic character → hard industrial fracture → near-disintegration.
- **Visual vocabulary:** scanline tearing, overload marks, fractured diagnostic readout.

## Routing and interaction
- Each macro is a defined compound processor, with serial **and** parallel branches where justified, not nine identical chains with renamed macros.
- Multiple macros can run simultaneously. Define **order, interaction and gain staging** explicitly before claiming a finished architecture. No unintended feedback path between macro chains.
- Combine multi-parameter curves continuously: crossfade, smooth modulation and parameter changes; do not hard-switch at macro thresholds.
- Use per-algorithm enable/bypass when inactive without disrupting delay/reverb tails. Shared DSP resources are optional optimizations **only if they do not cause state leakage between instances or macros**.
- Oversample relevant nonlinear processors where warranted; *intentional* alias textures must be explicitly differentiated from defects.
- No allocations, locks, disk reads or image processing on the realtime audio callback; ensure deterministic silence handling, mono/stereo behavior and sample-rate independence.

## Reactive historical display
An aesthetic of old clinical instruments, analog measurement devices, institutional corridors, treatment equipment, worn B/W photographs and a damaged monitor. The display changes imagery and transformation according to macro levels; combined macros can blend visual signatures. LOSS OF CONTROL affects both visuals and audio via the **same finalized macro values**, not an independent visual RNG. THERAPY reduces audio wetness; visual response can indicate whether the processed signal is audible.

- Still-photo pools may contain multiple images **per macro**. Historic photographs are an artistic reference, not claims that the effects portray illnesses or historical clinical conditions.
- Prioritize public-domain/CC0 objects, rooms and equipment; independently validate copyright, credit terms, privacy/personality rights and suitability of each item before shipping; keep an asset provenance register.
- Avoid presenting identifiable historic patients as disposable shock imagery.
- Image decoding/loading and animation stay off the audio thread. Provide an option to reduce motion / avoid rapid flashes. Display must not affect audio timing, quality, or processing.
- **No images are cleared for bundling yet.**

## Quality and product acceptance
1. Every macro must produce a **clearly distinct, immediately perceptible** effect in isolation; not a Colderator-style nearly inaudible knob.
2. Each macro must remain interesting across its travel, not merely become louder; no arbitrary abrupt jumps.
3. Test with clean synth bass, drums, guitar, human voice, atmosphere/field recording, and silence; include reference dry/wet level-matched WAVs.
4. Test all nine together, rapid automation, randomized settings, high-rate input, silence, host project save/reopen, multiple plugin instances and repeated start/stop.
5. No clipping jumps, NaN/Inf, DC buildup, runaway feedback or unsafe resonant peaks. Hard limiting, where used, is a final safeguard, not a substitute for correct gain staging.
6. Verify actual Windows VST3 build and Steinberg validator, then real listening in **Fender Studio Pro / Studio One** at the end of development, as requested.
7. No claim of a completed FX or passed audio test without a reproduced measurement or listenable artifact.

## Project policy
- Public repository, **main only**; simple, current docs and traceable commits.
- Windows x64 VST3 (C++ / Steinberg VST3 SDK / Visual Studio 2022).
- GitHub Actions should eventually build and test the *actual plugin*; a documentation-only green check is not a substitute for audio tests.
- Keep product name **125A PSYCHIATRY**, English default and DE/EN GUI labels.
- No personal names/account links in released GUI, installers or packaged metadata. Public repo hosting identity may still be visible on GitHub.
- Latest verified checkpoint **P022 / 0.9.10**. Fixed 96-sample reported latency; neutral, bypass and dry are exact after host compensation. Realistic musical audition, final GUI and Studio Pro acceptance remain open. STATUS.md is the authoritative current build record.

## External references (inspiration, not dependencies)
Sugar Bytes Turnado; UJAM Finisher series; Devious Machines Infiltrator; Freakshow Industries; Zopke Sounds PARANORMAL; remaincalm PARANOIA; de la Mancha therapy; Wavesfactory Whispers (visual reference). Do not copy proprietary code, imagery, presets, or product identity.
