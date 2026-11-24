# Third-party components

- Steinberg VST3 SDK 3.8.1, pinned 3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96: MIT, exact upstream LICENSE.txt included in the plugin as LICENSE-VST3-SDK.txt.
- Steinberg VSTGUI, pinned submodule 5db272256172557818b6158cf0bb2c4410bddb25: upstream BSD-style license, exact LICENSE included as LICENSE-VSTGUI.txt.
- The nine DSP engines and control-driven CRT graphics are original project code and illustrations. No external photos or voice samples are bundled as of P022.

The upstream licenses do not constitute endorsement by Steinberg. Packaging checks fail if either license notice is absent. No new third-party media may be added without a source, rights statement and provenance record.

P024: nine original generated grayscale PNGs (`resource/scene-1.png` through `scene-9.png`) are produced by `tools/generate_crt_scenes.mjs` from fictional box geometry, lighting, shadows, textures and deterministic grain. No external imagery, patient data, identifiable subjects or archival photographs. Generated art is historical-style fiction, not historical documentation.

P026c candidate: six CC BY 4.0 Oculus Audio Pack 01 source excerpts from the pinned sound library are embedded as build-generated immutable PCM for PARANOIA. Attribution and modification notices are staged in LICENSE-ACOUSTIC-SOURCES.txt. imageio-ffmpeg 0.6.0 is build-time only; no codec, original OGG, download or runtime dependency shipped. Verify actual bundle and source hashes before release.
