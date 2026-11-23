# Change history — 125A PSYCHIATRY

Only the `main` branch is used. The most recent **verified code commit** and test links are always in [STATUS.md](STATUS.md).

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
