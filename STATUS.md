# Project Status — 125A PSYCHIATRY
Updated: **2026-11-23** · Current engineering checkpoint: **P001 early audio prototype**

## Verified this turn
- Public repo, **main** only. Product specification is [docs/SPEC.md](docs/SPEC.md).
- Added **original PARANOIA C++ DSP prototype** in [src/paranoia.cpp](src/paranoia.cpp) and [src/paranoia.hpp](src/paranoia.hpp). It combines past-buffer reverse events, nonlinear comb-feedback and stereo offset delays. Time/DAW-tempo-derived timing, transient/hybrid triggering and every/alternating/random event selection exist at audio-engine level.
- Added [tests/test_paranoia.cpp](tests/test_paranoia.cpp) and [CMakeLists.txt](CMakeLists.txt). Tests check exact dry bypass, inverse THERAPY, reverse triggering, tempo division, block invariance, finite output at 192 kHz and audible-signal difference. Tests remain active with NDEBUG/Release.
- Added [tools/render_paranoia.cpp](tools/render_paranoia.cpp) and [GitHub Actions](.github/workflows/audio-prototype.yml). CI renders 5-s stereo synthetic **dry and wet WAVs**, uploaded as temporary build artifacts. These are not a VST3.
- Local Linux C++ build passed CTest **1/1**, using 48-kHz processed signal plus 44.1-kHz tempo and 192-kHz robustness cases. Local 5-second test render generated **109 reverse events** (not a musical approval).
- **Windows and Ubuntu Actions both passed** at [run 45185320199](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45185320199), commit `ce54aa6e1aa105f9153953d3de66f175431fd217`. Later pushes may trigger additional runs; check current branch head when judging status.

## Not yet verified or implemented
- **No Windows VST3 plugin** has been built. No GUI, presets, images, host tempo transport, installer, project recall, actual DAW tests, or release.
- Only **PARANOIA** is prototyped; **the other eight macros are still conceptual**. THERAPY works as an internal C++ setting; LOSS OF CONTROL and DE/EN UI are not implemented.
- No user listening sign-off or comparison against commercial products. Automated numerical tests do **not** establish musical quality.
- No sample image licenses cleared and no historical asset files bundled. No copied Backmask code.
- Production realtime allocation instrumentation, full spectral quality review, anti-click automation sweeps, full multiple-instance stress and CPU budget must still be tested.

## Next engineering gates
- **P001-a: evaluate whether PARANOIA is artistically distinctive** on user-provided bass, drums and voice sources. Change/replace as necessary, not just decorate with a GUI.
- **P002: build minimal Windows x64 Steinberg VST3 wrapper**, with reliable program recall, 1/2 channel I/O, automation and safe lifecycle; keep the nine-macro design without falsely claiming unimplemented effects operate.
- **P003: implement and listen-test the other eight compound macro effects** individually and in combinations.
- **P004: historical diagnostic monitor**, with asset provenance and reduced-motion option, off audio thread.
- **P005: Windows VST3 automated + actual end-of-project Studio Pro listening/recall acceptance**, packaging and release.

## Repository policy
Only `main`. Keep status factual/current, trace actual Windows CI jobs, do not equate a successful test executable with a finished instrument or FX. Do not place personal developer account identity in the released product.
