# 125A PSYCHIATRY — Verified Project Status

Updated: **2026-11-25**. Latest verified functional baseline: **P027g / v0.9.24**. Current audited candidate: P028j / v0.9.26 — **NO-GO** after independent Codex audit. P028k / v0.9.27 all four CI workflows PASS on 50222758; independent follow-up audit NO-GO; R01–R07 open (P028l). **Only `main`**; no final or musical approval.

## Current task — P028l / v0.9.28 consolidated corrective candidate (NO-GO)

R01–R07 source repairs and focused tests committed, including real Windows DLL native state/zoom/motion/retarget probes, invalid queue and active recall regressions, extracted ZIP validator and strict four-instance hosted deadline CSV. First CI 2ff8a81d: DSP Windows/Linux PASS; actual Windows module compiled and state regression PASS; Windows/recorded audio jobs FAIL compiling new R02 test (C3493 uncaptured constants); GUI did not trigger. Test-only capture/GUI trigger correction pending verification. No new release or Studio Pro approval. See [P028l follow-up](docs/P028l-FOLLOWUP.md).

## P028k v0.9.27 verification — 2026-11-25

Exact product/test commit: `502227585a874dccd84cd5031bbae2092f354ae7`. Four workflows SUCCESS:
- [Windows #45628879558](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45628879558): MSVC x64, Steinberg Validator 47/47, SDK-linked state and actual processor audio CTests PASS. A02 sparse/dense/partition THERAPY max PCM error 0, nonzero active delta 371.865058; A07 sample-aligned bypass oracle max 1.68134769e-7; A05/A06 inactive bus and malformed PCM PASS. A10 three real-bundle tamper negatives rejected (fake module, wrong notice, altered PNG), exact 47 PNG and 3 notices, PE32+ AMD64 and factory/module version checks PASS.
- [DSP #45628879049](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45628879049): 10/10 CTests on both jobs, including scene alpha math; 168-case objective audit step successful on its designated job. This is not a full native four-instance 96 kHz/64-frame realtime acceptance test.
- [Native GUI #45628879179](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45628879179): actual EditorHost, 13 knobs, randomization, bypass/restore, MOTION, timed scene rotation and 768x512 -> 1152x768 PASS. A09 rejected-host and live/multi-view recall are NOT independently verified by this smoke test.
- [Recorded audio #45628879053](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45628879053): SUCCESS.
- Actual [Windows artifact 15030461616](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45628879558/artifacts/15030461616) downloaded and inspected separately in local container. Outer CRC PASS, inner ZIP CRC PASS, 56 members, SHA256 `159dd9f86845291c998740b301dd1d01277041019fda6c15ae3df7d42ff39182` equals CI log, module and both factory classes 0.9.27. No DAW run in container.

**Technical candidate for independent Codex re-audit, not FINAL / not GO for Studio Pro.** Repeat original adversarial A01–A10 probes against exact product SHA, challenge queue bounds and concurrent active recall, native rejected zoom and all six sizes, live recall/multiple editors, realtime deadline behavior, and original audit's remaining gates. No original evidence ZIP available to ChatGPT. Existing new regression tests are self-authored, not independent. 125A Plugin Tester and owner musical/DAW acceptance remain unverified. No product edits or CI in this documentation checkpoint.

## Historical P028k audit corrections

Independent audit confirms nine product findings A01–A09 and package-verifier gap A10. Do not request final Studio Pro test. See [P028k correction gate](docs/P028k-AUDIT-CORRECTIONS.md). Full audit report supplied by owner; evidence ZIP download unavailable. Recreate exact counterprobes as needed. P028k source correction candidate committed; independent original audit counterprobes and target-host acceptance still required.

## P028k verification 2026-11-25 — exact source 8fa1854fd96ce408f9ebdd07022422df158d9602

- [Windows #45624709526](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45624709526): actual MSVC VST3 compiled, Steinberg 47/47, both SDK-linked state/audio regression CTests PASS. Package gate correctly rejected stale factory class version 0.9.26 despite module version 0.9.27; no ZIP published.
- [DSP #45624709552](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45624709552), [native GUI #45624709755](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45624709755), [recorded audio #45624709515](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45624709515) SUCCESS.
- Follow-up P028k retains v0.9.27, derives both factory version strings from CMake, extends package negatives against temporary real-bundle copies. Also addresses explicit Steinberg warning about missing output silence flags (accumulate across all spans, leave inactive bus unchanged); tests silent and mixed-channel active output. No creative DSP changes. New exact-SHA CI pending.
- Independent original audit replay, A09 host refusal/live recall/multi-view coverage, full 125A Tester, measured RT and Studio Pro remain open. NO-GO.

## Historical P028j verified technical candidate

Native GUI test [45536098801](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45536098801) SUCCESS: exact 50→75% resize, all 13 knobs, MOTION, timed scene rotation, static hold, bypass and randomization. Windows package artifact 14986961132 from unchanged P028i product, all 40 scenes. Final Studio Pro audition and higher zoom factors not yet verified. See [P028h/P028j evidence](docs/P028h.md#p028j-verified-result--2026-11-25).

## Historical P028j GUI-only desktop correction

P028i native 75% item selected, window enlarged but clipped to 1028x749 on actual 1024x768 desktop. Windows/DSP PASS. P028j adjusts test desktop to 1920x1080 and retains exact geometry gate; no product changes. See [P028h/P028j](docs/P028h.md). GUI-only retest pending.

## Historical P028i zoom verification

P028h Windows, DSP and recorded workflows PASS; native EditorHost reached final zoom check after all motion/scene/knob probes passed, then failed (unchanged 768x512). P028i synchronizes menu value/index and clicks the actual native 75% item with screenshots and unchanged geometry assertion. No DSP or scene changes. See [P028h/P028i](docs/P028h.md). New GUI and Windows verification pending.

## Historical P028h scene library integration

40 uploaded scenes mapped to twelve worlds. See [P028h](docs/P028h.md). Timed rotation, motion/static and zoom probes integrated; no DSP or approved artwork changes. New CI pending.

## Historical P028g verified raster candidate

All four CI workflows PASS on exact source `87be28cb8f4d5c34a9a0bc164e4eda06ec6b03f8`; actual VST3 packaged and ZIP checked, native screenshots visually examined. See [verification evidence and artifact hashes](docs/P028d.md#p028g-verification-result-2026-11-25). Remaining: complete scene selection, actual MOTION/zoom interaction checks, subjective sound and Studio Pro acceptance. No further build for documentation alone.

## Historical P028g package verification update

P028f Windows compilation, 47/47 Validator, native GUI smoke (13 knob drags, randomize, bypass/restore), DSP and state/audio CTests PASS. Overall Windows job FAIL at obsolete legacy package assertions. P028g updates package checks to the approved English raster layout with all 11 exact PNG hashes. No product changes; corrected package job pending. Final scenes, visual/motion/zoom and Studio Pro acceptance remain open. [Evidence](docs/P028d.md).

## Historical P028f SDK compile correction

P028e: DSP and recorded audio SUCCESS; Windows and EditorHost configure/hash checks PASS, compile FAIL (eight C2668 ambiguous `owned` calls). Qualified all eight with `VSTGUI::owned`; no audio or artwork change. [Evidence](docs/P028d.md). Corrected Windows compilation and native interaction pending.

## Historical P028e checkout correction

P028d DSP PASS, Windows/EditorHost/recorded configure FAIL on README.txt SHA256. Local autocrlf negative reproduction and positive exact-byte checkout PASS with `.gitattributes`; no product changes. See [P028d/P028e evidence](docs/P028d.md). Await native verification of this candidate.

## Historical P028d native raster editor candidate

See [P028d](docs/P028d.md). Artwork fully transferred and hash-verified; P027g DSP baseline retained. Native build/host evidence not yet available. Five provisional scenes; no final acceptance.

## Historical P028a layout review

See [geometry draft](docs/gui-layout/README.md). P027g passed all four workflows; functional baseline frozen. No production changes. Owner review of exact geometry before art.

## Verified P027g integrated functionality gate (factory v0.9.24 unchanged)

P027f exact source `88254962264c1d361f4b09a8e631f114e5ceeb75`: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45479875147), [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45479875048), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45479875182), [recorded](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45479875053) all SUCCESS. Twelve-world monitor regression and provisional mapping verified by their existing gates; unique final images and twelve physical controls not yet integrated.

P027g adds actual controller -> IComponentHandler edits -> SDK host parameter queues -> actual Processor::process -> saved component/UI state -> new processor/controller -> bit-exact restarted audio. Start from 12 active worlds; assert explicit zeros, real live audio change, preserved THERAPY/bypass/language/motion, exact restored values (float state precision), non-silent matching audio after reset. No production or graphical changes. All four workflows passed. Functional baseline is frozen for the approved P027 final GUI implementation; do not keep adding unrelated audit-only phases or change DSP without specific evidence. Musical and final Studio Pro acceptance remain unverified.

## Historical candidate — P027f / v0.9.24 twelve-world image state

P027e exact source `1cab684822bcf482c3076397bc50b7c9c27ea710`: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45477278047), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45477278151), [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45477278113), [recorded](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45477278139) all SUCCESS. 32 actual generated preset renders, 3–6 active worlds, raw gain -9.198 to -3.604 dB, gain-matched residual .40724–.96848, maximum peak -5.001 dBFS. These are eight fixed generator draws across four recordings, not a universal gain guarantee; different from previous dense test vectors. Validator 47/47, 60/60 substantive full wet, no weak cases. Actual VST3 12-world realtime factor .1029 at 85%, .0925 at 100%, .0638 neutral (runner-dependent, not the owner's PC).

P027f extends functional visual-state model to all twelve IDs, effective levels, primary and secondary selection, deterministic tie behavior, bypass and THERAPY. Existing diagnostic renderer reuses three old images as explicit placeholders, safely mapping all 12 logical indices to 9 bitmap slots; no final design or new artwork. Tests check isolated controller values for every world, bounds, ties, equality and dry/bypass. Existing custom editor still has nine knobs; generic host has all 12; final custom controls and image library remain deferred. DSP untouched. All four CI workflows subsequently passed.

## Historical candidate — P027e / v0.9.23 randomizer density

P027d exact source `181bf2f8f6dd15c1b64303a1a9c67a81dfca7663`: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45474084089), [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45474084135), [recorded](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45474084014), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45474084126) all SUCCESS. Recorded prefix2 gain recovery 1.938–1.939 dB; full twelve at 50% now -17.686/-17.602/-17.054/-16.108 dB vs -20.624/-20.780/-21.318/-19.377 dB (drums/guitar/vocal/machine), improvements 2.938/3.178/4.264/3.269 dB. Mixed dense vectors now -9.914 to -14.268 dB. 68 combination rows, 240 individual rows, 96 A/B PASS. Still substantial intentional/cumulative attenuation. No human listening approval.

P027e changes LOSS OF CONTROL only: select 3–6 of 12 worlds per press, nonzero amounts retain .12–.78, all others explicitly zero; all 12 host IDs still written, same group edits and untouched globals. Manual all-twelve setting remains possible. This is an engineering design choice to reduce dense random chain attenuation and improve differentiation, not a user-mandated exact density. The prior randomizer activated each macro with probability .82. Controller test now checks density; recorded QA additionally renders 8 seeded actual generator presets per 4 sources (32 cases) and records values and levels. No new gain gates or DSP/GUI changes. All four workflows subsequently passed.

## Historical candidate — P027d / v0.9.22 spectral midpoint correction

P027c exact source `c7f424092fed2465df227c4f148e0a1c1095f5e5`: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45470028527), [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45470028540), [recorded](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45470028649) SUCCESS. EditorHost did not trigger. 68 recorded combinations, combined THERAPY endpoints/midpoint and bypass PASS, 240 individual rows/96 A/B. All 12 at 50%: -20.624/-20.780/-21.318/-19.377 dB (drums/guitar/vocal/machine). Mixed vectors -11.618 to -16.382 dB. Prefix stage 2 SCHIZOPHRENIA -2.384 to -6.286 dB, stage 8 HALLUCINATION -4.414 to -4.969 dB. These are interaction-dependent, not isolated gain.

SCHIZOPHRENIA and HALLUCINATION only: partial power compensation of their inner dry/transformed crossfades, 0 and 100% unchanged, +1.938 dB at 50%, no dynamic normalization or feedback gain. This is a conservative mixing-law hypothesis, not proof of the cause of all attenuation. Preserve creative CATATONIA/AMNESIA loss, THERAPY and all other algorithms. Added analytic regression and pinned real-processor prefix 2 improvement oracle; all existing gates unchanged. All four CI subsequently passed as recorded above; no local build or audition claimed.

## Historical candidate — P027c actual recorded combination audit (factory v0.9.21 unchanged)

P027b exact source `2ec24a14954f5d34f8376f1fe06fa2c371b08ca1`: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45468719450), [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45468719431), [recorded](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45468719557) SUCCESS. DSP 8/8 on each OS; actual VST3 47/47 validator; 12 P019 host integrity scenarios PASS; 60/60 substantive full wet; recorded 240 rows and 96 A/B PASS. EditorHost did not trigger (no GUI/production changes). P006 168 cases, 15 review flags, zero safety-level violations. Full twelve at 50%: bass -20.16, drums -20.43, voice-like -21.82, atmosphere -19.93 dB. These are synthesized stimuli, not all source behavior. Nine-chain 50% bass -13.67, drums -12.23 dB. Remaining weak-onset PANIC and DEPRESSION flags are review items, not a reason to globally increase macro curves.

P027c measures prefix gains in actual VST3 routing at 50%, full chain 85%, four fixed mixed settings per recorded source (68 rows). Verify combined THERAPY dry/midpoint and bypass; raw audition files where PCM16 would not clip. No product DSP changes or blanket loudness compensation; diagnose cumulative gain and creative attenuation before choosing targeted corrections. CI subsequently passed as recorded above.

## Historical candidate — P027b expanded functional audit (same factory v0.9.21)

P027a exact source `c94be5023b3e78dda3589d3ab07512d8ab817f0b`: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45467293116), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45467293166), [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45467293117) SUCCESS. Native controller/state CTest (including the 64-press randomizer test) passed; validator 47/47, endpoint matrix 60/60, dense automation and nine/twelve tail tests passed. Recorded workflow did not trigger for P027a; no new recorded evidence. Previous full four-workflow checkpoint P026l remains applicable to unchanged DSP source.

P027b extends real processor host integrity across 9/12 effects, mono/stereo, 44.1/48/96 kHz, variable and oversized blocks, in-place, independent instances, restart, exact compensated dry and audible-effect delta. Standalone P006 measurement matrix now 168 cases (12 singles plus old nine and new twelve chains, four sources, three intensities); original level gates unchanged. Recorded workflow also watches actual processor and test changes. No production code or factory version changes. CI subsequently passed as recorded above. No human listening or final GUI approval.

## Historical candidate — P027a / v0.9.21

LOSS OF CONTROL previously addressed only nine IDs. Now edits all twelve, retains existing 0 or .12–.78 distribution, groups host begin/perform/end edits and preserves globals/UI settings. Real controller test checks 64 presses, all 12 IDs and 36 callbacks each, nonzero activation, exact reported values and untouched THERAPY, bypass, language, MOTION. No DSP, state version, parameter identity or GUI layout changes. Native CI subsequently passed as recorded above. Existing diagnostic monitor and custom nine-knob layout intentionally remain provisional; all 12 in host parameter list. Owner explicitly deferred final graphic implementation until functions are ready.

## Current checkpoint — P026l / v0.9.20 verified; P027 visual design

P026l exact source `0f0a4d66bfc5bde3c9a6ee1054e80619240f8f82`: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45463559947), [recorded](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45463559848), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45463559886), [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45463560015) all SUCCESS. Windows validator 47/47; actual processor substantive full-wet 60/60, WEAK=0, gain-placebo and sample-accurate automation PASS. P023 nine- and twelve-world excitation/tail PASS at 44.1/48/96 kHz. Twelve-world active peaks .33882/.50740/.48635, first-tail peaks .30756/.26496/.28322. Technical verification only, not final musical or DAW approval.

Next: implement the [P027 GUI](docs/P027-GUI-DESIGN.md). Large physical observation window, rusty pipes and fan, original raster filmstrips and simple motion. Preserve verified audio and host contract.

## Historical checkpoint — P026l correction candidate

P026k exact source `485223691aa1f399df439ed51dc2a08d10ac1dac`: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45460684862), [recorded](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45460681915) and [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45460682047) SUCCESS; [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45460681890) FAIL in P023 all-twelve meaningful excitation/tail assertion. MSVC plugin and legacy/v5 state passed. No full approval.

P026l adds CATATONIA stronger-source reacquisition to avoid locking an almost silent leading edge, a focused quiet-onset regression, and detailed tail diagnostics without relaxing gates. See [P026l](docs/P026l.md). All candidate CI subsequently passed as recorded above. The new [P027 design](docs/P027-GUI-DESIGN.md) is owner-approved visual direction; original art archive preserved in chat, not yet uploaded as repository binaries. No filmstrips/layout yet. Sound verification first.

## Historical checkpoint — P026j / v0.9.18 verified; P026k / v0.9.19 twelve-world audio candidate

P026j exact source `a3d805afdccd6daf623b2a211a7bb83961d67b1b`: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45458793401), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45458793305), [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45458793490), [recorded audio](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45458793320) all SUCCESS. Cumulative MANIA and DISSOCIATION recorded-source gate passed. Musical audition remains open.

P026k is one consolidated sound-first batch: HALLUCINATION resonant source-derived voice/reverse diffusion, DELIRIUM variable-speed history reads, CATATONIA phase-propagating FFT freeze, AMNESIA time and band erasure. All twelve DSP engines wired into VST3 with new IDs 109–111, v5 appending three floats after the legacy v4 prefix, and host automation. Existing nine-knob custom GUI, monitor and randomizer are intentionally provisional; the three new parameters are visible in the host generic parameter list. The twelve-knob GUI and full twelve-world randomizer/monitor are the next separate visual milestone, not yet implemented. New tests and expanded 12-world actual processor/recorded-source matrix. All four candidate CI results pending; exact-blob isolated C++17 new-world tests and three negative variants executed as documented. No local full checkout or human listening claim. See [P026k](docs/P026k.md).

## Historical checkpoint — P026i / v0.9.17 verified in three workflows; P026j / v0.9.18 candidate

P026i source `02a91c5fa837e816c99b83fe80a155978673f6d3`: [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45457364438), [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45457364257), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45457364319) SUCCESS. Recorded audio workflow did not trigger because the path filter omitted three_worlds; not verified for this SHA. Correct filter to src/** so all DSP updates run it. Do not call v0.9.17 a four-workflow pass.

P026j DISSOCIATION candidate: two overlapping immutable forward/reverse grains, varied causal history offsets, stereo decorrelation and two short diffusion allpasses. No permanent static freeze or hard grain gaps. [Design and tests](docs/P026j.md). All four CI workflows and musical acceptance pending. Next HALLUCINATION, then DELIRIUM, CATATONIA, AMNESIA and finally GUI.

## Historical checkpoint — P026h / v0.9.16 verified; P026i / v0.9.17 candidate

P026h exact source `d1fa2c6e4f4ac360866105672ba960fa9c0deac6`: [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45447624240), [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45447622118), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45447622131), [recorded-source audio](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45447622166) all SUCCESS. 47/47 validator and 7/7 standalone groups. Quality CSV: seven review flags, zero safety-level violations. Four 20% weak-onset flags (DEPRESSION bass; PANIC bass, voice, atmosphere), one 50% PANIC voice flag, two combined 50% all-nine attenuation flags (bass -13.4695 dB, drums -14.0969 dB). None concern SCHIZOPHRENIA. Do not globally boost macro curves. Combined level balance remains open. No musical approval.

P026i MANIA: actual band-limited two-repeat retriggers, common stereo clock, moving resonant bandpass, existing oversampled chopper/ring/feedback; see [design and verification](docs/P026i.md). Full CI pending. No other effect algorithms, GUI, fixed latency, state or parameter IDs changed. Next DISSOCIATION after verification, then HALLUCINATION, three new worlds, and GUI.

## P026 — historical revised sound-design baseline

User audition of v0.9.12 confirmed several usable effects above 50%, but rejected the character of PANIC and PSYCHOSIS; DEPRESSION and SCHIZOPHRENIA require character review. Placeholder artwork and current MOTION are not accepted. No global macro-curve boost requested. [P026 sound design](docs/P026-SOUND-DESIGN.md) is the confirmed 12-world design and takes precedence over older specification text: brutal EBM/Industrial PSYCHOSIS without bitcrushing/sample-rate reduction; bitcrushing generally disfavored; cold clinical circular 12-knob GUI with central bull's-eye, THERAPY and LOSS OF CONTROL. Detailed effect topology and audible differentiation must be considered before implementation. Sound first, GUI later. Existing verified v0.9.12 remains the technical reference, not an accepted final product.

## Latest verified build and fixed references

- **Source commit tested:** `eedbb5ca9bef415bd40e96044f4a4a921bfa0a2d` on **main** (P024, factory v0.9.12).
- **Windows/MSVC x64 real VST3 workflow:** [run 45309769352](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769352), **SUCCESS**.
  - Steinberg VST3 SDK 3.8.1 validator: **47/47 PASS, 0 FAIL**.
  - Actual VST3 processor/controller state regression: **1/1 PASS** (historical state v1, v2, v3, v4, plus NaN, truncation and future-version rejection). **Not a DAW project reopen test**.
  - [P024 Windows VST3 bundle artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769352/artifacts/14882642144): `125A-PSYCHIATRY-P024-CRT-NOT-FINAL` (7-day retention, expires 2026-12-01).
- **Windows + Linux standalone audio workflow:** [run 45309769415](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769415), **SUCCESS**.
  - **7/7 CTest groups passed on each platform**. Includes all nine concurrent DSP engines and callback no-heap allocation test, plus precomputed Hilbert tap coefficient regression.
  - **P006 objective quality audit:** 120 sound/source/intensity cases, **0 review flags, 0 safety-level violations** after addressing measured issues. **P010 adds a separate 180-case test of the actual VST3 processor, not merely standalone DSP**.
  - [P011-quality and CPU metric methodology](docs/P011.md); metrics include wet/dry RMS ratio, dBFS peak, normalized delta, DC, maximum single-sample step, and runner-local render time.
- All Windows Actions artefacts are temporary engineering packages, not release installers.



## P026h — SCHIZOPHRENIA candidate v0.9.16 (verification pending)

[Design and test scope](docs/P026h.md): independent three pitch voices, much shorter causal readhead delays, separate spectral coloring, stereo divergence and bounded pairwise nonlinear products. No other macro, PDC, GUI or state change.

## P026g — v0.9.15 technical checkpoint

Exact source 78b239a654c187e3525fff31133f4c8be53a769e: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45446438095), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45446438026), [actual Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45446437930), [recorded source audio](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45446437990) all SUCCESS. No human audition or final acceptance. DEPRESSION's proposed resonant filter and granular return remain deferred pending musical assessment.

## P026g — DEPRESSION candidate v0.9.15 (verification pending)

[Implementation and test scope](docs/P026g.md): foreground transient attenuation, leveling and dark shelf, dual-readhead downward drift, less dominant FDN. No GUI or other world changes. P026f is the verified technical reference.

## P026f — v0.9.14 verified technical checkpoint

Exact source 81a0e821e41a22236f0bfbefe565ba99ae21b4c1: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45442484081), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45442484160), [recorded sources](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45442484382), [actual Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45442484881) all SUCCESS. Steinberg 47/47, 45/45 substantial full-wet cases, prior alias, phase, tail and state gates passed. OBSESSION percussion gain +5.07 dB (previous +6.789). Latest technical checkpoint, not a musical or Studio Pro acceptance.

## P026f — OBSESSION percussion gain correction (v0.9.14, pending verification)

P026e exact source 62035ceec9eb99a9d4477f3bcf27f2384a84b575: [Windows/Linux DSP PASS](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45436494194), [EditorHost PASS](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45436493940), [actual Windows VST3 FAIL](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45436494016). Steinberg validator 47/47 and legacy state passed, as did the earlier audio-path gates; the full-wet P011 level guard failed on drums at +6.789 dB (limit +6 dB). Apply a continuous wet-only trim of 0.82 at 100% (-1.724 dB relative to P026e; predicted drum gain +5.065 dB, not yet measured). Preserve capture, filter, nonlinear transfer and fade. Add the exact P010 sparse percussion fixture to standalone OBSESSION regression; no thresholds weakened. Re-run Windows actual VST3 and recorded-source export. Latest fully verified checkpoint remains P026d/v0.9.13; no musical acceptance claimed.

## P026e — OBSESSION continuous timbral wear / v0.9.14 candidate

Remove the old repeat-dependent 6–16-bit quantizer. Preserve immutable capture, grain length, repeat count, fades and neutral/fully wet semantics. The replacement is a repeat-dependent one-pole lowpass (6200 Hz towards ~713 Hz as wear approaches one), gradual blend up to 80%, continuous tanh shaping and bounded gain. The intent is compulsively repeated material that becomes increasingly muffled and strained, not bitcrushed. No change to other engines, GUI, latency or state. Tests retain frozen-grain controls and add high/low spectral-bin ratio at early vs late repeat plus low-level amplitude scaling. These are objective proxies, not musical acceptance. No tests executed on P026e yet.

## P026d — v0.9.13 verified technical checkpoint

Exact source c103cffb1d9811b4697dedd1e47215999545a8a1: Windows/Linux DSP [45434439617](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45434439617) SUCCESS; Windows actual VST3 [45434439087](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45434439087) SUCCESS; EditorHost [45434439260](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45434439260) SUCCESS; recorded-source audio [45434439426](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45434439426) SUCCESS. This includes the previous alias gate and embedded PARANOIA sources; no subjective approval or Studio Pro acceptance. Latest technical baseline v0.9.13, not final.

## P026d — Windows build interpreter correction (verification pending)

P026c exact source d314dda1d9e8ee5c7530ce54f32993256077c603: Linux decoded six archived OGG files with matching hashes and 7/7 CTests passed, including the new acoustic player test and no-heap combined chain. Windows standalone, VST3, EditorHost and recorded-audio builds failed before compiling the acoustic engine: pip installed imageio-ffmpeg 0.6.0 into PATH Python, while CMake selected hosted Python 3.14.10/python3.exe without the module. See Actions 45431442142 / 45431442305 / 45431442219 / 45431442186. Correct all four build workflows by explicitly selecting Python 3.12 with setup-python, passing its exact python-path output to CMake, and checking decoder availability at configure time. No DSP changes or weakened test oracles. Native Windows VST3 audio, alias correction and acoustic bundle verification remain unverified; v0.9.13 not final.

## P026c — PARANOIA embedded acoustic layer candidate / factory 0.9.13 (not yet verified)

[Implementation and scope](docs/P026c.md). Six source recordings are selected from the 23 archived OGG files; build-time hash-verified PCM generation, reactive playback and actual PARANOIA wet/cross-feedback routing. No new controls or runtime codec. Native build/test results pending. Last fully verified source remains P024/P025 v0.9.12.

## P026b — PSYCHOSIS anti-alias correction candidate (verification pending)

P026a source fc48623e26fc2a8e1ed6464260c073a1f9aec224: [DSP suite PASS](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45425652865), [EditorHost PASS](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45425652906), [Windows VST3 audio gate FAIL](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45425652866) after 47/47 Steinberg and legacy state PASS. The 48 kHz two-tone 2x15k+14k folded line exceeded -45 dB(input). Add a second pole before the heavily nonlinear midband; keep complementary high residual and fixed 24-sample stage latency. No test thresholds changed. Numerical replica (not compiled VST3) reproduced -44.54 dB and predicts -89.92 dB for that line after correction, with 55 Hz fundamental 0.37382 -> 0.37633, H3 0.01828 -> 0.03167 and H5 0.08231 -> 0.07036. Actual Windows verification still pending.

## P026a — new PANIC / PSYCHOSIS DSP candidate (partially verified)

The first implementation pass is in `src/three_worlds.*`, with focused tests and [design/verification notes](docs/P026a.md). No GUI or other world changes. P024/P025 remain the last verified baseline; the new candidate is not accepted or released.

## Actual audio code in the VST3

**All nine sound-world prototypes are implemented**: DEPRESSION, PARANOIA, SCHIZOPHRENIA, MANIA, PANIC, DISSOCIATION, OBSESSION, HALLUCINATION and PSYCHOSIS. Each is a dedicated compound DSP engine, not an inert parameter. Distortion is **one ingredient inside relevant effect worlds**, especially PSYCHOSIS, rather than the universal plugin core.

Processing order: DEPRESSION → SCHIZOPHRENIA → MANIA → PANIC → PARANOIA → DISSOCIATION → OBSESSION → HALLUCINATION → PSYCHOSIS → **one global inverse THERAPY Dry/Wet mix**.

- **P009 verified CRT interface:** Steinberg **VSTGUI**, dark 1004×570 panel, two EN/DE templates, nine live FX knobs, THERAPY, Bypass, LOSS OF CONTROL, percent readouts, language switch and original software-drawn **monochrome diagnostic CRT**. CRT scenes and cross-macro signal corruption are calculated on the UI thread from the actual controller parameters. No patient photos or third-party graphics are used.
- VST3 `createView(kEditor)` now constructs `VSTGUI::VST3Editor`, rather than returning null and falling back to host generic controls.
- **Official Steinberg EditorHost GUI interaction PASSED:** [run 45269301923](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301923). An actual Win32 GUI was created, a mouse click on LOSS OF CONTROL changed nine effect values and the CRT image (482 sampled pixels differed, RGB delta 81,702), and the subsequent DE language switch retained a working editor. [Three run screenshots](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45269301923/artifacts/14864592486) were visually inspected. **This is still not Studio Pro or an audio listening test.**
- Stereo/mono, 32-bit floating point input/output, deterministic state per instance; sample-rate dependent buffers allocated in preparation. CPU and realtime allocation *suite* exists, but actual end-user DAW latency/CPU stress still needed.
- VST3 factory version now **0.9.12**, actual component/controller IDs and ParamIDs unchanged. Project state **v4**, with decoding of v1/v2/v3.


## P025 — verified recorded-source audit

[Actual Windows processor recording audit](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45312252550) **SUCCESS**, test source `63544cf1ae6dd0d0037f149943bd956521420c7f`. All nine source file Git hashes verified. Four arranged recorded inputs (drum kit, acoustic guitar, breath/laugh vocal foley, machine-room ambience), 5/10/20/50/100% on each of nine effects = **180/180 measurement rows** and **72 RMS-matched A/B WAVs**. Peak maximum −1.943 dBFS; RMS gain range −5.149 to +3.576 dB; all finite. Minimum full-wet gain-matched residual 0.42477 (recorded guitar/PSYCHOSIS). At 20% weakest measured residual 0.02474 (recorded guitar/DEPRESSION); subjective low-position strength is not established by measurement alone. [Recorded comparisons and attribution](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45312252550/artifacts/14882529933). No claim of human audition, sung/spoken vocal or recorded bass tests.

Git comparison from P024 verified source to P025 test source confirms no changes in src/, vst3/, resource/ or CMakeLists.txt; the actual product remains the verified v0.9.12 P024 bundle, with P025 QA extensions only. No more speculative DSP changes before actual musical/DAW feedback.

## P024 — verified original grayscale CRT artwork

Factory **0.9.12**, tested `eedbb5ca9bef415bd40e96044f4a4a921bfa0a2d`. [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769352), [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769415), [Win32 EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769462) all **SUCCESS**. Nine original grayscale PNGs packaged. Real editor sampled 186 distinct grayscale levels in the image region, proving raster artwork rather than vector fallback. Actual bypass/restore brightness 2840 → 124 → 2840; randomizer and language switch passed. 47/47 validator, 45/45 full-wet, tail and stress regressions passed. [GUI screenshots](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769462/artifacts/14883140743). [Metrics](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45309769352/artifacts/14882781951). Art is fictional procedural rendering, not archival photography. P025 recording audit passed.

## P023 — verified host tail declaration

Factory **0.9.11**, tested `4b3a5c9e8401ac94cdaaa34b8f06048fb2d0636d`. [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45307623931), [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45307623916), [Win32 EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45307623955) all **SUCCESS**. Reported tail = 30 seconds + 96 samples. Actual all-nine full-wet decay test at 44.1/48/96 kHz: initial silence exact, first tail second peak 0.5453/0.5238/0.5450; tenth second below −102 dBFS, thirtieth below −290 dBFS. No artificial truncation. 47/47 validator, 45/45 wet reference, previous automation and phase guards pass. [Metrics](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45307623931/artifacts/14881577646). Studio Pro acceptance remains open.

## P022 — verified control-driven CRT and branded bundle

Factory **0.9.10**, tested `6a74747fddb21129f37baadb8c7db2a33c4a45ad`. [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45306570917), [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45306570973), [Win32 EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45306571192) all **SUCCESS**. 47/47 validator, 45/45 full-wet, dense automation at three rates passed. Real GUI bypass brightness sum 2340 → 240 → 2340, randomizer and language switching pass. Correctly named `125A-PSYCHIATRY.vst3` x64 bundle and both upstream license notices verified, plugin-only ZIP separated from measurement artifacts. [GUI screenshots](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45306571192/artifacts/14881526267). [Metrics](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45306570917/artifacts/14880867925). Still not archival visual fidelity or Studio Pro sign-off.

## P021 — verified dense actual VST3 automation stress

Factory remains **0.9.9**; tested `7b542769b684cc8a67624651ccf19c566aec4a01`. [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45305211996) and [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45305212003) all **SUCCESS**. All nine effects and THERAPY receive 11,723/12,763/25,503 deterministic automation events at 44.1/48/96 kHz. Output is bit-identical across fixed and irregular block layouts, including native bypass and exact compensated THERAPY dry. Peak magnitudes 0.508267/0.516637/0.528570. Three seconds of silent input exercised tails. 47/47 validator, 45/45 full-wet and all prior regression gates passed. [Metrics](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45305211996/artifacts/14880422491). P022 UI/bundle candidate has no DSP changes.

## P020 — verified OBSESSION frozen loop

Factory **0.9.9**, tested `4fd85a9d1e5c1ce69b7860bef8435a10f1a1bb27`. [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45300506720), [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45300506694), [Win32 EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45300506798) all **SUCCESS**. Validator 47/47, full-wet 45/45, automation endpoints 54/54, mono/stereo host integrity passed. Full-wet immutable grain replay at 44.1/48/96 kHz passed. OBSESSION gains for bass/drums/guitar/voice-like/atmosphere +1.21/+5.24/−0.65/+0.03/+1.60 dB. [Metrics](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45300506720/artifacts/14879445094). P021 stress is now triggered.

## P019 — verified engine integrity

Factory **0.9.8**, tested `b005ed084f674e2b0430867d28bc139e75f87f44`. [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45297895778), [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45297895678), [Win32 EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45297895606) all **SUCCESS**. Validator 47/47, full-wet 45/45, automation endpoints 54/54; actual mono/stereo, variable block, in-place, isolated instances and deterministic restart passed. SCHIZOPHRENIA atmosphere full-wet gain corrected to −7.56 dB without reintroducing the dry startup. DISSOCIATION immutable replay and continuous PANIC tremor pass regressions. P018 phase and alias limits retained. [Metrics](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45297895778/artifacts/14877471849).

## P018 — verified phase-aligned oversampling and fixed PDC

Factory **0.9.7**, tested `1cb6df8d9462c6edf4a673151dc7ca4a3203afdc`. [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45295639573), [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45295639454), [official Win32 EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45295639417) all **SUCCESS**. 47/47 validator, 7/7 standalone DSP groups per OS, 0/120 quality flags, 54 automation endpoints, 45/45 full-wet reference cases.

- Four nonlinear stages use symmetric 49-tap polyphase FIR 2x oversampling, each aligned to 24 host samples. Fixed total **96 samples** (~2 ms at 48 kHz), reported to host; exact neutral/bypass/THERAPY after PDC; delayed parameter routing.
- Across 144 phase observations worst cancellation improved **−23.55 to −2.28 dB** (DEPRESSION/96k/1k); MANIA/PANIC/PSYCHOSIS worst −0.47 dB. The 24 compensated impulse measurements include small FIR pre-ringing, raw output is causal. Worst folded two-tone line −52.93 dB(input); existing alias gates passed.
- Reference CI all-nine 100% CPU 0.0641× realtime, max callback 0.4092 ms (not Studio Pro performance).
- [Metrics](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45295639573/artifacts/14876258244) · [development bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45295639573/artifacts/14875973051). Remaining: additional engine behavior, recorded musical audition, long-session/multiple-instance actual VST3 tests, final GUI and Studio Pro acceptance.

## P017 — exact automation convergence verified

Factory **v0.9.6**, tested commit `79fa77c297933b31cb5b81acefa5ad9d4a3345bf`. [Windows VST3](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45293157519) and [Windows/Linux DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45293157516) **SUCCESS**. All 54 automated macro/THERAPY wet-to-dry transitions at 44.1/48/96 kHz reached exact Dry after the smoothing window, including tiny and above-unity finite samples. Steinberg 47/47, full-Wet comparisons 45/45, all earlier alias/phase-diagnostic and audio tests passed. [Developer bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45293157519/artifacts/14875257365). P016's measured partial-mix phase cancellation remains open until a phase-aligned chain is tested.

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

## Final target-DAW acceptance

- **Recorded-source audit P025:** real sampled drums/acoustic guitar, vocal foley and machine-room ambience; objective measurements and RMS-matched A/B export are complete and passed. Vocal foley is not sung or spoken voice. Synthetic audio, alias/intermodulation, PDC, exact endpoints, three-rate automation, tail decay and allocation/CPU regression tests have already passed in P024.
- **Visual:** all nine original fictional grayscale scene bitmaps are loaded in the actual Windows GUI; no third-party or archival images are bundled. Manual inspection of P024 EditorHost screenshot confirmed the new CRT art, legible controls and complete panel. The original photographic reference is not claimed to be reproduced exactly.
- **Final host acceptance:** only actual Studio Pro session listening, recall and automation in the target DAW can establish end-user acceptance. No completed DAW test is claimed.
- **Distribution:** deliver the inspected P024 v0.9.12 portable candidate for the final target-DAW test. No public final release, installer or final listening approval claimed. No test recordings in plugin bundle.

## P024 package inspection

Downloaded actual artifact 14882642144 and inspected independently: ZIP CRC valid, exactly one branded VST3 bundle with PE32+ AMD64 module, matching 0.9.12 metadata, stable IDs, nine decodable 224×196 grayscale PNGs, UI resource, both SDK license notices. No QA recordings or personal developer identity in package metadata. System/VC++ runtime dependencies only; no external host required.

ZIP SHA-256: `24626e65ffc0282036cfde16db341b1b3fbc0edb2c197db1b826464e311015de`
Module SHA-256: `32357f529914b572a6d2deaf6e02b01b526a776ddbf5ae24161906e12c47f4b3`

## Project rules

Keep **only `main`**, keep every verified last-tested commit and Actions links in this file; distinguish synthetic objective QA from actual audible approval. Release UI, packages and product metadata use the neutral **125A** brand; do not surface personal developer identity.

P022 initial Windows runs 45305541109 and 45305541179 failed before plugin compilation: upstream SDK resource custom command splits the spaced package path (MSB8066). Standalone DSP 45305541318 passed. Use `125A-PSYCHIATRY.vst3` for the bundle and module; factory display name remains `125A PSYCHIATRY`. Rerun Windows and EditorHost, no DSP changes.

### P028k — erste Implementierungsgruppe (nicht abgenommen)
A01: kohärente Audio-Snapshots und separate Recall-Mailbox, Veröffentlichung pro geänderter Callback-Generation. Recall wird am nächsten Callback-Beginn übernommen; laufende alte Callback-Automation kann einen neueren Recall nicht überschreiben. A02: lineare VST3-Kurven mit vorherigem Sample als implizitem Anker, Bypass diskret. A03: expliziter Processing-Lifecycle. A04: strikter Stream-Status und Bytezahl, Wireformat unverändert. A05: Bus-Aktivität respektieren. A06: nichtfinite Samples an Eingangsgrenze zu Null. A07: 5-ms linearer Bypass-Übergang, bestehende THERAPY-Glättung unverändert. A08: mathematisch korrekte Source-over-Überblendung. A09: Zoom nur nach angewandtem Frame-Transform speichern. A10 und eigenständige gezielte Gegenproben noch offen. Alte Step-Fixtures explizit verankert, Busse in Host-Fixtures aktiviert. Noch nicht kompiliert oder abgenommen; **NO-GO**. Keine CI bis konsolidierter Kandidat mit neuen Regressionen.

### P028k / v0.9.27 — consolidated verification candidate
A01–A09 initial source repair in 8170b90870f57e26fbc89dac0e1dfe018856393c. Follow-up adds exact-duration bypass ramp, deterministic reentrant save test, concurrent coherent saves, active recall, full-count stream error injection, sparse/dense/partitioned THERAPY PCM and bypass oracle, inactive bus/nonfinite PCM tests, mathematical scene alpha test, A10 PE/notice/moduleinfo checks, actual staging ZIP byte comparison. Native rejection of zoom, concurrent recall/automation ordering beyond the chosen callback-boundary policy, every original Codex counterprobe, Windows compile/test and deadline tests remain gates; **NO-GO until independent verification**. Five-ms bypass ramp is intentional engineering correction; no creative DSP topology or artwork changes. Packaging now provides the verified inner ZIP in the GitHub artifact. No original Codex evidence ZIP available.

## P028l — follow-up audit correction in progress
Independent P028k follow-up audit confirms A01/A03/A04/A05/A06 locally, A02/A07/A08/A09 partial failures, A10 static test gaps. Four-instance 96k/64 deadline failures in Linux/KVM; no Windows/ASIO equivalence claimed. R01/R02/R03 candidate: prevalidate and cache every known queue point before any side effect; delayed per-lane curves and blockwise stage processing instead of whole-chain single-sample spans; FFT bit-reversal, decay and fallback rotation precomputed at prepare; active recall uses existing bypass ramp. Creative math and assets unchanged. Requires baseline PCM comparison, dense/invalid queue tests, active recall ramp, profiling; not yet compiled. R04–R07 and packaging/GUI tests remain open. NO-GO. No CI yet.

P028l second source group: R04 interrupted fade freezes the current opaque photo mixture; R05/R07 attached-view live state synchronization with per-view accepted zoom and post-attach MOTION; R06 executable section checks, exact 56-path allowlist, verified extraction, native loader + validator of extracted ZIP and targeted package negatives. See docs/P028l-FOLLOWUP.md. No CI yet; R01–R07 execution and original independent counterprobes remain open. NO-GO.

P028m CI 1d34b1fe: DSP PASS, actual DLL/official EditorHost interaction PASS; new audio harness failed C2975, custom native host exited 1 without output. P028n test-only repair: local constexpr inside lambda, native COM initialization and explicit diagnostics, independent R01 timing execution after successful harness compilation. See docs/P028l-FOLLOWUP.md. NO-GO.

P028n edf33fec: DSP, recorded audio, actual state/audio regression PASS; hosted R01 1620/9600 misses (FAIL), R06 missing-DLL diagnostic check FAIL, new GUI geometry fails at host desktop clamp above 100%. P028o fixes and DSP arithmetic optimizations unverified. See docs/P028l-FOLLOWUP.md. NO-GO.

P028o 2182a2f8: DSP and recorded audio PASS; 47/47 validator, state/audio tests PASS; native MOTION/zoom/two-view gates passed, R04 sampling gate FAIL. R06 ctypes discarded numeric loader error. R01 1448/9600 deadline misses, still FAIL. P028p: direct Windows loader error capture, persistent GDI capture and timings, SSE2 FFT exact paired regression. Pending execution. NO-GO.

P028p 6063273a: native R04/R05/R07 and R06 package/loader PASS; actual R02/R03 and 47/47 validator PASS; Linux scalar/SSE2 PCM regression FAIL and R01 2444/9600 deadline misses FAIL. P028q rejects SSE2 and distributes scalar previous-frame FFT work, with unchanged PCM tolerance; verify again. NO-GO. See docs/P028l-FOLLOWUP.md.

P028q a3a30167: scalar/distributed PCM, DSP and recorded audio PASS Windows/Linux, Windows 47/47 validator/R02/R03/R06 PASS; R01 1306/9600 misses FAIL; R04 insufficient retarget sampling, other native checks passed. P028r distributes predictable inverse FFT and improves sampling without weakening gates. Pending CI, NO-GO.

P028r 76c6a918: DSP/recorded/native GUI PASS; actual validator/state/audio/package PASS; R01 4247/9600 misses FAIL. P028s removes speculative inverse prediction, batches forward FFT, hoists exact duplicate DSP math and precomputes grain window, adds thread-cycle diagnostics. Unverified. NO-GO.

P028s e8fa904f: DSP/recorded/native GUI/validator/state/audio/package PASS, R01 1196/9600 misses FAIL. P028t packed Hermitian inverse FFT N/2, paired full-inverse PCM and same-host isolated hop timing, actual callback phase/cycle diagnostics. Local standalone transform check only; full CI pending. NO-GO.

P028t 8d094733: all function/PCM/GUI/package checks PASS; R01 1169/9600 misses, 1011 in callback phase 7. P028u spreads packed inverse hold prediction including decay/packing, no change to output latency, retains capture fallback and paired regressions. CI pending. NO-GO.

P028u 6238afef: DSP/recorded/GUI/validator/state/audio/package PASS, R01 871/9600 misses FAIL (phase 7: 254). P028v exact cached spectral constants and synthesis-only SSE2, conservative magnitude threshold; matched scalar spread PCM/timing oracle, verbose evidence. Pending CI. NO-GO.

P028v 8d3aa654: DSP/PCM/recorded/GUI/validator/state/audio/package PASS; R01 183/9600 misses FAIL (1.91%). Owner-directed separate practical matrix at 44.1/48 kHz, 64/128/256 frames, 1/4 instances, representative/high-dense. Test-only change, DSP unchanged. Existing R01 stays FAIL until independently proven otherwise; practice matrix pending. No FULL AUDIT or Studio Pro/ASIO result yet. See docs/P028l-FOLLOWUP.md.

c2051514 practical matrix: 23/24 PASS, 1/32308 missed (44.1k/64, four representative steady). 48k all PASS, both rates 128/256 all PASS. Strict hosted matrix FAIL; R01 3459/9600 FAIL, same DSP as prior 183/9600 run, different CI host. All other workflows and actual functional/package tests PASS. Freeze candidate DSP; independent review and actual DAW/ASIO evidence outstanding, no FULL AUDIT/technical GO. Docs-only record; source candidate c2051514. See docs/P028l-FOLLOWUP.md.

P028w follows independent closing audit N01/N02/N03 (full report supplied): candidate preserves active THERAPY smoothing, retains actual VSTGUI CFrame during synchronous resize and clears detached frame pointer, refreshes zoom before attach. Added analytic PCM and native close/reopen/preattach probes. No creative DSP changes; CI and independent re-audit pending. NO-GO.

P028x: independent P028w audit N01 PASS (24/24), original live detach fixed; new initial attach detach/refusal and same-view reattach FAIL. Candidate separates VSTGUI construction from host resize, resets size caches, transactional attach failure; native accept/refuse/detach/retry same-view probes. DSP unchanged. CI pending; NO-GO. See docs/P028l-FOLLOWUP.md.

P028y: P028x official EditorHost revealed initial parent size mismatch (1536x1024 instead of 768x512); next 75% actually reached 1152x768. Correct host getSize/resizeView handshake and native equality-shortcut test; native follow-up no longer skipped on preceding test failure. DSP unchanged; P028x timing strict FAIL (4/32308 practical, 3026/9600 extreme). Pending verification; NO-GO.

P028z: correct test-only ViewRect comparison C2678 and gate native follow-up on successful build. P028y product compiled/validator 47/47; GUI runtime NOT RUN. P028y timing strict FAIL 5/32308 practical, 2365/9600 extreme; no DSP changes. Pending native verification; NO-GO.

P028z tested candidate 6fa62bbe: native GUI including official EditorHost and N02/N03 all PASS, DSP and functional Windows/package PASS, validator 47/47. Strict practical timing 8/32308 and extreme 64/9600 FAIL. Freeze for independent follow-up of GUI lifecycle; no speculative DSP change. NO-GO pending independent decision. See docs/P028l-FOLLOWUP.md.

P029: independent P028z review confirms previous GUI fixes; new nested attach rollback and initial resize recall defects reproduced on Linux. Generation-bound completion and bounded preference reconciliation implemented with native regression probes. Audio/DSP unchanged. Await actual CI and independent replay; NO-GO.

P029a: P029 native GUI all PASS (including Codex nested reattach and in-attach recall), functional DSP/Windows/package PASS; strict timing 22/24, 2/32308 practical and 2971/9600 extreme FAIL. Broaden audit to live-resize nested recall previously discarded by applyingUi_; coalesce latest request after current resize, bounded passes; six-case native matrix. Pending verification; NO-GO.

P029a tested product e610b01c: all native GUI including P029/P029a, functional Windows, DSP/package and validator 47/47 PASS. Strict timing 22/24, 40/32308 misses at 64 samples, extreme 146/9600 FAIL; all 128/256 cases PASS. Frozen for independent lifecycle follow-up; NO-GO pending review, no speculative DSP changes. See docs/P028l-FOLLOWUP.md.

P029b: independent P029a audit confirms prior fixes, new DPI/recall and nested registration failures, X11 last-view Cairo/XCB sporadic abort. Candidate serializes geometry and reconciles newly registered attachments; native tests added. X11 root cause unresolved; Win32 repeat test pending. DSP unchanged, NO-GO until verification.

P029b tested product 4115d07e: all native Windows GUI including 8 new scenarios (24 last-view reopen cycles), functional Windows/DSP/package/validator PASS. Timing 16/24, 42/32308 practical and 3314/9600 extreme FAIL; 128/256 also affected in this run. X11 root cause unresolved; Win32 last-view repeat PASS. Freeze for independent review; NO-GO. No DSP changes.

P029c: P029b independent audit confirms previous fixes; new pre-frame DPI/getSize mismatch blocks fixed-parent host attach. Candidate publishes canonical pre-frame geometry, negotiates DPI when host assigned, and adds 30-case DPI/zoom fixed-parent + reuse matrix and frame-before-attach accept/refuse. Native CI pending. X11 issue remains open; DSP unchanged. NO-GO.

P029c tested product 66809e97: native GUI all PASS including 30 pre-frame DPI/zoom cases + same-view reattach, frame-before-attach accept/refuse, 24 last-view cycles. DSP/state/audio/package/validator 47/47 PASS. Strict timing 21/24, 5/32308 practical at 64 samples, 3154/9600 extreme FAIL; all 128/256 PASS. Frozen pending independent review; NO-GO. X11 issue open, DSP unchanged.

P029d: independent P029c review confirms prior fixes; detached recall stale preferred size and nested preattach DPI return mismatch remain. Candidate separates requested detached size from last host rectangle and verifies effective nested attachment DPI; 12+3 native cases added. CI pending, X11 issue open, DSP unchanged, NO-GO.

P029e: P029d product compiled and functional DSP/Windows/package/validator PASS, GUI harness C3487 prevented native execution. Explicit callback tresult types, test/docs only. Practical timing 22/24, 4/32308, extreme 3425/9600 FAIL. Native tests pending, NO-GO.

P029e tested 03fde438 (P029d product unchanged): all native GUI incl. 12+3 new cases PASS, functional DSP/Windows/package/validator 47/47 PASS. Practical timing 24/24 PASS, 0/32308 misses; extreme 55/9600 FAIL. Freeze pending independent review; no ASIO claim. X11 issue remains open. NO-GO pending independent decision.

P029f: independent P029e audit found fractional DPI attach rejection, stale early onSize corrupting new attachment, fixed-parent redundant resize refusal. Candidate keeps fractional logical size separate from canonical host pixels, rejects stale onSize and reconciles parent, validates matching Win32 parent before accepting harmless refusal. Expanded native matrix. DSP unchanged; CI pending; NO-GO.
