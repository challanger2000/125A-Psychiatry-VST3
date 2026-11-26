# 125A PSYCHIATRY

Creative multi-effect for dark electronic music, Industrial, EBM, Gothic, Darkwave and cinematic horror sound design.

**Current version: 0.9.29 (P031) — Windows x64 VST3, development complete for owner acceptance; freeware V1.0 planned, not yet released.**

[Download Windows v0.9.29 candidate](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45883728460/artifacts/15148832276) · [Verification and known limitations](STATUS.md) · [Change history](CHANGELOG.md)

## Controls

Twelve independent sound worlds can be combined: DEPRESSION, PARANOIA, SCHIZOPHRENIA, MANIA, PANIC, DISSOCIATION, OBSESSION, HALLUCINATION, PSYCHOSIS, DELIRIUM, CATATONIA and AMNESIA. Each macro is neutral at 0% and increases its own composite effect with intensity. The names are artistic metaphors, not medical simulations.

- **THERAPY:** inverted global dry/wet; 0% fully processed, 100% dry.
- **LOSS OF CONTROL:** randomizes the twelve effect macros, leaving global settings unchanged.
- **First-aid box:** resets all 13 rotary controls, including THERAPY, to 0%; preserves BYPASS and MOTION.
- **Double-click:** resets the selected rotary control to 0%.
- **MOTION:** enabled by default for new instances; saved explicit settings are recalled.
- **BYPASS:** VST3-native bypass.

The fixed 960 × 640 editor at 100% host DPI uses fictional historical-horror imagery, including 40 scene textures. Host DPI scaling is supported; manual zoom and drag-resize are not provided.

## Installation and use

Extract the ZIP and copy the complete included .vst3 bundle (not just the internal DLL) into the Windows VST3 folder, normally `C:\Program Files\Common Files\VST3`. Rescan plugins in the DAW if necessary.

Recommended starting point: 44.1 or 48 kHz, 128 or 256 samples. The native hosted functional tests passed; the synthetic deadline tests at 64 samples and extreme serial four-instance load did not all pass. See [STATUS.md](STATUS.md) for precise evidence and acceptance gates.

The download above is a temporary Actions artifact with an older NOT-FINAL label, not a published V1.0 release. The product version is 0.9.29.

## Development and documentation

- [START-HERE.md](START-HERE.md) — current handoff and freeze.
- [STATUS.md](STATUS.md) — authoritative current verification record.
- [CHANGELOG.md](CHANGELOG.md) — chronological development history.
- [docs/](docs/) — design, engineering and QA history; older documents are historical unless explicitly referenced as current.
- [sound-library/](sound-library/) — source recordings and attribution information.
- `src/`: DSP; `vst3/`: host integration and editor; `tests/`: regressions; `tools/`: measurement tools.

C++17, Visual Studio 2022, pinned Steinberg VST3 SDK 3.8.1 and VSTGUI. Only `main`. Stable VST3 class and parameter IDs. Standalone DSP tests:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

See [Windows build setup](docs/P002.md). Historical README is preserved at [the pre-cleanup commit](https://github.com/challanger2000/125A-Psychiatry-VST3/blob/99e89f9bf43ad1f204bde2ea9dbb62896cf10452/README.md).

© 125A. Intended distribution: freeware binary. Final redistribution terms and third-party notices must be confirmed before public release; freeware does not itself grant source-code redistribution rights.
