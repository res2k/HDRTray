configure_file(version.h.in "${CMAKE_CURRENT_BINARY_DIR}/generated/version.h")

add_library(common STATIC)
target_sources(common PRIVATE
               "DisplayConfig.hpp"
               "DisplayConfig.cpp"
               "LoginStartupConfig.hpp"
               "LoginStartupConfig.cpp"
               "HDR.h"
               "HDR.cpp"
               "RegistryKey.hpp"
               "WinVerCheck.hpp"
               )
target_compile_definitions(common PRIVATE UNICODE _UNICODE)
target_include_directories(common PUBLIC .)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/generated")
