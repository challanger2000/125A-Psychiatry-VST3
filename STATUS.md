# Project Status — 125A PSYCHIATRY
Updated: **2026-11-23**

## Current state
- Repository created, public, default branch **main**.
- **Concept specification v0.1.0** documented in [docs/SPEC.md](docs/SPEC.md).
- No audio DSP, VST3, GUI, historical picture assets, tests, Windows installer, GitHub Actions workflow or release has been implemented or validated yet.
- Nine macro-effect identities plus THERAPY (inverted Dry/Wet) and LOSS OF CONTROL (macro randomize only) are **proposed design**, not audited implementation.

## Immediate engineering milestones
- **P001 — Audio reference prototype:** make an automatable, artifact-backed test of macro sound recipes with dry/processed level matching.
- **P002 — VST3 host skeleton:** Windows x64 VST3; host-recalled automation; reliable bypass, macro states, mono/stereo.
- **P003 — Nine macros:** implementation and listening tests one by one with distinct reference sources and objective safety measurements.
- **P004 — Reactive monitor:** licensed historical asset selection with provenance records, accessible motion, no audio-thread work.
- **P005 — Product QA & CI:** Windows GitHub Actions compile/validator/DSP tests, then end-stage DAW listening tests and packaging.

## Release gate
Nothing is considered finished until audible Windows VST3 tests and host recall pass. Do not report a green build as proof that the sound works.

## Repository hygiene
Use only **main**, avoid unnecessary parallel branches, and keep this file up-to-date with actual work/results. Public releases and GUI should use a neutral 125A identity.
