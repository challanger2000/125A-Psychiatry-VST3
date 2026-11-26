cmake_minimum_required(VERSION 3.25)
project(PsychiatryLocalPerf LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SMTG_ENABLE_VSTGUI_SUPPORT OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "" FORCE)
file(TO_CMAKE_PATH "${SDK_DIR}" SDK_DIR)
file(TO_CMAKE_PATH "${ASIO_SDK_DIR}" ASIO_SDK_DIR)
add_subdirectory("${SDK_DIR}" sdk-build)
add_executable(performance performance.cpp "${SDK_DIR}/public.sdk/source/vst/hosting/parameterchanges.cpp")
target_include_directories(performance PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../..")
target_link_libraries(performance PRIVATE sdk winmm ole32)

# Separate GPL-3.0 test executable, never linked into the product DLL.
add_executable(driver_test asio_test.cpp
    "${SDK_DIR}/public.sdk/source/vst/hosting/parameterchanges.cpp"
    "${ASIO_SDK_DIR}/host/pc/asiolist.cpp")
target_include_directories(driver_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../.."
    "${ASIO_SDK_DIR}/common" "${ASIO_SDK_DIR}/host" "${ASIO_SDK_DIR}/host/pc")
target_compile_definitions(driver_test PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(driver_test PRIVATE sdk ole32 advapi32 winmm)
