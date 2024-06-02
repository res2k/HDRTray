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

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace hdr {

enum class Status { Unsupported = 0, Off = 1, On = 2 };

/// Display information
struct Display
{
    /// Display name
    std::wstring name;
    /// HDR status
    Status status;
};

Status GetWindowsHDRStatus();
std::optional<Status> SetWindowsHDRStatus(bool enable);
std::optional<Status> ToggleHDRStatus();
/// Get information for all displays
std::vector<Display> GetDisplays();

} // namespace hdr

#endif // HDR_H_
