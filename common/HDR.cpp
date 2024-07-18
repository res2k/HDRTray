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

    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS)
        return;

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), 0) != ERROR_SUCCESS)
        return;

    for (const auto& path : paths) {
        const auto& mode = modes.at(path.targetInfo.modeInfoIdx);

        func(mode);
    }
}

static Status GetDisplayHDRStatus(const DISPLAYCONFIG_MODE_INFO& mode)
{
    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
    getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
    getColorInfo.header.size = sizeof(getColorInfo);
    getColorInfo.header.adapterId.HighPart = mode.adapterId.HighPart;
    getColorInfo.header.adapterId.LowPart = mode.adapterId.LowPart;
    getColorInfo.header.id = mode.id;

    if (DisplayConfigGetDeviceInfo(&getColorInfo.header) != ERROR_SUCCESS)
        return Status::Unsupported;

    if (!getColorInfo.advancedColorSupported)
        return Status::Unsupported;

    return getColorInfo.advancedColorEnabled ? Status::On : Status::Off;
}

Status GetWindowsHDRStatus()
{
    bool anySupported = false;
    bool anyEnabled = false;

    ForEachDisplay([&](const DISPLAYCONFIG_MODE_INFO& mode) {
        Status displayStatus = GetDisplayHDRStatus(mode);
        anySupported = displayStatus != Status::Unsupported;
        anyEnabled = displayStatus == Status::On;
    });

    if (anySupported)
        return anyEnabled ? Status::On : Status::Off;
    else
        return Status::Unsupported;
}

static std::optional<Status> SetDisplayHDRStatus(const DISPLAYCONFIG_MODE_INFO& mode, bool enable)
{
    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
    getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
    getColorInfo.header.size = sizeof(getColorInfo);
    getColorInfo.header.adapterId.HighPart = mode.adapterId.HighPart;
    getColorInfo.header.adapterId.LowPart = mode.adapterId.LowPart;
    getColorInfo.header.id = mode.id;

    if (DisplayConfigGetDeviceInfo(&getColorInfo.header) != ERROR_SUCCESS)
        return std::nullopt;

    if (!getColorInfo.advancedColorSupported)
        return std::nullopt;

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE setColorState = {};
    setColorState.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
    setColorState.header.size = sizeof(setColorState);
    setColorState.header.adapterId.HighPart = mode.adapterId.HighPart;
    setColorState.header.adapterId.LowPart = mode.adapterId.LowPart;
    setColorState.header.id = mode.id;
    setColorState.enableAdvancedColor = enable;

    if (DisplayConfigSetDeviceInfo(&setColorState.header) != ERROR_SUCCESS)
        return std::nullopt;
    // Don't assume changing the HDR mode was successful... re-query the status
    return GetDisplayHDRStatus(mode);
}

std::optional<Status> SetWindowsHDRStatus(bool enable)
{
    std::optional<Status> status;

    ForEachDisplay([&](const DISPLAYCONFIG_MODE_INFO& mode) {
        auto new_status = SetDisplayHDRStatus(mode, enable);
        if (!new_status)
            return;

        if(!status)
            status = *new_status;
        else
            status = static_cast<Status>(std::max(static_cast<int>(*status), static_cast<int>(*new_status)));
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

static const wchar_t* GetFallbackDisplayName(const DISPLAYCONFIG_MODE_INFO& mode)
{
    DISPLAYCONFIG_TARGET_BASE_TYPE target_base = {};
    target_base.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_BASE_TYPE;
    target_base.header.size = sizeof(target_base);
    target_base.header.adapterId.HighPart = mode.adapterId.HighPart;
    target_base.header.adapterId.LowPart = mode.adapterId.LowPart;
    target_base.header.id = mode.id;

    if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&target_base.header)) {
        if ((target_base.baseOutputTechnology != DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER)
            && (target_base.baseOutputTechnology & DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL))
            return L"Internal Display";
    }

    return L"Unnamed";
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

        if (DisplayConfigGetDeviceInfo(&getColorInfo.header) != ERROR_SUCCESS)
            return;

        if (getColorInfo.advancedColorSupported)
            new_disp.status = getColorInfo.advancedColorEnabled ? Status::On : Status::Off;
        else
            new_disp.status = Status::Unsupported;

        DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName = {};
        deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        deviceName.header.size = sizeof(deviceName);
        deviceName.header.adapterId.HighPart = mode.adapterId.HighPart;
        deviceName.header.adapterId.LowPart = mode.adapterId.LowPart;
        deviceName.header.id = mode.id;
        if (DisplayConfigGetDeviceInfo(&deviceName.header) != ERROR_SUCCESS)
            return;

        if (deviceName.flags.friendlyNameFromEdid)
            new_disp.name = deviceName.monitorFriendlyDeviceName;
        else
            new_disp.name = GetFallbackDisplayName(mode); // Seen with eg a laptop display.

        result.emplace_back(std::move(new_disp));
    });

    return result;
}

} // namespace hdr
