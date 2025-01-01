// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <MddBootstrap.h>
#include <WindowsAppSDK-VersionInfo.h>
#include <CommCtrl.h>

#include "Resource.h"

#include <format>

namespace Microsoft::Windows::ApplicationModel::DynamicDependency::Bootstrap
{
    struct AutoInitialize
    {
        AutoInitialize()
        {
            Initialize();
        }

        ~AutoInitialize()
        {
            ::MddBootstrapShutdown();
        }

        constexpr static MddBootstrapInitializeOptions Options()
        {
            return MddBootstrapInitializeOptions_None;
        }

        static void Initialize()
        {
            const UINT32 c_majorMinorVersion{ WINDOWSAPPSDK_RELEASE_MAJORMINOR };
            PCWSTR c_versionTag{ WINDOWSAPPSDK_RELEASE_VERSION_TAG_W };
            const PACKAGE_VERSION c_minVersion{ WINDOWSAPPSDK_RUNTIME_VERSION_UINT64 };
            const auto c_options{ Options() };
            const HRESULT hr{ ::MddBootstrapInitialize2(c_majorMinorVersion, c_versionTag, c_minVersion, c_options) };
            if (FAILED(hr))
            {
                if (hr == STATEREPOSITORY_E_DEPENDENCY_NOT_RESOLVED)
                    DownloadRuntimeDialog();
                exit(hr);
            }
        }

        static void DownloadRuntimeDialog()
        {
            auto body_text =
                std::format(L"HDRTray Configuration requires the Windows App Runtime {0}.{1}, but a suitable version was "
                            L"not found on your system.\n"
                            "Installers for the required Windows App Runtime version can be downloaded online.\n\n"
                            "Download the latest Windows App Runtime {0}.{1} installer now?",
                            WINDOWSAPPSDK_RELEASE_MAJOR, WINDOWSAPPSDK_RELEASE_MINOR);

            TASKDIALOGCONFIG task_dialog = { sizeof(task_dialog) };
            task_dialog.hInstance = GetModuleHandle(nullptr);
            task_dialog.dwFlags = TDF_SIZE_TO_CONTENT;
            task_dialog.dwCommonButtons = TDCBF_CANCEL_BUTTON;
            task_dialog.pszWindowTitle = MAKEINTRESOURCE(IDS_APP_TITLE);
            task_dialog.pszMainIcon = TD_ERROR_ICON;
            task_dialog.pszMainInstruction = MAKEINTRESOURCE(IDS_APP_RUNTIME_MISSING);
            task_dialog.pszContent = body_text.c_str();

            TASKDIALOG_BUTTON buttons[] = { { .nButtonID = IDOK, .pszButtonText = MAKEINTRESOURCE(IDS_DOWNLOAD_APP_RUNTIME) } };
            task_dialog.cButtons = std::size(buttons);
            task_dialog.pButtons = buttons;
            task_dialog.nDefaultButton = IDOK;

            int result_button = 0;
            if (SUCCEEDED(TaskDialogIndirect(&task_dialog, &result_button, nullptr, nullptr)) && result_button == IDOK) {
                auto download_url =
                    std::format(L"https://aka.ms/windowsappsdk/{}.{}/latest/windowsappruntimeinstall-x64.exe",
                                WINDOWSAPPSDK_RELEASE_MAJOR, WINDOWSAPPSDK_RELEASE_MINOR);
                ShellExecuteW(NULL, L"open", download_url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            }
        }
    };
    static AutoInitialize g_autoInitialize;
}
