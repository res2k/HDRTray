add_library(HDR STATIC)
target_sources(HDR PRIVATE "HDR.h" "HDR.cpp")
target_link_libraries(HDR PUBLIC common)
target_compile_definitions(HDR PRIVATE UNICODE _UNICODE)
target_include_directories(HDR PUBLIC .)
