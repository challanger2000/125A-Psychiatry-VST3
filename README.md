# 125A PSYCHIATRY

**Experimental Dark Multi-FX for Gothic, Darkwave, EBM, Industrial and Cinematic Horror Sound Design.**

> **Status: concept specification (2026-11-23). No functioning VST3 or audio build yet.**

## Vision
Nine **equal-priority** effect worlds, one knob per world. Each knob morphs a carefully tuned combination of sound processors, not a single standard FX module. The goal is disturbing, powerful **and useful** sound design, from rhythmic mechanical destruction to haunted spaces and fractured source audio.

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

**THERAPY / THERAPIE:** inverted global Dry/Wet (0% = wet; 100% = dry).

**LOSS OF CONTROL / KONTROLLVERLUST:** randomize nine macros while keeping THERAPY unchanged.

## Visual concept
An interactive old diagnostic monitor with changeable, rights-cleared historical psychiatric-institution imagery; each macro modifies the imagery in its own way. Horror aesthetic with respect for real historic people. No third-party media bundled at this stage.

## Project files
- [Full concept, tentative DSP recipes and acceptance criteria](docs/SPEC.md)
- [Current implementation status and milestones](STATUS.md)

## Engineering
Target: **Windows x64 VST3**, C++ / Steinberg SDK, Visual Studio 2022. Use only the `main` branch. GitHub Actions will be added when an actual build/test target exists. Automated checks must not be confused with a successful listening test.

© 125A. License to be decided before any code or asset redistribution.
