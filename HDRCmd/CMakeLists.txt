configure_file(version.h.in "${CMAKE_CURRENT_BINARY_DIR}/generated/version.h")
configure_file(version.rc.in "${CMAKE_CURRENT_BINARY_DIR}/generated/version.rc")

add_executable(HDRCmd)
target_sources(HDRCmd PRIVATE
               "HDRCmd.cpp"
               "HDRCmd.manifest"
               "HDRCmd.rc"
               "subcommand/display_status.hpp"
               "subcommand/display_status.cpp"
               "subcommand/resolve_display.hpp"
               "subcommand/resolve_display.cpp"
               "subcommand/Base.hpp"
               "subcommand/Disable.hpp"
               "subcommand/Disable.cpp"
               "subcommand/Enable.hpp"
               "subcommand/Enable.cpp"
               "subcommand/LoginStartup.hpp"
               "subcommand/LoginStartup.cpp"
               "subcommand/Select.hpp"
               "subcommand/Select.cpp"
               "subcommand/Status.hpp"
               "subcommand/Status.cpp"
               )
target_compile_definitions(HDRCmd PRIVATE UNICODE _UNICODE)
target_include_directories(HDRCmd PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(HDRCmd PRIVATE CLI11 common)
set_target_properties(HDRCmd PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
