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
#include "WinVerCheck.hpp"

#if !defined(NTDDI_WIN11_GA) || WDK_NTDDI_VERSION < NTDDI_WIN11_GA
#error Windows SDK too old: Version >= 10.0.26100 required
#endif

namespace hdr {

static const bool use_win11_24h2_color_functions = IsWindows11_24H2OrGreater();

DisplayID DisplayID::FromMode(const DISPLAYCONFIG_MODE_INFO& mode)
{
    return DisplayID { .adapter = mode.adapterId, .id = mode.id };
}

void DisplayID::ToDeviceInputHeader(DISPLAYCONFIG_DEVICE_INFO_HEADER& header) const
{
    header.adapterId = adapter;
    header.id = id;
}

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

        func(DisplayID::FromMode(mode));
    }
}

static Status GetDisplayHDRStatus(const DisplayID& display)
{
    // Prefer GET_ADVANCED_COLOR_INFO_2, this reports the actual HDR mode if ACM is enabled
    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO_2 getColorInfo2 = {};
    getColorInfo2.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO_2;
    getColorInfo2.header.size = sizeof(getColorInfo2);
    display.ToDeviceInputHeader(getColorInfo2.header);
    if (use_win11_24h2_color_functions && DisplayConfigGetDeviceInfo(&getColorInfo2.header) == ERROR_SUCCESS)
    {
        if (!getColorInfo2.highDynamicRangeSupported)
            return Status::Unsupported;

        // Only DISPLAYCONFIG_ADVANCED_COLOR_MODE_HDR is true HDR.
        return getColorInfo2.activeColorMode == DISPLAYCONFIG_ADVANCED_COLOR_MODE_HDR ? Status::On : Status::Off;
    }

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
    getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
    getColorInfo.header.size = sizeof(getColorInfo);
    display.ToDeviceInputHeader(getColorInfo.header);

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

    ForEachDisplay([&](const DisplayID& display) {
        Status displayStatus = GetDisplayHDRStatus(display);
        anySupported |= displayStatus != Status::Unsupported;
        anyEnabled |= displayStatus == Status::On;
    });

    if (anySupported)
        return anyEnabled ? Status::On : Status::Off;
    else
        return Status::Unsupported;
}

static std::optional<Status> SetDisplayHDRStatus(const DisplayID& display, bool enable)
{
    if (GetDisplayHDRStatus(display) == Status::Unsupported)
        return std::nullopt;

    /* Try SET_HDR_STATE first, if available (on Windows 11 >= 24H2).
     * This seems to work better with ACM enabled (in which case "advanced color" is always
     * enabled and changing it doesn't do much.) */
    DISPLAYCONFIG_SET_HDR_STATE setHdrState = {};
    setHdrState.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE;
    setHdrState.header.size = sizeof(setHdrState);
    display.ToDeviceInputHeader(setHdrState.header);
    setHdrState.enableHdr = enable;
    if (use_win11_24h2_color_functions && DisplayConfigSetDeviceInfo(&setHdrState.header) == ERROR_SUCCESS)
        return GetDisplayHDRStatus(display);

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE setColorState = {};
    setColorState.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
    setColorState.header.size = sizeof(setColorState);
    display.ToDeviceInputHeader(setColorState.header);
    setColorState.enableAdvancedColor = enable;

    if (DisplayConfigSetDeviceInfo(&setColorState.header) != ERROR_SUCCESS)
        return std::nullopt;
    // Don't assume changing the HDR mode was successful... re-query the status
    return GetDisplayHDRStatus(display);
}

std::optional<Status> SetWindowsHDRStatus(bool enable)
{
    std::optional<Status> status;

    ForEachDisplay([&](const DisplayID& display) {
        auto new_status = SetDisplayHDRStatus(display, enable);
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

static const wchar_t* GetFallbackDisplayName(const DisplayID& display)
{
    DISPLAYCONFIG_TARGET_BASE_TYPE target_base = {};
    target_base.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_BASE_TYPE;
    target_base.header.size = sizeof(target_base);
    display.ToDeviceInputHeader(target_base.header);

    if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&target_base.header)) {
        if ((target_base.baseOutputTechnology != DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER)
            && (target_base.baseOutputTechnology & DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL))
            return L"Internal Display";
    }

    return L"Unnamed";
}

std::vector<DisplayInfo> GetDisplays()
{
    std::vector<DisplayInfo> result;

    ForEachDisplay([&](const DisplayID& display) {
        DisplayInfo new_disp;

        new_disp.status = GetDisplayHDRStatus(display);

        DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName = {};
        deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        deviceName.header.size = sizeof(deviceName);
        display.ToDeviceInputHeader(deviceName.header);
        if (DisplayConfigGetDeviceInfo(&deviceName.header) != ERROR_SUCCESS)
            return;

        if (deviceName.flags.friendlyNameFromEdid)
            new_disp.name = deviceName.monitorFriendlyDeviceName;
        else
            new_disp.name = GetFallbackDisplayName(display); // Seen with eg a laptop display.

        result.emplace_back(std::move(new_disp));
    });

    return result;
}

} // namespace hdr
