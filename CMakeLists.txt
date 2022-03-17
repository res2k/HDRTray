# CMakeList.txt : CMake project for HDRTray, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("HDRTray")

# Add source to this project's executable.
add_executable(HDRTray "HDRTray.cpp" "HDRTray.h" "HDRTray.rc" "HDR.cpp" "HDR.h")
target_compile_definitions(HDRTray PRIVATE UNICODE _UNICODE)
set_target_properties(HDRTray PROPERTIES WIN32_EXECUTABLE ON)

# TODO: Add tests and install targets if needed.
