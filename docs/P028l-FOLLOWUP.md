# P028l follow-up candidate — GUI and PE corrections

Baseline d361df96a2ea7a2d40ef4a7c1d098f3474769555; main only. Independent Nachaudit report recovered from owner's uploaded text. No creative DSP, artwork, IDs or state format change.

- R04: preserve opaque current photo mixture using a bounded offscreen bitmap when an active fade is retargeted; normal fade remains 1.2 s; if composition fails defer scene switch. Frame/glass unchanged. Native pixel regression NOT RUN.
- R05/R07: attached-view registry, live MOTION synchronization, initialization after didOpen, accept only editing gestures for checkbox, shared zoom request to attached views. Individual rejected views keep last applied geometry/menu. Shared saved zoom changes when at least one view accepts; all refuse retains previous preference; no attached views saves requested preference. Native attach, partial refusal, live recall, reentrant host and multi-view regression NOT RUN.
- R06: required exports must target executable raw-backed sections; explicit 56-file allowlist (confirmed from exact previous Windows workflow log), ZIP SHA256/CRC/member hashes, verified extraction, real Windows loader dependency resolution, validator on extracted candidate, targeted non-executable-export/missing-DLL/extra-file/archive-tampering negatives added. Execution NOT RUN.

R01–R03 earlier candidate uncompiled. This change is a source candidate only, no CI triggered. Local network checkout unavailable; no SDK locally. All seven findings remain unverified, NO-GO for Studio Pro.

Native follow-up test source added: actual DLL factory + IEditController, real HWND/IPlugFrame accepted and rejected sizes, multi-view partial refusal, open/close state, MOTION pixel changes and interrupted-fade desktop ROI. No claim of execution yet. Higher zoom tests geometry only, not full physical desktop presentation. v0.9.28 candidate.
