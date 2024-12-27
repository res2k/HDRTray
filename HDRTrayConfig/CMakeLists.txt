include(arch_name)
include(find_platform_winmds)
include(generate_winappsdk)
include(generate_XamlMetaDataProvider)
include(make_pri)
include(merge_winmds)
include(run_cppwinrt)
include(target_xaml_compile)
include(target_xaml_sources)
include(winmd_from_idl)

set(ROOT_NAMESPACE "HDRTrayConfig") # must match XAML class namespaces etc.

# Arguments that are shared by multiple functions
set(common_args
    WINSDK_VERSION ${WINSDK_VERSION}
    NAMESPACE "${ROOT_NAMESPACE}"
    PCH "pch.h")

generate_XamlMetaDataProvider(OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/XamlMetaDataProvider"
                              IDL_VAR XamlMetaDataProvider_IDL
                              CPP_VAR XamlMetaDataProvider_CPP
                              ${common_args})


generate_winappsdk(TARGET HDRTrayConfig_generate_winappsdk OUT "${CMAKE_CURRENT_BINARY_DIR}/generated" WINMDS WINMD_PATHS ${common_args})

set(XamlMetaDataProvider_WINMD "${CMAKE_CURRENT_BINARY_DIR}/winmd_unmerged/XamlMetaDataProvider.winmd")
winmd_from_idl(OUT "${XamlMetaDataProvider_WINMD}"
               IN "${XamlMetaDataProvider_IDL}"
               REF ${WINMD_PATHS}
               ${common_args})

set(idl_winmds "")
# There are some dependencies between .idl files, so the order isn't completely arbitrary
foreach(idl_file IN ITEMS SettingsViewModel HDRDisplay HDRViewModel AboutPage HDRPage MainWindow SettingsPage ToggleCard)
    set(idl_winmd "${CMAKE_CURRENT_BINARY_DIR}/winmd_unmerged/${idl_file}.winmd")
    winmd_from_idl(OUT "${idl_winmd}"
                   IN "${CMAKE_CURRENT_SOURCE_DIR}/${idl_file}.idl"
                   REF ${WINMD_PATHS} ${idl_winmds}
                   ${common_args})
    list(APPEND idl_winmds "${idl_winmd}")
endforeach()

merge_winmds(TARGET HDRTrayConfig_merge_winmds
             OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/winmd_merged"
             IN "${XamlMetaDataProvider_WINMD}" ${idl_winmds}
             REF ${WINMD_PATHS}
             ${common_args})


run_cppwinrt(TARGET HDRTrayConfig_generate_cppwinrt
             IN "${CMAKE_CURRENT_BINARY_DIR}/winmd_merged/${ROOT_NAMESPACE}.winmd"
             OUT "${CMAKE_CURRENT_BINARY_DIR}/generated"
             COMPONENT "${CMAKE_CURRENT_BINARY_DIR}/generated/sources"
             COMPONENT_NAME "${ROOT_NAMESPACE}"
             PREFIX OPTIMIZE OVERWRITE
             REF ${WINMD_PATHS}
             GENERATED_SOURCES cppwinrt_generated
             ${common_args})

translate_system_processor(BUILD_ARCH "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}")

configure_file(version.rc.in "${CMAKE_CURRENT_BINARY_DIR}/generated/version.rc")

add_executable(HDRTrayConfig WIN32)
add_dependencies(HDRTrayConfig HDRTrayConfig_generate_winappsdk)
add_dependencies(HDRTrayConfig HDRTrayConfig_merge_winmds) # Avoids a warning in XAML compilation
target_sources(HDRTrayConfig PRIVATE
               "app.manifest"
               "${cppwinrt_generated}"
               "${XamlMetaDataProvider_CPP}"
               "SettingsViewModel.cpp"
               "SettingsViewModel.h"
               "HDRDisplay.cpp"
               "HDRDisplay.h"
               "HDRTrayConfig.rc"
               "HDRViewModel.cpp"
               "HDRViewModel.h"

               "${Microsoft_WindowsAppSDK_PATH}/include/MddBootstrapAutoInitializer.cpp"
               )
target_xaml_sources(HDRTrayConfig PRIVATE "App.xaml" VS_XAML_TYPE ApplicationDefinition)
target_xaml_sources(HDRTrayConfig PRIVATE "AboutPage.xaml" "HDRPage.xaml" "MainWindow.xaml" "SettingsPage.xaml" "ToggleCard.xaml")
target_compile_definitions(HDRTrayConfig PRIVATE UNICODE _UNICODE)
set_target_properties(HDRTrayConfig PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION ${WINSDK_VERSION})
target_include_directories(HDRTrayConfig PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_include_directories(HDRTrayConfig PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(HDRTrayConfig PRIVATE "${Microsoft_Windows_ImplementationLibrary_PATH}/include")
target_include_directories(HDRTrayConfig PRIVATE "${Microsoft_WindowsAppSDK_PATH}/include")
target_link_libraries(HDRTrayConfig PRIVATE common)
target_link_libraries(HDRTrayConfig PRIVATE "${Microsoft_WindowsAppSDK_PATH}/lib/win10-${BUILD_ARCH}/Microsoft.WindowsAppRuntime.Bootstrap.lib" comctl32 runtimeobject)
set_target_properties(HDRTrayConfig PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
target_precompile_headers(HDRTrayConfig PRIVATE pch.h)
target_xaml_compile(HDRTrayConfig
                    OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
                    MERGED_WINMD_DIR "${CMAKE_CURRENT_BINARY_DIR}/winmd_merged"
                    REF_WINMD ${WINMD_PATHS}
                    XBF_DIR_VAR xbf_dir
                    XBF_FILES_VAR xbf_files
                    ${common_args})
add_custom_command(TARGET HDRTrayConfig
                   POST_BUILD
                   COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${Microsoft_WindowsAppSDK_PATH}/runtimes/win-${BUILD_ARCH}/native/Microsoft.WindowsAppRuntime.Bootstrap.dll" "$<TARGET_FILE_DIR:HDRTrayConfig>")

set(pri_file "${CMAKE_CURRENT_BINARY_DIR}/resources.pri")
make_pri_postbuild(HDRTrayConfig
                   OUTPUT "${pri_file}"
                   DIRECTORY "${xbf_dir}"
                   FILES "${xbf_files}")
add_custom_command(TARGET HDRTrayConfig
                   POST_BUILD
                   COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${pri_file}" "$<TARGET_FILE_DIR:HDRTrayConfig>/HDRTrayConfig.pri")
