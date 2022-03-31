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

template<typename F> static void ForEachDisplay(F func)
{
    uint32_t pathCount = 0;
    uint32_t modeCount = 0;

    if (ERROR_SUCCESS == GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount)) {
        std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
        std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

        if (ERROR_SUCCESS
            == QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), 0)) {

            DISPLAYCONFIG_SOURCE_DEVICE_NAME getSourceName = {};
            getSourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            getSourceName.header.size = sizeof(getSourceName);

            for (const auto& path : paths) {
                const auto& mode = modes.at(path.targetInfo.modeInfoIdx);

                func(mode);
            }
        }
    }
}

Status GetWindowsHDRStatus()
{
    bool advancedColorSupported = false;
    bool advancedColorEnabled = false;
    Status status = Status::Unsupported;

    ForEachDisplay([&](const DISPLAYCONFIG_MODE_INFO& mode) {
        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
        getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
        getColorInfo.header.size = sizeof(getColorInfo);
        getColorInfo.header.adapterId.HighPart = mode.adapterId.HighPart;
        getColorInfo.header.adapterId.LowPart = mode.adapterId.LowPart;
        getColorInfo.header.id = mode.id;

        if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&getColorInfo.header)) {
            if (getColorInfo.advancedColorEnabled)
                advancedColorEnabled = true;

            if (getColorInfo.advancedColorSupported)
                advancedColorSupported = true;
        }
    });

    if (!advancedColorSupported) {
        status = Status::Unsupported;
    } else {
        status = advancedColorEnabled ? Status::On : Status::Off;
    }

    return status;
}

std::optional<Status> ToggleHDRStatus()
{
    std::optional<Status> status;

    ForEachDisplay([&](const DISPLAYCONFIG_MODE_INFO& mode) {
        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
        getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
        getColorInfo.header.size = sizeof(getColorInfo);
        getColorInfo.header.adapterId.HighPart = mode.adapterId.HighPart;
        getColorInfo.header.adapterId.LowPart = mode.adapterId.LowPart;
        getColorInfo.header.id = mode.id;

        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE setColorState = {};
        setColorState.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
        setColorState.header.size = sizeof(setColorState);
        setColorState.header.adapterId.HighPart = mode.adapterId.HighPart;
        setColorState.header.adapterId.LowPart = mode.adapterId.LowPart;
        setColorState.header.id = mode.id;

        if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&getColorInfo.header)) {
            if (getColorInfo.advancedColorSupported) {
                Status new_status = Status::Unsupported;
                if (getColorInfo.advancedColorEnabled) // HDR is ON
                {
                    setColorState.enableAdvancedColor = FALSE;
                    new_status = Status::Off;
                } else // HDR is OFF
                {
                    setColorState.enableAdvancedColor = TRUE;
                    new_status = Status::On;
                }
                if (ERROR_SUCCESS == DisplayConfigSetDeviceInfo(&setColorState.header)) {
                    if(!status)
                        status = new_status;
                    else
                        status = static_cast<Status>(std::max(static_cast<int>(*status), static_cast<int>(new_status)));
                }
            }
        }
    });

    return status;
}

} // namespace hdr
