# CMakeList.txt : CMake project for HDRTray, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15...3.25)

include(build/GitVersionDetect.cmake)

project ("HDRTray")
set(VERSION_MAJOR ${GITVERSIONDETECT_VERSION_MAJOR})
set(VERSION_MINOR ${GITVERSIONDETECT_VERSION_MINOR})
set(VERSION_REVISION ${GITVERSIONDETECT_VERSION_PATCH})
set(VERSION_BUILD ${GITVERSIONDETECT_VERSION_COMMIT_NUM})
set(VERSION_FULL ${GITVERSIONDETECT_VERSION})
set(VERSION_COMMIT ${GITVERSIONDETECT_VERSION_COMMIT_SHA})
set(COPYRIGHT_STR "Copyright (c) 2022-2025 Frank Richter")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Python3_FIND_REGISTRY LAST)
find_package (Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    set(python_venv_dir "${CMAKE_BINARY_DIR}/python_venv")
    make_directory(${python_venv_dir})
    execute_process(COMMAND "${Python3_EXECUTABLE}" -m venv "${python_venv_dir}")
    set(Python3_EXECUTABLE "${python_venv_dir}/Scripts/python.exe")
    execute_process(COMMAND "${Python3_EXECUTABLE}" -m pip install marko
                    WORKING_DIRECTORY "${python_venv_dir}")
    set(MARKO_AVAILABLE NO)
    execute_process(COMMAND "${Python3_EXECUTABLE}" -m marko
                    INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
                    OUTPUT_QUIET
                    RESULT_VARIABLE MARKO_RESULT)
    if(MARKO_RESULT EQUAL 0)
        set(MARKO_AVAILABLE YES)
    endif()
    message(STATUS "Python marko module available: ${MARKO_AVAILABLE}")
else()
    message(STATUS "No Python 3 interpreter found!")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

add_subdirectory(ext)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext/winui3_cmake/cmake")

add_subdirectory(common)

add_subdirectory(HDRCmd)
add_subdirectory(HDRTray)
add_subdirectory(HDRTrayConfig)

if(MARKO_AVAILABLE)
    set(MD2HTML "${CMAKE_CURRENT_SOURCE_DIR}/build/md2html.py")
    set(MD_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(GENERATED_HTML_FILES)
    foreach(md_file LICENSE README)
        set(MD_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${md_file}.md")
        set(MD_OUTPUT "${MD_OUTPUT_DIR}/${md_file}.html")
        add_custom_command(OUTPUT "${MD_OUTPUT_DIR}/${md_file}.html"
                           MAIN_DEPENDENCY "${MD_INPUT}"
                           DEPENDS "${MD2HTML}"
                           COMMAND "${Python3_EXECUTABLE}" "${MD2HTML}" "${MD_INPUT}" "${MD_OUTPUT}"
                           COMMENT "Generating ${md_file}.html")
        list(APPEND GENERATED_HTML_FILES "${MD_OUTPUT}")
    endforeach()
    add_custom_target(ConvertMD ALL DEPENDS ${GENERATED_HTML_FILES})
endif()

install(TARGETS HDRTray HDRCmd
        RUNTIME
        DESTINATION ".")
if(MARKO_AVAILABLE)
    foreach(md_file LICENSE README)
        install(FILES "${MD_OUTPUT_DIR}/${md_file}.html"
                DESTINATION ".")
    endforeach()
endif()
