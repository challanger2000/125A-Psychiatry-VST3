cmake_minimum_required(VERSION 3.20)
project(PsychiatryAudioPrototype VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(psychiatry_audio STATIC src/paranoia.cpp)
target_include_directories(psychiatry_audio PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
if(MSVC)
  target_compile_options(psychiatry_audio PRIVATE /W4 /permissive-)
else()
  target_compile_options(psychiatry_audio PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(test_paranoia tests/test_paranoia.cpp)
target_link_libraries(test_paranoia PRIVATE psychiatry_audio)
add_test(NAME P001_ParanoiaDSP COMMAND test_paranoia)

add_executable(render_paranoia tools/render_paranoia.cpp)
target_link_libraries(render_paranoia PRIVATE psychiatry_audio)
