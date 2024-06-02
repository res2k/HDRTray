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

std::optional<Status> SetWindowsHDRStatus(bool enable)
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
                setColorState.enableAdvancedColor = enable;
                new_status = enable ? Status::On : Status::Off;
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

std::optional<Status> ToggleHDRStatus()
{
    auto status = GetWindowsHDRStatus();
    if (status == Status::Unsupported)
        return Status::Unsupported;
    return SetWindowsHDRStatus(status == Status::Off ? true : false);
}

std::vector<Display> GetDisplays()
{
    std::vector<Display> result;

    ForEachDisplay([&](const DISPLAYCONFIG_MODE_INFO& mode) {
        Display new_disp;

        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
        getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
        getColorInfo.header.size = sizeof(getColorInfo);
        getColorInfo.header.adapterId.HighPart = mode.adapterId.HighPart;
        getColorInfo.header.adapterId.LowPart = mode.adapterId.LowPart;
        getColorInfo.header.id = mode.id;

        if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&getColorInfo.header)) {
            if (getColorInfo.advancedColorSupported)
                new_disp.status = getColorInfo.advancedColorEnabled ? Status::On : Status::Off;
            else
                new_disp.status = Status::Unsupported;
        }
        else
            return;

        DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName = {};
        deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        deviceName.header.size = sizeof(deviceName);
        deviceName.header.adapterId.HighPart = mode.adapterId.HighPart;
        deviceName.header.adapterId.LowPart = mode.adapterId.LowPart;
        deviceName.header.id = mode.id;
        if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&deviceName.header))
            new_disp.name = deviceName.monitorFriendlyDeviceName;
        else
            return;

        result.emplace_back(std::move(new_disp));
    });

    return result;
}

} // namespace hdr
