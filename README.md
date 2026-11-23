# 125A PSYCHIATRY

**Experimental Dark Multi-FX for Gothic, Darkwave, EBM, Industrial and Cinematic Horror Sound Design.**

> **Current status (2026-11-23): P001 C++ audio-only PARANOIA prototype. Windows and Linux CI passed. NOT YET A VST3.** See [STATUS.md](STATUS.md) for the exact engineering state, CI evidence, limitations and next milestones.

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

**THERAPY / THERAPIE** is inverted global Dry/Wet (0% = wet; 100% = dry). **LOSS OF CONTROL / KONTROLLVERLUST** randomizes the nine macro values without changing therapy. Only PARANOIA and THERAPY are currently implemented *inside the non-VST3 P001 audio engine*. Other controls remain planned.

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

The successful P001 CI checks **do not** establish usable sound quality or a working Windows VST3 plugin. Listening tests on real audio sources and the actual DAW host are separate release gates.

## Historical visual concept
An old diagnostic display with dynamic monochrome historical asylum interiors and equipment. Each macro drives corresponding image transformations; source and license records for all assets are required before bundling. No third-party pictures ship now.

## Project references
- [Full nine-macro specification](docs/SPEC.md)
- [Current verified status and next steps](STATUS.md)

## Engineering rules
Target: **Windows x64 VST3**, C++ / Steinberg SDK, Visual Studio 2022. Use only `main`. Keep the repository current and do not confuse CI/validator green checks with finished, audible DSP.

© 125A. Licensing to be confirmed before redistribution.
