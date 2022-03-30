# CMakeList.txt : CMake project for HDRTray, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("HDRTray")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_REVISION 0)
set(VERSION_BUILD 0)

configure_file(version.rc.in "${CMAKE_CURRENT_BINARY_DIR}/generated/version.rc")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE}")

    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /debug /opt:ref /opt:icf")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
endif()

add_library(Windows10Colors STATIC)
target_sources(Windows10Colors PRIVATE Windows10Colors/Windows10Colors/Windows10Colors.cpp Windows10Colors/Windows10Colors/Windows10Colors.h)
target_include_directories(Windows10Colors PUBLIC Windows10Colors/Windows10Colors/)

# Add source to this project's executable.
add_executable(HDRTray)
target_sources(HDRTray PRIVATE
               "HDR.h"
               "HDRTray.h"
               "NotifyIcon.hpp"
               "HDR.cpp"
               "HDRTray.cpp"
               "HDRTray.manifest"
               "HDRTray.rc"
               "NotifyIcon.cpp"
               )
target_compile_definitions(HDRTray PRIVATE UNICODE _UNICODE)
target_include_directories(HDRTray PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(HDRTray PRIVATE Windows10Colors comctl32)
set_target_properties(HDRTray PROPERTIES WIN32_EXECUTABLE ON)

# TODO: Add tests and install targets if needed.
