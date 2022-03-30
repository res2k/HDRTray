/*
 *  Copyright (C) 2005-2018 Team Kodi
 *
 *  This file is based on source code from Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "HDR.h"

#include <cstdint>
#include <string>
#include <vector>

#include "framework.h"

namespace hdr {
static Status GetMonitorHDRStatus(HMONITOR monitor, MONITORINFOEXW& monitor_info)
{
    bool advancedColorSupported = false;
    bool advancedColorEnabled = false;
    Status status = Status::Unsupported;

    uint32_t pathCount = 0;
    uint32_t modeCount = 0;

    GetMonitorInfoW(monitor, &monitor_info);
    const wchar_t* deviceNameW = monitor_info.szDevice;

    if (ERROR_SUCCESS == GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount)) {
        std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
        std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

        if (ERROR_SUCCESS
            == QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), 0)) {
            DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
            getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
            getColorInfo.header.size = sizeof(getColorInfo);

            DISPLAYCONFIG_SOURCE_DEVICE_NAME getSourceName = {};
            getSourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            getSourceName.header.size = sizeof(getSourceName);

            for (const auto& path : paths) {
                getSourceName.header.adapterId.HighPart = path.sourceInfo.adapterId.HighPart;
                getSourceName.header.adapterId.LowPart = path.sourceInfo.adapterId.LowPart;
                getSourceName.header.id = path.sourceInfo.id;

                if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&getSourceName.header)) {
                    const wchar_t* sourceNameW = getSourceName.viewGdiDeviceName;
                    if (wcscmp(deviceNameW, sourceNameW) == 0){
                        const auto& mode = modes.at(path.targetInfo.modeInfoIdx);

                        getColorInfo.header.adapterId.HighPart = mode.adapterId.HighPart;
                        getColorInfo.header.adapterId.LowPart = mode.adapterId.LowPart;
                        getColorInfo.header.id = mode.id;

                        if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&getColorInfo.header)) {
                            if (getColorInfo.advancedColorEnabled)
                                advancedColorEnabled = true;

                            if (getColorInfo.advancedColorSupported)
                                advancedColorSupported = true;
                        }
                    }
                }
            }
        }
    }

    if (!advancedColorSupported) {
        status = Status::Unsupported;
    } else {
        status = advancedColorEnabled ? Status::On : Status::Off;
    }

    return status;
}

static BOOL monitor_enum_proc(HMONITOR monitor, HDC /*dc*/, LPRECT /*rect*/, LPARAM param)
{
    auto& status = *(reinterpret_cast<Status*>(param));

    MONITORINFOEXW monitor_info = {};
    monitor_info.cbSize = sizeof(monitor_info);
    auto hdr = GetMonitorHDRStatus(monitor, monitor_info);

    status = static_cast<Status>(std::max(static_cast<int>(status), static_cast<int>(hdr)));

    return true;
}

Status GetWindowsHDRStatus()
{
    Status status = Status::Unsupported;
    EnumDisplayMonitors(NULL, NULL, &monitor_enum_proc, reinterpret_cast<LPARAM>(&status));
    return status;
}

static std::optional<Status> ToggleMonitorHDR(HMONITOR monitor, MONITORINFOEXW& monitor_info)
{
    std::optional<Status> status;

    uint32_t pathCount = 0;
    uint32_t modeCount = 0;

    GetMonitorInfoW(monitor, &monitor_info);
    const wchar_t* deviceNameW = monitor_info.szDevice;

    if (ERROR_SUCCESS == GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount)) {
        std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
        std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

        if (ERROR_SUCCESS
            == QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), nullptr)) {
            DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
            getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
            getColorInfo.header.size = sizeof(getColorInfo);

            DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE setColorState = {};
            setColorState.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
            setColorState.header.size = sizeof(setColorState);

            DISPLAYCONFIG_SOURCE_DEVICE_NAME getSourceName = {};
            getSourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            getSourceName.header.size = sizeof(getSourceName);

            // Only try to toggle display currently used by Kodi
            for (const auto& path : paths) {
                getSourceName.header.adapterId.HighPart = path.sourceInfo.adapterId.HighPart;
                getSourceName.header.adapterId.LowPart = path.sourceInfo.adapterId.LowPart;
                getSourceName.header.id = path.sourceInfo.id;

                if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&getSourceName.header)) {
                    const wchar_t* sourceNameW = getSourceName.viewGdiDeviceName;
                    if (wcscmp(deviceNameW, sourceNameW) == 0) {
                        const auto& mode = modes.at(path.targetInfo.modeInfoIdx);

                        getColorInfo.header.adapterId.HighPart = mode.adapterId.HighPart;
                        getColorInfo.header.adapterId.LowPart = mode.adapterId.LowPart;
                        getColorInfo.header.id = mode.id;

                        setColorState.header.adapterId.HighPart = mode.adapterId.HighPart;
                        setColorState.header.adapterId.LowPart = mode.adapterId.LowPart;
                        setColorState.header.id = mode.id;

                        if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&getColorInfo.header)) {
                            if (getColorInfo.advancedColorSupported) {
                                if (getColorInfo.advancedColorEnabled) // HDR is ON
                                {
                                    setColorState.enableAdvancedColor = FALSE;
                                    status = Status::Off;
                                } else // HDR is OFF
                                {
                                    setColorState.enableAdvancedColor = TRUE;
                                    status = Status::On;
                                }
                                if (ERROR_SUCCESS != DisplayConfigSetDeviceInfo(&setColorState.header))
                                    status = std::nullopt;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    return status;
}

static BOOL toggle_monitor_enum_proc(HMONITOR monitor, HDC /*dc*/, LPRECT /*rect*/, LPARAM param)
{
    std::optional<Status>& status = *(reinterpret_cast<std::optional<Status>*>(param));

    MONITORINFOEXW monitor_info = {};
    monitor_info.cbSize = sizeof(monitor_info);
    auto toggle_res = ToggleMonitorHDR(monitor, monitor_info);

    if(toggle_res) {
        if(!status)
            status = toggle_res;
        else
            status = static_cast<Status>(std::max(static_cast<int>(*status), static_cast<int>(*toggle_res)));
    }

    return true;
}

std::optional<Status> ToggleHDRStatus()
{
    std::optional<Status> result;
    EnumDisplayMonitors(NULL, NULL, &toggle_monitor_enum_proc, reinterpret_cast<LPARAM>(&result));
    return result;
}


} // namespace hdr
