# 125A PSYCHIATRY — current status

**2026-11-26 · P031 / v0.9.29 · Windows x64 VST3 · frozen acceptance candidate.**
Exact tested product commit: [`49a8837ab733f29c5f05f546732aba146da1968b`](https://github.com/challanger2000/125A-Psychiatry-VST3/commit/49a8837ab733f29c5f05f546732aba146da1968b).
Owner requested repository cleanup and end of development loops. Documentation only; no DSP, GUI, build, state or package changes. No new builds requested. V1.0 release approval is not recorded.

## Verification

| Gate | Result |
|---|---|
| Native Windows compilation and bundle verification | PASS |
| Steinberg VST3 validator (both invocations) | 47/47 PASS |
| Native GUI and official EditorHost | PASS |
| First-aid reset, all 13 double-click resets, MOTION default and explicit recall | PASS |
| Actual processor/controller state regression and audio-path CTest | PASS |
| Standalone DSP tests, Windows and Linux | 10/10 PASS each |
| Recorded-source export | PASS |
| Hosted practical deadline matrix, 44.1/48 kHz | FAIL overall: 20/24 cases PASS; 6/32308 deadlines missed, only at 64 samples; all 128/256 cases PASS |
| Hosted four serial instances, 96 kHz/64 samples | FAIL: 4116/9600 deadline misses |
| Actual Studio Pro / ASIO, exact v0.9.29 | NOT VERIFIED |
| Separate 125A Plugin Tester, exact v0.9.29 | NOT VERIFIED |
| Independent final release decision | NOT RECORDED |

Previous owner report for P030d / v0.9.28: Studio Pro at ASIO 64 samples, roughly 20% under LOSS OF CONTROL, no directly audible glitches. This is subjective prior-version evidence, not a measured deadline result for P031. Prior independent GO was restricted to P030d controlled audition at 48 kHz/256 samples and unchanged DPI. X11 residual issues remain outside the Windows-only distribution scope; no Linux/macOS plugin release claimed.

## Exact candidate and evidence

- [Windows candidate v0.9.29](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45883728460/artifacts/15148832276) (legacy artifact label `125A-PSYCHIATRY-P028-RASTER-NOT-FINAL`; temporary Actions artifact).
- Artifact SHA-256: `799fe3ea364e527c8c131c56e9c1a879aecf4c42f5cee4ed8ceca6c16c370075`.
- [Windows workflow](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45883728460) — overall FAIL because of deadline tests, despite successful functional/package steps.
- [Native GUI](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45883728516) · [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45883728389) · [Recorded audio](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45883728399).
- [Raw hosted deadline evidence](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45883728460/artifacts/15148603382).

## Handoff

Freeze the exact P031 candidate. No speculative DSP changes or repeated CI timing runs. If release preparation is explicitly authorized, record actual owner DAW acceptance (including reset, MOTION and project recall), outstanding QA and redistribution clearance before marking V1.0 final. Recommend 128/256 samples; do not claim universal 64-sample realtime performance.

## Historical records

All original engineering status entries and links are retained verbatim in [STATUS.md at the pre-cleanup commit](https://github.com/challanger2000/125A-Psychiatry-VST3/blob/99e89f9bf43ad1f204bde2ea9dbb62896cf10452/STATUS.md). The [changelog](CHANGELOG.md) and [docs/](docs/) remain in this repository. Historical checkpoints do not supersede this page.
