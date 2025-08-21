/*
    HDRTray, a notification icon for the "Use HDR" option
    Copyright (C) 2024 Frank Richter

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/**\file
 * Per-display configuration
 */
#ifndef DISPLAYCONFIG_HPP_
#define DISPLAYCONFIG_HPP_

#include "framework.h"

#include <expected>
#include <string_view>

class DisplayConfig
{
public:
    /// Path of display configuration below HKCU
    static const wchar_t displayconfig_hkcu_path[];

    /// Return reference to singleton
    static DisplayConfig& instance();

    /// Read enable flag from registry
    std::expected<bool, LSTATUS> GetEnabledFlag(std::wstring_view display_stable_id) const;
    /// Write enable flag to registry
    std::expected<void, LSTATUS> SetEnabledFlag(std::wstring_view display_stable_id, bool flag);

    /// Whether given display is currently enabled
    bool IsEnabled(std::wstring_view display_stable_id) const
    {
        return GetEnabledFlag(display_stable_id).value_or(true);
    }
};

#endif // DISPLAYCONFIG_HPP_
