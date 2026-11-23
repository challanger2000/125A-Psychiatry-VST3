# 125A PSYCHIATRY

**Experimental Dark Multi-FX for Gothic, Darkwave, EBM, Industrial and Cinematic Horror Sound Design.**

> **Current status (2026-11-23): P003 Windows x64 VST3 prototype. Steinberg validator 47/47 PASS for the three-macro build on tested commit `717941a0`. DEPRESSION, PARANOIA and OBSESSION now have real audio DSP, global THERAPY and Bypass. Six sound worlds and the custom GUI remain to be built.** See [STATUS.md](STATUS.md) for the tested commit, logs, limitations and milestones.

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

**THERAPY / THERAPIE** is inverted global Dry/Wet (0% = wet; 100% = dry). **LOSS OF CONTROL / KONTROLLVERLUST** randomizes the nine macro values without changing therapy. **Three macros (DEPRESSION, PARANOIA, OBSESSION) and global THERAPY are implemented and wired through the **P003 VST3**, along with native Bypass. The other six remain reserved and not exposed; the nine-macro design is not yet complete.

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

The P001 CI checks verify only the standalone audio prototype. **P003 now builds and passes the Steinberg VST3 validator 47/47**, but listening tests on real audio sources and host recall tests in an actual DAW are still separate release gates.

## P003 Windows VST3 prototype

- [Real Windows build, Steinberg 47/47 validator and prototype bundle](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45191507273)
- [VST3 engineering notes](docs/P002.md)
- Audio is available through the **host's generic parameter UI**; there is **no custom GUI yet**.
- Supported host I/O: mono/stereo, 32-bit float; DEPRESSION, PARANOIA, OBSESSION, THERAPY and Bypass are automated and saved in project state; the P002 state layout remains readable.
- The Actions artifact is a **temporary development build, not a final tested release**. [P003 compound sound examples](docs/P003.md) are synthetic, not a substitute for musician listening tests.

## Historical visual concept
An old diagnostic display with dynamic monochrome historical asylum interiors and equipment. Each macro drives corresponding image transformations; source and license records for all assets are required before bundling. No third-party pictures ship now.

## Project references
- [Full nine-macro specification](docs/SPEC.md)
- [Current verified status and next steps](STATUS.md)

## Engineering rules
Target: **Windows x64 VST3**, C++ / Steinberg SDK, Visual Studio 2022. Use only `main`. Keep the repository current and do not confuse CI/validator green checks with finished, audible DSP.

© 125A. Licensing to be confirmed before redistribution.
