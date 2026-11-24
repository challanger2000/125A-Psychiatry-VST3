# START HERE — 125A PSYCHIATRY

**Current design authorities: P026 sound and P027 visuals, confirmed 2026-11-24.** Read [docs/P027-GUI-DESIGN.md](docs/P027-GUI-DESIGN.md) in full for GUI work. Read [docs/P026-SOUND-DESIGN.md](docs/P026-SOUND-DESIGN.md) in full, then [STATUS.md](STATUS.md), [docs/SPEC.md](docs/SPEC.md) (historical baseline where conflicting), and relevant DSP code before changes. Follow 125A Engineering mandatory reading and guardrails. Do not assume older green tests establish musical quality.

User audition of v0.9.12: some macros convincing above 50% (acceptable), PANIC bird chirps and PSYCHOSIS too weak. DEPRESSION mostly reverb, SCHIZOPHRENIA mostly delay on initial impression. Original placeholder art and minimal MOTION not accepted. No global curve change. Preserve successful parts.

**Next task:** Functionality first (owner decision 2026-11-24). P027a v0.9.21 LOSS OF CONTROL passed all three triggered workflows. P027b passed all three triggered workflows. P027c passed three triggered workflows; P027d v0.9.22 adjusts only SCHIZOPHRENIA/HALLUCINATION spectral blend at intermediate settings. Verify new CI, compare 68 recorded combination rows and individual peaks before further changes; no blanket loudness matching. GUI remains deferred. P026l v0.9.20 passed all four CI workflows; preserve that verified DSP, parameter IDs, state, latency and tail. Complete all-twelve randomizer and monitor. No global macro-curve changes, bitcrushing or sample-rate reduction. Earlier checkpoints below are historical.

**GUI after sound:** [P027 design](docs/P027-GUI-DESIGN.md) is current: dirty COLOR ivory/green/brown tiled faceplate, genuine rendered black Bakelite knob filmstrips with aged ivory dials, central circular image field (B/W or yellowed/sepia fictional horror photography), ivory THERAPY, red LOSS OF CONTROL, MOTION, DE/EN. Do not use old CRT placeholder art or replace real raster artwork with standard vector knobs.

Only main. v0.9.21 P027a is the latest verified controller checkpoint; v0.9.20 P026l is the last four-workflow technical baseline, NOT an accepted final product. Keep latest tested source and verification status current in STATUS.md. No unnecessary intermediate DAW requests; final real DAW test only when engineering is ready. Never substitute technical metrics for audible judgment.

**New owner-approved sound-layer extension (2026-11-24):** read [sound-library/README.md](sound-library/README.md) and [source manifest](sound-library/sources.json). Selected worlds may blend real atonal recordings reactively with their own DSP; no extra controls. Source recordings are candidates, not proof of musical approval. Preserve 0%/THERAPY dry and silence behavior.

P026c candidate: PARANOIA acoustic layer uses six pinned recordings embedded by tools/prepare_acoustic_bank.py at build time; see docs/P026c.md. Verify native Windows source/host/level regressions before claiming success. Other worlds still require design and review. v0.9.13 candidate not approved.

P026d: P026c Linux decoded all six files and passed 7/7 DSP tests. Windows build failed on interpreter mismatch; CI now pins and passes the exact decoder-equipped Python. Inspect new Actions results before further DSP edits. No final approval.

P026d is now a verified technical checkpoint (v0.9.13, all four CI workflows successful; see STATUS). P026e v0.9.14 candidate replaces OBSESSION bit reduction with progressive filtered/saturated replay; preserve frozen capture and verify new test results. Remaining work: DEPRESSION, SCHIZOPHRENIA, MANIA, DISSOCIATION, HALLUCINATION and three new worlds, then circular GUI. No user audition yet.

P026f: P026e DSP/EditorHost passed, Windows audio gain guard failed for drums at +6.789 dB. Corrected wet-only replay gain and added a percussion regression. Verify P026f CI before further DSP changes. Last fully verified v0.9.13.

P026g: P026f v0.9.14 verified all four CI workflows; current DEPRESSION candidate v0.9.15 uses transient attenuation, slow leveling, processed dark body and subtle downward drift, less dominant hall. See docs/P026g.md. Verify all regressions before further edits; resonant filter/granular return not implemented in this pass. Next SCHIZOPHRENIA after DEPRESSION verification. No final DAW test until all worlds and GUI complete.

P026h: v0.9.15 P026g all four CI workflows passed. SCHIZOPHRENIA v0.9.16 candidate replaces the shared long-lag pitch paths with three independent short-grain voices. See docs/P026h.md. Verify all four workflows before continuing. Next MANIA, DISSOCIATION, HALLUCINATION, then DELIRIUM/CATATONIA/AMNESIA and GUI. No final listening acceptance yet.
