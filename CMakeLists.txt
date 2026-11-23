cmake_minimum_required(VERSION 3.25)
project(PsychiatryAudioPrototype VERSION 0.9.5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# A standalone DSP executable remains buildable without Steinberg's SDK.
add_library(psychiatry_audio STATIC
    src/paranoia.cpp
    src/depression.cpp
    src/obsession.cpp
    src/three_worlds.cpp
    src/spectral_worlds.cpp)
target_include_directories(psychiatry_audio PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(psychiatry_audio PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(psychiatry_audio PRIVATE /W4 /permissive-)
else()
  target_compile_options(psychiatry_audio PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
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
  smtg_add_vst3plugin(PsychiatryPrototype
      vst3/ids.h
      vst3/processor.h vst3/processor.cpp
      vst3/controller.h vst3/controller.cpp
      vst3/entry.cpp
      resource/psychiatry.uidesc
  )
  target_compile_features(PsychiatryPrototype PRIVATE cxx_std_17)
  target_include_directories(PsychiatryPrototype PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_link_libraries(PsychiatryPrototype PRIVATE psychiatry_audio sdk vstgui_support)
  # The actual .uidesc MUST be staged inside the built Windows VST3 bundle.
  # It is not enough to include the file in the CMake source list alone.
  smtg_target_add_plugin_resources(PsychiatryPrototype
      RESOURCES resource/psychiatry.uidesc)
  smtg_target_configure_version_file(PsychiatryPrototype)

  # Exercise processor and controller state decoding against the genuine
  # Steinberg SDK interfaces (legacy v1-v4, malformed/NaN/future states).
  add_executable(test_vst3_recall
      tests/test_vst3_recall.cpp
      vst3/processor.cpp
      vst3/controller.cpp)
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
endif()
