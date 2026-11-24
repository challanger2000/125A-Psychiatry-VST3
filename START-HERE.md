# START HERE — 125A PSYCHIATRY

**Current design authority: P026, confirmed 2026-11-24.** Read [docs/P026-SOUND-DESIGN.md](docs/P026-SOUND-DESIGN.md) in full, then [STATUS.md](STATUS.md), [docs/SPEC.md](docs/SPEC.md) (historical baseline where conflicting), and relevant DSP code before changes. Follow 125A Engineering mandatory reading and guardrails. Do not assume older green tests establish musical quality.

User audition of v0.9.12: some macros convincing above 50% (acceptable), PANIC bird chirps and PSYCHOSIS too weak. DEPRESSION mostly reverb, SCHIZOPHRENIA mostly delay on initial impression. Original placeholder art and minimal MOTION not accepted. No global curve change. Preserve successful parts.

**Next task:** finish verification of P026b PANIC/PSYCHOSIS (P026a standalone passed; Windows two-tone alias gate failed and P026b addresses it), then revise PARANOIA with optional acoustic layers and review the remaining worlds. PSYCHOSIS must be extreme but musically usable EBM/Industrial distortion: asymmetric/hard clipping, real wavefolding, multiband drive, intermodulation, resonant filtering, powerful bass. NO bitcrushing or sample-rate reduction in PSYCHOSIS; bitcrushing generally unwanted. Before implementation, explicitly consider signal routing and whether each component contributes to the intended sound. Then review the other seven and implement DELIRIUM, CATATONIA, AMNESIA. Twelve distinct macros, not superficial labels.

**GUI after sound:** central circular bull's-eye for realistic monochrome imagery, twelve equal-sized knobs in a ring, THERAPY, LOSS OF CONTROL (randomizes only twelve macros), MOTION, DE/EN. Cold, clinical, oppressive blue-gray/steel/anthracite, not warm orange. Replace placeholders and redesign full-field motion.

Only main. v0.9.12 is previous verified technical baseline, NOT an accepted final product. Keep latest tested source and verification status current in STATUS.md. No unnecessary intermediate DAW requests; final real DAW test only when engineering is ready. Never substitute technical metrics for audible judgment.

**New owner-approved sound-layer extension (2026-11-24):** read [sound-library/README.md](sound-library/README.md) and [source manifest](sound-library/sources.json). Selected worlds may blend real atonal recordings reactively with their own DSP; no extra controls. Source recordings are candidates, not proof of musical approval. Preserve 0%/THERAPY dry and silence behavior.

P026c candidate: PARANOIA acoustic layer uses six pinned recordings embedded by tools/prepare_acoustic_bank.py at build time; see docs/P026c.md. Verify native Windows source/host/level regressions before claiming success. Other worlds still require design and review. v0.9.13 candidate not approved.

P026d: P026c Linux decoded all six files and passed 7/7 DSP tests. Windows build failed on interpreter mismatch; CI now pins and passes the exact decoder-equipped Python. Inspect new Actions results before further DSP edits. No final approval.

P026d is now a verified technical checkpoint (v0.9.13, all four CI workflows successful; see STATUS). P026e v0.9.14 candidate replaces OBSESSION bit reduction with progressive filtered/saturated replay; preserve frozen capture and verify new test results. Remaining work: DEPRESSION, SCHIZOPHRENIA, MANIA, DISSOCIATION, HALLUCINATION and three new worlds, then circular GUI. No user audition yet.
