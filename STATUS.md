# 125A PSYCHIATRY — Verified Development Status
Updated: **2026-11-23** · **P002 validated Windows VST3 prototype (NOT the final product)**

## Last verified tested source
- Branch: **main** only.
- Source commit: **`7648a57928fbefb31e052d0f581429effa82d8d7`**.
- [Windows P002 GitHub Actions run 45189637433](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45189637433): **SUCCESS**. Build produced `PsychiatryPrototype.vst3`.
- **Steinberg VST3 validator: 47/47 PASS, 0 FAIL** (observed in same run). It tested factory/lifecycle, bus arrangements, input processing, automation and other API contract checks. The previous rejection of two extreme samplerates was corrected.
- A 7-day **prototype bundle** is attached to that Actions run as artifact `125A-PSYCHIATRY-P002-Windows-VST3-UNVALIDATED`. The word UNVALIDATED refers to **real DAW sound/usage not yet accepted**; the Steinberg **API validator has passed**.
- P001 standalone DSP test workflow also passed on same commit: [P001 workflow](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45189637160).

## Functionality built
- Genuine **Windows x64 VST3 audio effect**, built against pinned original **Steinberg VST3 SDK 3.8.1**, Git commit `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96`.
- **PARANOIA** is connected to the P001 DSP (time/transient-triggered previous-audio reversing, stereo divergent nonlinear feedback, safe crossfade envelopes).
- Automatable `PARANOIA`, reverse global Dry/Wet `THERAPY`, native `Bypass` exposed in host generic editor; other eight planned macros **not exposed and NOT implemented**.
- Supports 1/2 channels (mono/stereo) and 32-bit float processing; VST3 state includes versioned PARANOIA, THERAPY and Bypass; reads parameter events at sample offsets.
- A temporary Windows VST3 artifact exists; **no public release or complete product is claimed**.

## Outstanding requirements / NO-GO for final release
- **No custom GUI or historical photo monitor yet.**
- **No eight other macro DSP engines yet**, no LOSS OF CONTROL or DE/EN GUI.
- **No confirmed Studio Pro session test, audible quality acceptance, project save/reopen in a real DAW, or production CPU budget test**. Steinberg's 47/47 is **not** proof that the effect sounds good.
- Only a synthetic WAV demonstration and offline basic DSP checks exist. Audio input-specific artistically useful signal transformations still need listening tests on bass, drums, voice, field recordings and multi-effect combinations.
- No historical photographs/licensed asset files have been selected or bundled. Rights/privacy/provenance and reduce-motion settings remain requirements.
- Very unusual sample rates `1234.568` and `1234567.8` Hz are accepted for API compatibility and buffer allocation, but musical DSP quality at rates outside normal production ranges has **not** been evaluated.
- No 64-bit processing path; VST3 legitimately reports 32-bit support only.
- The GitHub artifact is temporary and not an installer; no finished user package.

## Next milestones
- **P003**: improve PARANOIA listening quality, and implement remaining **8 peer macro effects** in independent audio reference comparisons, then wire to VST3 with stable parameter IDs.
- **P004**: build the interactive vintage diagnostic GUI and rights-cleared historic imagery, audiovisual modulation plus accessible reduced-motion option.
- **P005**: production stress/latency/automation/recall, full Windows VST3 validator and real Studio Pro musical acceptance at the end, then installer/release.

## Repository policy
**Only main.** Document each verified build with explicit commit and workflow URL. Do not equate automated validator passes with complete, usable sound. No personal developer account details in product GUI/installer or released metadata.
