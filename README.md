# 125A PSYCHIATRY

**Experimental Dark Multi-FX for Gothic, Darkwave, EBM, Industrial and Cinematic Horror Sound Design.**

> **P005 prototype · Windows x64 VST3 · 2026-11-23. All nine DSP macro worlds are compiled, DAW-automatable, and pass 47/47 Steinberg validator checks. This is still not the finished product: the psychiatric diagnostic GUI, LOSS OF CONTROL, DE/EN switching, listening/CPU acceptance and Studio Pro final verification remain outstanding.** See [STATUS.md](STATUS.md).


## Vision
Nine **equal-priority** multi-algorithm sound worlds, one macro per world. Each knob progressively morphs a *curated compound effect* rather than one ordinary processor. The goal is extreme **and usable** sound design with a reactive historical diagnostic-monitor GUI.

| EN | DE | Character |
|---|---|---|
| DEPRESSION | DEPRESSION | Compressed, suffocating dark decay |
| PARANOIA | PARANOIA | Unpredictable reverse and spatial afterimages |
| SCHIZOPHRENIA | SCHIZOPHRENIE | Contradictory fractured sound layers |
| MANIA | MANIE | Escalating restless modulation |
| PANIC | PANIK | Electric jolts and controlled shock transients |
| DISSOCIATION | DISSOZIATION | Granular and spectral time fragmentation |
| OBSESSION | ZWANG | Repeating, collapsing micro-loops |
| HALLUCINATION | HALLUZINATION | Phantom harmonics and spectral ghosts |
| PSYCHOSIS | PSYCHOSE | Powerful *localized* distortion world |

**All nine** effects have dedicated C++ DSP and are exposed as automatable VST3 parameters (the native host generic editor is used for now). `THERAPY` is an inverted global Dry/Wet (0% wet; 100% dry); native Bypass is also supported.

**LOSS OF CONTROL / KONTROLLVERLUST** randomization is a firm design requirement but **not implemented yet**. The custom horror GUI and the DE/EN switch are likewise still planned.

**This is an early DSP quality prototype, not a claim of nine fully refined, release-quality effects.**

## P001: PARANOIA audio-only prototype
Based on an **original** past-audio reverser design inspired by Freakshow Industries Backmask's event-driven approach, combined with nonlinear feedback and stereo diffusion.

- C++17 core: [src/paranoia.hpp](src/paranoia.hpp), [src/paranoia.cpp](src/paranoia.cpp)
- Dedicated [sound-design notes and test methods](docs/P001.md)
- [CTest audio DSP checks](tests/test_paranoia.cpp) for dry passthrough, trigger logic, exact output, finite processing and buffer-size invariance
- [WAV test probe renderer](tools/render_paranoia.cpp) with 5-second dry/wet examples generated in CI
- [Windows/Ubuntu GitHub Actions](.github/workflows/audio-prototype.yml) to build and run the tests

### Compile and test
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The P001 CI checks verify only the standalone audio prototype. **P005 now builds and passes the Steinberg VST3 validator 47/47**, but listening tests on real audio sources and host recall tests in an actual DAW are still separate release gates.

## P005 Windows nine-macro VST3 prototype

- **Verified Windows Actions:** [P005 complete VST3, Steinberg 47/47 PASS](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45198889548)
- [P005 Windows VST3 build artifact](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45198889548/artifacts/14829987368) — a temporary engineering zip, **not a release**
- [P005 DSP tests, Windows and Linux](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45198889541), 5/5 CTest groups
- [P005 synthetic dry and processed WAV comparisons](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45197686369/artifacts/14828402907)
- An additional **all-nine concurrent DSP regression** checks no callback allocations, exact dry, finite outputs, and block-size invariance.
- [Implementation notes and acoustic limitations](docs/P005.md); [current status](STATUS.md)

**Host support:** Windows x64, 32-bit floating-point audio, mono/stereo, automatable nine macros plus THERAPY and native Bypass; historical saved project states v1–v3 are readable by v4 processor/controller.

**No custom GUI yet:** the host displays all supported knobs through its generic VST3 parameter editor. No historical photo assets are bundled. The VST3 SDK API validator passing **does not** establish Studio Pro audio quality or release readiness.

## Historical visual concept
An old diagnostic display with dynamic monochrome historical asylum interiors and equipment. Each macro drives corresponding image transformations; source and license records for all assets are required before bundling. No third-party pictures ship now.

## Project references
- [Full nine-macro specification](docs/SPEC.md)
- [Current verified status and next steps](STATUS.md)

## Engineering rules
Target: **Windows x64 VST3**, C++ / Steinberg SDK, Visual Studio 2022. Use only `main`. Keep the repository current and do not confuse CI/validator green checks with finished, audible DSP.

© 125A. Licensing to be confirmed before redistribution.
