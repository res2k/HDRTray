configure_file(version.rc.in "${CMAKE_CURRENT_BINARY_DIR}/generated/version.rc")

# Add source to this project's executable.
add_executable(HDRTray)
target_sources(HDRTray PRIVATE
               "HDRTray.h"
               "HDRTray.cpp"
               "HDRTray.manifest"
               "HDRTray.rc"
               "NotifyIcon.hpp"
               "NotifyIcon.cpp"
               )
target_compile_definitions(HDRTray PRIVATE UNICODE _UNICODE)
target_include_directories(HDRTray PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(HDRTray PRIVATE common Windows10Colors comctl32)
set_target_properties(HDRTray PROPERTIES
                      WIN32_EXECUTABLE ON
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
