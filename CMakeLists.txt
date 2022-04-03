# CMakeList.txt : CMake project for HDRTray, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

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

    # Always use UCRT DLL
    foreach(flag_var
            CMAKE_EXE_LINKER_FLAGS_RELEASE
            CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO)
        set(${flag_var} "${${flag_var}} /nodefaultlib:libucrt.lib ucrt.lib")
    endforeach()
    foreach(flag_var
            CMAKE_EXE_LINKER_FLAGS_DEBUG)
        set(${flag_var} "${${flag_var}} /nodefaultlib:libucrtd.lib ucrtd.lib")
    endforeach()
endif()

add_library(Windows10Colors STATIC)
target_sources(Windows10Colors PRIVATE Windows10Colors/Windows10Colors/Windows10Colors.cpp Windows10Colors/Windows10Colors/Windows10Colors.h)
target_include_directories(Windows10Colors PUBLIC Windows10Colors/Windows10Colors/)
set_target_properties(Windows10Colors PROPERTIES
                      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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
set_target_properties(HDRTray PROPERTIES
                      WIN32_EXECUTABLE ON
                      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

install(TARGETS HDRTray
        RUNTIME
        DESTINATION ".")
