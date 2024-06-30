/*
 *  Copyright (C) 2005-2020 Team Kodi
 *
 *  This file is based on source code from Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#ifndef HDR_H_
#define HDR_H_

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "framework.h"

namespace hdr {

enum class Status { Unsupported = 0, Off = 1, On = 2 };

struct DisplayID
{
    LUID adapter;
    UINT32 id;

    static DisplayID FromMode(const DISPLAYCONFIG_MODE_INFO& mode);
    void ToDeviceInputHeader(DISPLAYCONFIG_DEVICE_INFO_HEADER& header) const;
};

/// Display information
class DisplayInfo
{
public:
    DisplayInfo(DisplayID id) : id(id) {}

    /// Get display ID
    const DisplayID& GetID() const { return id; }

    template<typename T>
    using result_type = std::expected<T, LONG>;

    /// Indicates how "fresh" a queried value should be
    enum struct ValueFreshness { Cached, ForceRefresh };

    /// Get name of display
    result_type<std::wstring> GetName() const;
    /// Get HDR status for display
    result_type<Status> GetStatus(ValueFreshness freshness = ValueFreshness::Cached) const;

private:
    template<typename T>
    using cache_type = std::optional<result_type<T>>;

    /// Display ID
    DisplayID id;
    /// Display name
    mutable cache_type<DISPLAYCONFIG_TARGET_DEVICE_NAME> deviceName;
    /// HDR status
    mutable cache_type<Status> status;

    template<typename T, typename F>
    const result_type<T>& GetCached(cache_type<T>& cache, F produce, ValueFreshness freshness) const;
};

Status GetWindowsHDRStatus();
std::optional<Status> SetWindowsHDRStatus(bool enable);
std::optional<Status> ToggleHDRStatus();
/// Get information for all displays
std::vector<DisplayInfo> GetDisplays();

} // namespace hdr

#endif // HDR_H_
