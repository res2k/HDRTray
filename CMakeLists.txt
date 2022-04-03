# CMakeList.txt : CMake project for HDRTray, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

project ("HDRTray")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_REVISION 0)
set(VERSION_BUILD 0)

configure_file(version.rc.in "${CMAKE_CURRENT_BINARY_DIR}/generated/version.rc")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package (Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    set(MARKDOWN_AVAILABLE NO)
    execute_process(COMMAND "${Python3_EXECUTABLE}" -m markdown
                    INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
                    OUTPUT_QUIET
                    RESULT_VARIABLE MARKDOWN_RESULT)
    if(MARKDOWN_RESULT EQUAL 0)
        set(MARKDOWN_AVAILABLE YES)
    endif()
    message(STATUS "Python markdown module available: ${MARKDOWN_AVAILABLE}")
else()
    message(STATUS "No Python 3 interpreter found!")
endif()

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

if(MARKDOWN_AVAILABLE)
    set(MD_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/md")
    make_directory("${MD_OUTPUT_DIR}")
    set(GENERATED_HTML_FILES)
    foreach(md_file LICENSE README)
        set(MD_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${md_file}.md")
        set(MD_OUTPUT "${MD_OUTPUT_DIR}/${md_file}.html")
        add_custom_command(OUTPUT "${MD_OUTPUT_DIR}/${md_file}.html"
                           MAIN_DEPENDENCY "${MD_INPUT}"
                           COMMAND "${Python3_EXECUTABLE}" -m markdown ${MD_INPUT} -f ${MD_OUTPUT}
                           COMMENT "Generating ${md_file}.html")
        list(APPEND GENERATED_HTML_FILES "${MD_OUTPUT}")
    endforeach()
    add_custom_target(ConvertMD ALL DEPENDS ${GENERATED_HTML_FILES})
endif()

install(TARGETS HDRTray
        RUNTIME
        DESTINATION ".")
if(MARKDOWN_AVAILABLE)
    foreach(md_file LICENSE README)
        install(FILES "${MD_OUTPUT_DIR}/${md_file}.html"
                DESTINATION ".")
    endforeach()
endif()
