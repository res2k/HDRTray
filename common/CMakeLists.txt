add_library(common INTERFACE)
target_sources(common PRIVATE
               "WinVerCheck.hpp"
               )
target_include_directories(common INTERFACE .)
