# P028l follow-up candidate — GUI and PE corrections

Baseline d361df96a2ea7a2d40ef4a7c1d098f3474769555; main only. Independent Nachaudit report recovered from owner's uploaded text. No creative DSP, artwork, IDs or state format change.

- R04: preserve opaque current photo mixture using a bounded offscreen bitmap when an active fade is retargeted; normal fade remains 1.2 s; if composition fails defer scene switch. Frame/glass unchanged. Native pixel regression NOT RUN.
- R05/R07: attached-view registry, live MOTION synchronization, initialization after didOpen, accept only editing gestures for checkbox, shared zoom request to attached views. Individual rejected views keep last applied geometry/menu. Shared saved zoom changes when at least one view accepts; all refuse retains previous preference; no attached views saves requested preference. Native attach, partial refusal, live recall, reentrant host and multi-view regression NOT RUN.
- R06: required exports must target executable raw-backed sections; explicit 56-file allowlist (confirmed from exact previous Windows workflow log), ZIP SHA256/CRC/member hashes, verified extraction, real Windows loader dependency resolution, validator on extracted candidate, targeted non-executable-export/missing-DLL/extra-file/archive-tampering negatives added. Execution NOT RUN.

R01–R03 earlier candidate uncompiled. This change is a source candidate only, no CI triggered. Local network checkout unavailable; no SDK locally. All seven findings remain unverified, NO-GO for Studio Pro.

Native follow-up test source added: actual DLL factory + IEditController, real HWND/IPlugFrame accepted and rejected sizes, multi-view partial refusal, open/close state, MOTION pixel changes and interrupted-fade desktop ROI. No claim of execution yet. Higher zoom tests geometry only, not full physical desktop presentation. v0.9.28 candidate.

R01 actual SDK processor: 4 instances serial, 96k/64, 12 macros .85 steady and all 14 queues dense (64 points each), three 1600-group repetitions per mode, raw wall-clock durations and strict no-missed-deadline result. Hosted VM result is not target ASIO. R02/R03 regressions include late malformed queues, output/state/timeline rollback and 5-ms active recall with/without zero-frame flush. Native CI first consolidated candidate; baseline PCM comparison and independent original counterprobes remain open. No results claimed before runs complete.

## First consolidated CI: 2ff8a81d2411c5a904f158bc8c78752b2be6800d
- DSP Windows/Linux [45645280809](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45645280809) SUCCESS.
- Windows [45645280821](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45645280821) actual plugin build and state regression SUCCESS, overall FAIL compiling new R02 test: MSVC C3493 for uncaptured local constants block/total. No audio/package/RT results.
- Recorded [45645280842](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45645280842) same compile failure. GUI did not trigger; final push changed audio test only, excluded by GUI path filters.
- Fix lambda capture and include tests/** in GUI workflow trigger. Product source unchanged; next consolidated verification pending. NO-GO.

## P028m CI: 1d34b1fe8f9edd3691f2a8b1e5d6ba4c6af6ec17
- [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45647157474) SUCCESS; [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45647157635) and [recorded](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45647157600) FAIL on C2975 (captured local constant used as template argument). Move constants inside lambda; no product change.
- [GUI](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45647157501): compiled actual DLL and native host; official EditorHost all existing interaction gates passed, including actual 50%→75% geometry. Added native host exits 1 without diagnostic; root cause not proven. Match official host STA COM initialization, add unbuffered stage diagnostics and explicit executable/module path and exit code capture. No weaker assertions.
- Run R01 independently of earlier audio/package step outcome once its executable builds, so unrelated failures do not suppress timing evidence. All new targeted QA pending. NO-GO.
