cmake_minimum_required(VERSION 3.25)
project(PsychiatryAudioPrototype VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# A standalone DSP executable remains buildable without Steinberg's SDK.
add_library(psychiatry_audio STATIC
    src/paranoia.cpp
    src/depression.cpp
    src/obsession.cpp)
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
  set(SMTG_ENABLE_VSTGUI_SUPPORT OFF CACHE BOOL "" FORCE)
  set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SMTG_CREATE_PLUGIN_LINK OFF CACHE BOOL "" FORCE)
  add_subdirectory("${PSYCHIATRY_VST3_SDK_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/steinberg-sdk")
  smtg_enable_vst3_sdk()
  smtg_add_vst3plugin(PsychiatryPrototype
      vst3/ids.h
      vst3/processor.h vst3/processor.cpp
      vst3/controller.h vst3/controller.cpp
      vst3/entry.cpp
  )
  target_compile_features(PsychiatryPrototype PRIVATE cxx_std_17)
  target_include_directories(PsychiatryPrototype PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_link_libraries(PsychiatryPrototype PRIVATE psychiatry_audio sdk)
  smtg_target_configure_version_file(PsychiatryPrototype)
endif()
