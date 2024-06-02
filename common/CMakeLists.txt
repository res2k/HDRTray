add_library(common STATIC)
target_sources(common PRIVATE
               "HDR.h"
               "HDR.cpp"
               "WinVerCheck.hpp"
               )
target_compile_definitions(common PRIVATE UNICODE _UNICODE)
target_include_directories(common PUBLIC .)
