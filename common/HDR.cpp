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

//---------------------------------------------------------------------------

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

DisplayInfo::result_type<std::wstring> DisplayInfo::GetName() const
{
    const auto& dev_name = GetCached<DISPLAYCONFIG_TARGET_DEVICE_NAME>(
        deviceName, [this]() -> result_type<DISPLAYCONFIG_TARGET_DEVICE_NAME> {
            DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName = {};
            deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
            deviceName.header.size = sizeof(deviceName);
            id.ToDeviceInputHeader(deviceName.header);
            LONG result = DisplayConfigGetDeviceInfo(&deviceName.header);
            if (result != ERROR_SUCCESS)
                return std::unexpected(result);
            return deviceName;
        }, ValueFreshness::Cached);

    if (dev_name) {
        if (dev_name->flags.friendlyNameFromEdid)
            return dev_name->monitorFriendlyDeviceName;
        else
            return GetFallbackDisplayName(id); // Seen with eg a laptop display.
    }
    return std::unexpected(dev_name.error());
}

DisplayInfo::result_type<Status> DisplayInfo::GetStatus(ValueFreshness freshness) const
{
    return GetCached<Status>(
        status,
        [this]() -> result_type<Status> {
            // Prefer GET_ADVANCED_COLOR_INFO_2, this reports the actual HDR mode if ACM is enabled
            DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO_2 getColorInfo2 = {};
            getColorInfo2.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO_2;
            getColorInfo2.header.size = sizeof(getColorInfo2);
            id.ToDeviceInputHeader(getColorInfo2.header);
            if (use_win11_24h2_color_functions && DisplayConfigGetDeviceInfo(&getColorInfo2.header) == ERROR_SUCCESS)
            {
                if (!getColorInfo2.highDynamicRangeSupported)
                    return Status::Unsupported;

                return getColorInfo2.activeColorMode == DISPLAYCONFIG_ADVANCED_COLOR_MODE_HDR ? Status::On : Status::Off;
            }

            DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
            getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
            getColorInfo.header.size = sizeof(getColorInfo);
            id.ToDeviceInputHeader(getColorInfo.header);
            LONG result = DisplayConfigGetDeviceInfo(&getColorInfo.header);
            if (result != ERROR_SUCCESS)
                return std::unexpected(result);
            if (getColorInfo.advancedColorSupported)
                return getColorInfo.advancedColorEnabled ? Status::On : Status::Off;
            else
                return Status::Unsupported;
        },
        freshness);
}

template<typename T, typename F>
const DisplayInfo::result_type<T>& DisplayInfo::GetCached(cache_type<T>& cache, F produce, ValueFreshness freshness) const
{
    if(!cache || freshness == ValueFreshness::ForceRefresh)
        cache = produce();
    return *cache;
}

//---------------------------------------------------------------------------

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

Status GetWindowsHDRStatus(const DisplayInfo_vec& displays)
{
    bool anySupported = false;
    bool anyEnabled = false;
    for (auto& display : displays) {
        auto status = display.GetStatus();
        if (!status || *status == Status::Unsupported)
            continue;
        anySupported = true;
        anyEnabled |= *status == Status::On;
    }

    if (!anySupported)
        return Status::Unsupported;
    return anyEnabled ? Status::On : Status::Off;
}

static std::optional<Status> SetDisplayHDRStatus(const DisplayInfo& display, bool enable)
{
    if (display.GetStatus().value_or(Status::Unsupported) == Status::Unsupported)
        return std::nullopt;

    /* Try SET_HDR_STATE first, if available (on Windows 11 >= 24H2).
     * This seems to work better with ACM enabled (in which case "advanced color" is always
     * enabled and changing it doesn't do much.) */
    DISPLAYCONFIG_SET_HDR_STATE setHdrState = {};
    setHdrState.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE;
    setHdrState.header.size = sizeof(setHdrState);
    display.GetID().ToDeviceInputHeader(setHdrState.header);
    setHdrState.enableHdr = enable;
    if (use_win11_24h2_color_functions && DisplayConfigSetDeviceInfo(&setHdrState.header) == ERROR_SUCCESS)
        return display.GetStatus(DisplayInfo::ValueFreshness::ForceRefresh).value_or(Status::Unsupported);

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE setColorState = {};
    setColorState.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
    setColorState.header.size = sizeof(setColorState);
    display.GetID().ToDeviceInputHeader(setColorState.header);
    setColorState.enableAdvancedColor = enable;

    if (DisplayConfigSetDeviceInfo(&setColorState.header) != ERROR_SUCCESS)
        return std::nullopt;
    // Don't assume changing the HDR mode was successful... re-query the status
    return display.GetStatus(DisplayInfo::ValueFreshness::ForceRefresh).value_or(Status::Unsupported);
}

std::optional<Status> SetWindowsHDRStatus(const DisplayInfo_vec& displays, bool enable)
{
    std::optional<Status> status_result;

    for (auto& display : displays) {
        auto new_status = SetDisplayHDRStatus(display, enable);
        if (!new_status)
            continue;

        if(!status_result)
            status_result = *new_status;
        else
            status_result = static_cast<Status>(std::max(static_cast<int>(*status_result), static_cast<int>(*new_status)));
    }

    return status_result;
}

std::optional<Status> ToggleHDRStatus(const DisplayInfo_vec& displays)
{
    auto status = GetWindowsHDRStatus(displays);
    if (status == Status::Unsupported)
        return Status::Unsupported;
    return SetWindowsHDRStatus(displays, status == Status::Off ? true : false);
}

std::vector<DisplayInfo> GetDisplays()
{
    std::vector<DisplayInfo> result;

    ForEachDisplay([&](const DisplayID& display) {
        result.emplace_back(display);
    });

    return result;
}

} // namespace hdr
