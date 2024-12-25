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

    bool operator==(const DisplayID& other) const { return memcmp(&adapter, &other.adapter, sizeof(LUID)) == 0 && id == other.id; }
    std::strong_ordering operator<=>(const DisplayID& other) const
    {
        int luid_cmp = memcmp(&adapter, &other.adapter, sizeof(LUID));
        if (luid_cmp < 0)
            return std::strong_ordering::less;
        else if (luid_cmp > 0)
            return std::strong_ordering::greater;
        return std::compare_three_way{}(id, other.id);
    }
};

/// Display information
class DisplayInfo
{
public:
    DisplayInfo(size_t index, DisplayID id) : index(index), id(id) {}

    /// Get display index
    size_t GetIndex() const { return index; }
    /// Get display ID
    const DisplayID& GetID() const { return id; }

    template<typename T>
    using result_type = std::expected<T, HRESULT>;

    /// Indicates how "fresh" a queried value should be
    enum struct ValueFreshness { Cached, ForceRefresh };

    /// Get name of display
    result_type<std::wstring> GetName() const;
    /// Get HDR status for display
    result_type<Status> GetStatus(ValueFreshness freshness = ValueFreshness::Cached) const;
    /// Get "stable ID" of display
    result_type<std::wstring> GetStableID() const;

private:
    template<typename T>
    using cache_type = std::optional<result_type<T>>;

    /// Display index
    size_t index;
    /// Display ID
    DisplayID id;
    /// Display name
    mutable cache_type<DISPLAYCONFIG_TARGET_DEVICE_NAME> deviceName;
    /// HDR status
    mutable cache_type<Status> status;
    /// Stable name
    mutable cache_type<std::wstring> stableID;

    template<typename T, typename F>
    const result_type<T>& GetCached(cache_type<T>& cache, F produce, ValueFreshness freshness) const;
    const result_type<DISPLAYCONFIG_TARGET_DEVICE_NAME>& GetCachedDeviceName() const;
};

using DisplayInfo_vec = std::vector<DisplayInfo>;

Status GetWindowsHDRStatus(const DisplayInfo_vec& displays);
std::optional<Status> SetWindowsHDRStatus(const DisplayInfo_vec& displays, bool enable);
std::optional<Status> ToggleHDRStatus(const DisplayInfo_vec& displays);

/// Get information for all displays
std::vector<DisplayInfo> GetDisplays();
/// Get information for enabled displays (via DisplayConfig)
std::vector<DisplayInfo> GetEnabledDisplays();

static inline Status GetWindowsHDRStatus()
{
    return GetWindowsHDRStatus(GetDisplays());
}
static inline std::optional<Status> SetWindowsHDRStatus(bool enable)
{
    return SetWindowsHDRStatus(GetDisplays(), enable);
}
static inline std::optional<Status> ToggleHDRStatus()
{
    return ToggleHDRStatus(GetDisplays());
}

} // namespace hdr

#endif // HDR_H_
