configure_file(version.rc.in "${CMAKE_CURRENT_BINARY_DIR}/generated/version.rc")

add_executable(HDRCmd)
target_sources(HDRCmd PRIVATE
               "HDRCmd.cpp"
               "HDRCmd.rc"
               "Subcommands.hpp"
               "Subcommands.cpp"
               )
target_compile_definitions(HDRCmd PRIVATE UNICODE _UNICODE)
target_include_directories(HDRCmd PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(HDRCmd PRIVATE CLI11 HDR common)
set_target_properties(HDRCmd PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
