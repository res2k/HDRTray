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
    auto& status = *(reinterpret_cast<monitor_status_vec*>(param));

    MONITORINFOEXW monitor_info = {};
    monitor_info.cbSize = sizeof(monitor_info);
    auto hdr = GetMonitorHDRStatus(monitor, monitor_info);

    status.emplace_back(monitor_info.szDevice, hdr);

    return true;
}

monitor_status_vec GetWindowsHDRStatus()
{
    monitor_status_vec status;
    EnumDisplayMonitors(NULL, NULL, &monitor_enum_proc, reinterpret_cast<LPARAM>(&status));
    return status;
}

} // namespace hdr
