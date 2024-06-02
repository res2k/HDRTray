set(CLI11_BUILD_DOCS OFF)
set(CLI11_PRECOMPILED ON)
add_subdirectory(CLI11)

add_library(Windows10Colors STATIC)
target_sources(Windows10Colors PRIVATE Windows10Colors/Windows10Colors/Windows10Colors.cpp Windows10Colors/Windows10Colors/Windows10Colors.h)
target_include_directories(Windows10Colors PUBLIC Windows10Colors/Windows10Colors/)
