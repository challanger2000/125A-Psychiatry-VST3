cmake_minimum_required(VERSION 3.25)
project(PsychiatryAudioPrototype VERSION 0.9.28 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# A standalone DSP executable remains buildable without Steinberg's SDK.
# Decode only archived, hash-verified sources at build time. The generated
# immutable PCM is linked into the same VST3; no decoder at runtime.
# CI explicitly supplies the exact interpreter whose decoder was installed.
# For local builds prefer the active environment/PATH rather than a Windows
# registry entry for an unrelated Python installation.
set(Python3_FIND_STRATEGY LOCATION)
set(Python3_FIND_REGISTRY LAST)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND "${Python3_EXECUTABLE}" -c
    "import importlib.util, shutil, sys; sys.exit(0 if importlib.util.find_spec('imageio_ffmpeg') or shutil.which('ffmpeg') else 1)"
    RESULT_VARIABLE ACOUSTIC_DECODER_CHECK)
if(NOT ACOUSTIC_DECODER_CHECK EQUAL 0)
    message(FATAL_ERROR "Acoustic bank decoder missing for ${Python3_EXECUTABLE}. Install imageio-ffmpeg==0.6.0 with that interpreter, or install ffmpeg on PATH.")
endif()
set(ACOUSTIC_BANK "${CMAKE_CURRENT_BINARY_DIR}/generated/acoustic_bank.hpp")
file(GLOB_RECURSE ACOUSTIC_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/sound-library/originals/*.ogg")
add_custom_command(OUTPUT "${ACOUSTIC_BANK}"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_acoustic_bank.py" "${ACOUSTIC_BANK}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_acoustic_bank.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/sound-library/sources.json" ${ACOUSTIC_SOURCE_FILES}
    VERBATIM)
add_library(psychiatry_audio STATIC
    "${ACOUSTIC_BANK}"
    src/reactive_acoustic.cpp
    src/paranoia.cpp
    src/depression.cpp
    src/obsession.cpp
    src/three_worlds.cpp
    src/spectral_worlds.cpp)
target_include_directories(psychiatry_audio PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
set_target_properties(psychiatry_audio PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(psychiatry_audio PRIVATE /W4 /permissive-)
else()
  target_compile_options(psychiatry_audio PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(test_scene_sequence tests/test_scene_sequence.cpp)
add_test(NAME P028h_SceneSequence COMMAND test_scene_sequence)
add_executable(test_scene_blend tests/test_scene_blend.cpp)
add_test(NAME P028k_SceneBlend COMMAND test_scene_blend)
add_executable(test_linear_phase_2x tests/test_linear_phase_2x.cpp)
target_include_directories(test_linear_phase_2x PRIVATE src)
add_test(NAME P018_LinearPhaseFIR COMMAND test_linear_phase_2x)
add_executable(test_paranoia tests/test_paranoia.cpp)
target_link_libraries(test_paranoia PRIVATE psychiatry_audio)
add_test(NAME P001_ParanoiaDSP COMMAND test_paranoia)

add_executable(test_dark_macros tests/test_dark_macros.cpp)
target_link_libraries(test_dark_macros PRIVATE psychiatry_audio)
add_test(NAME P003_DarkMacros COMMAND test_dark_macros)

add_executable(render_dark_macros tools/render_dark_macros.cpp)
target_link_libraries(render_dark_macros PRIVATE psychiatry_audio)

add_executable(test_three_worlds tests/test_three_worlds.cpp)
target_link_libraries(test_three_worlds PRIVATE psychiatry_audio)
add_test(NAME P004_ThreeWorlds COMMAND test_three_worlds)

add_executable(render_three_worlds tools/render_three_worlds.cpp)
target_link_libraries(render_three_worlds PRIVATE psychiatry_audio)

add_executable(test_spectral_worlds tests/test_spectral_worlds.cpp)
target_link_libraries(test_spectral_worlds PRIVATE psychiatry_audio)
add_test(NAME P005_SpectralWorlds COMMAND test_spectral_worlds)

add_executable(render_spectral_worlds tools/render_spectral_worlds.cpp)
target_link_libraries(render_spectral_worlds PRIVATE psychiatry_audio)

add_executable(test_creative_worlds tests/test_creative_worlds.cpp)
target_link_libraries(test_creative_worlds PRIVATE psychiatry_audio)
add_test(NAME P026k_CreativeWorlds COMMAND test_creative_worlds)

add_executable(test_full_chain tests/test_full_chain.cpp)
target_link_libraries(test_full_chain PRIVATE psychiatry_audio)
add_test(NAME P005_AllNineConcurrent_NoHeap COMMAND test_full_chain)

# P006 objective metrics for synthetic bass/drums/voice-like/atmosphere source proxies.
add_executable(audit_p006 tools/audit_p006.cpp)
target_link_libraries(audit_p006 PRIVATE psychiatry_audio)

add_executable(test_hilbert_coefficients tests/test_hilbert_coefficients.cpp)
add_test(NAME P006_HilbertCoefficientRegression COMMAND test_hilbert_coefficients)

add_executable(render_paranoia tools/render_paranoia.cpp)
target_link_libraries(render_paranoia PRIVATE psychiatry_audio)

# P002: actual audio-processing VST3, only implemented macros + THERAPY + BYPASS.
option(PSYCHIATRY_BUILD_VST3 "Build the Steinberg VST3 component" OFF)
if(PSYCHIATRY_BUILD_VST3)
  set(PSYCHIATRY_VST3_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/vst3sdk" CACHE PATH
      "Steinberg VST3 SDK checkout with initialized submodules")
  if(NOT EXISTS "${PSYCHIATRY_VST3_SDK_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Steinberg SDK not found at ${PSYCHIATRY_VST3_SDK_DIR}; see docs/P002.md")
  endif()
  set(SMTG_ENABLE_VSTGUI_SUPPORT ON CACHE BOOL "" FORCE)
  set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SMTG_CREATE_PLUGIN_LINK OFF CACHE BOOL "" FORCE)
  add_subdirectory("${PSYCHIATRY_VST3_SDK_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/steinberg-sdk")
  smtg_enable_vst3_sdk()
  # Keep the CMake target stable, but name the actual VST3 package and
  # Windows binary after the product, using the pinned SDK package API.
  smtg_add_vst3plugin(PsychiatryPrototype
      PACKAGE_NAME "125A-PSYCHIATRY"
      SOURCES_LIST
      vst3/ids.h vst3/monitor_model.h vst3/scene_sequence.h
      vst3/processor.h vst3/processor.cpp
      vst3/controller.h vst3/controller.cpp
      vst3/entry.cpp
      resource/psychiatry.uidesc
  )
  target_compile_features(PsychiatryPrototype PRIVATE cxx_std_17)
  # One version authority for both factory classes and the module manifest.
  target_compile_definitions(PsychiatryPrototype PRIVATE PSYCHIATRY_PRODUCT_VERSION="${PROJECT_VERSION}")
  target_include_directories(PsychiatryPrototype PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_link_libraries(PsychiatryPrototype PRIVATE psychiatry_audio sdk vstgui_support)
  # The actual .uidesc MUST be staged inside the built Windows VST3 bundle.
  # It is not enough to include the file in the CMake source list alone.
  # Preserve the exact notices from the pinned SDK and VSTGUI checkouts
  # in every distributed binary bundle (MIT and BSD-style respectively).
  configure_file("${PSYCHIATRY_VST3_SDK_DIR}/LICENSE.txt"
      "${CMAKE_CURRENT_BINARY_DIR}/LICENSE-VST3-SDK.txt" COPYONLY)
  configure_file("${PSYCHIATRY_VST3_SDK_DIR}/vstgui4/LICENSE"
      "${CMAKE_CURRENT_BINARY_DIR}/LICENSE-VSTGUI.txt" COPYONLY)
  execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_artwork.py"
      "${CMAKE_CURRENT_SOURCE_DIR}" RESULT_VARIABLE ARTWORK_CHECK)
  if(NOT ARTWORK_CHECK EQUAL 0)
    message(FATAL_ERROR "P028c artwork verification failed")
  endif()
  smtg_target_add_plugin_resources(PsychiatryPrototype
      RESOURCES resource/psychiatry.uidesc resource/LICENSE-ACOUSTIC-SOURCES.txt
      resource/artwork/faceplate.png resource/artwork/macro-atlas.png
      resource/artwork/therapy-atlas.png resource/artwork/porthole-glass.png
      resource/artwork/loss-released.png resource/artwork/loss-pressed.png
      resource/artwork/scene-corridor-bw.png
      resource/artwork/scene-p028h-01-depression-isolation.png
      resource/artwork/scene-p028h-02-panic-medical-room.png
      resource/artwork/scene-p028h-03-dissociation-reflection.png
      resource/artwork/scene-p028h-04-mania-scribbled-walls.png
      resource/artwork/scene-p028h-05-corridor-wheelchair.png
      resource/artwork/scene-p028h-06-padded-room-bed.png
      resource/artwork/scene-p028h-07-treatment-room-straitjacket.png
      resource/artwork/scene-p028h-08-eeg-mannequin.png
      resource/artwork/scene-p028h-09-empty-ward-bed.png
      resource/artwork/scene-p028h-10-mania-motion-blur.png
      resource/artwork/scene-p028h-11-paranoia-observer.png
      resource/artwork/scene-p028h-12-isolation-padded-room.png
      resource/artwork/scene-p028h-13-fragmented-reflections.png
      resource/artwork/scene-p028h-14-hallucination-double-exposure.png
      resource/artwork/scene-p028h-15-corridor-straitjacket.png
      resource/artwork/scene-p028h-16-hydrotherapy-tub.png
      resource/artwork/scene-p028h-17-treatment-room-shadow.png
      resource/artwork/scene-p028h-18-electrotherapy-console.png
      resource/artwork/scene-p028h-19-historical-treatment-nurse.png
      resource/artwork/scene-p028h-20-historical-restraint-chair.png
      resource/artwork/scene-p028h-21-empty-electrotherapy-chair.png
      resource/artwork/scene-p028h-22-historical-electrode-portrait.png
      resource/artwork/scene-p028h-23-historical-treatment-seated.png
      resource/artwork/scene-p028h-24-historical-doctor-and-nurse.png
      resource/artwork/scene-p028h-25-ward-hanging-straitjacket.png
      resource/artwork/scene-p028h-26-patient-portrait-young.png
      resource/artwork/scene-p028h-27-matron-with-keys.png
      resource/artwork/scene-p028h-28-open-medicine-cabinet.png
      resource/artwork/scene-p028h-29-patient-records-archive.png
      resource/artwork/scene-p028h-30-antique-instruments.png
      resource/artwork/scene-p028h-31-abandoned-common-room.png
      resource/artwork/scene-p028h-32-institution-staircase.png
      resource/artwork/scene-p028h-33-basement-corridor.png
      resource/artwork/scene-p028h-34-long-ward.png
      resource/artwork/scene-p028h-35-double-exposure-portrait.png
      resource/artwork/scene-p028h-36-elderly-patient-portrait.png
      resource/artwork/scene-p028h-37-nurse-portrait.png
      resource/artwork/scene-p028h-38-neurological-instruments.png
      resource/artwork/scene-p028h-39-closed-medicine-cabinet.png
      resource/artwork/scene-p028h-40-disordered-common-room.png
      resource/artwork/scenes-p028h.json
      "${CMAKE_CURRENT_BINARY_DIR}/LICENSE-VST3-SDK.txt"
      "${CMAKE_CURRENT_BINARY_DIR}/LICENSE-VSTGUI.txt")
  smtg_target_configure_version_file(PsychiatryPrototype)

  # Exercise processor and controller state decoding against the genuine
  # Steinberg SDK interfaces (legacy v1-v4, malformed/NaN/future states).
  add_executable(test_vst3_recall
      tests/test_vst3_recall.cpp
      vst3/processor.cpp
      vst3/controller.cpp
      "${PSYCHIATRY_VST3_SDK_DIR}/public.sdk/source/vst/hosting/parameterchanges.cpp")
  target_include_directories(test_vst3_recall PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_link_libraries(test_vst3_recall PRIVATE psychiatry_audio sdk vstgui_support)
  add_test(NAME P006_VST3LegacyState COMMAND test_vst3_recall)

  # P010: process real stereo input through the actual Processor::process()
  # call with the same serialized VST3 macro state/THERAPY used by DAWs.
  # Reject zero-effect placebo, pure-gain false positives and bad endpoints.
  add_executable(test_vst3_audio_path
      tests/test_vst3_audio_path.cpp
      vst3/processor.cpp
      # Steinberg's host-side ParameterChanges helper is not part of sdk.lib.
      "${PSYCHIATRY_VST3_SDK_DIR}/public.sdk/source/vst/hosting/parameterchanges.cpp")
  target_include_directories(test_vst3_audio_path PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_link_libraries(test_vst3_audio_path PRIVATE psychiatry_audio sdk)
  add_test(NAME P010_ActualVST3AudioWetDry COMMAND test_vst3_audio_path)
  if(WIN32)
    add_executable(test_native_followup tests/test_native_followup.cpp)
    target_include_directories(test_native_followup PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(test_native_followup PRIVATE sdk user32 gdi32)
  endif()
endif()
