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

#include "DisplayConfig.hpp"

#include "RegistryKey.hpp"

#include <format>
#include <optional>

static std::optional<DisplayConfig> displayconfig_singleton;

const wchar_t DisplayConfig::displayconfig_hkcu_path[] = L"SOFTWARE\\HDRTray";

DisplayConfig& DisplayConfig::instance()
{
    if (!displayconfig_singleton)
        displayconfig_singleton.emplace(DisplayConfig{});
    return *displayconfig_singleton;
}

static std::wstring DisplayValueName(std::wstring_view display_stable_id)
{
    return std::format(L"Display.{}", display_stable_id);
}

std::expected<bool, LSTATUS> DisplayConfig::GetEnabledFlag(std::wstring_view display_stable_id) const
{
    RegistryKey key_displayconfig;
    auto open_result =
        key_displayconfig.Open(HKEY_CURRENT_USER, displayconfig_hkcu_path, 0, KEY_WOW64_64KEY | KEY_READ | KEY_QUERY_VALUE);
    if (open_result != ERROR_SUCCESS)
        return std::unexpected(open_result);
    auto value_name = DisplayValueName(display_stable_id);

    DWORD value, value_size = sizeof(value);
    auto get_value_res =
        key_displayconfig.GetValue(nullptr, value_name.c_str(), RRF_RT_REG_DWORD, nullptr, &value, &value_size);
    if (get_value_res != ERROR_SUCCESS)
        return std::unexpected(get_value_res);

    return value != 0;
}

std::expected<void, LSTATUS> DisplayConfig::SetEnabledFlag(std::wstring_view display_stable_id, bool flag)
{
    RegistryKey key_displayconfig;
    auto create_result = key_displayconfig.Create(HKEY_CURRENT_USER, displayconfig_hkcu_path, 0,
                                                  KEY_WOW64_64KEY | KEY_READ | KEY_WRITE | KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr);
    if (create_result != ERROR_SUCCESS)
        return std::unexpected(create_result);
    auto value_name = DisplayValueName(display_stable_id);

    DWORD value = flag ? 1 : 0;
    key_displayconfig.SetValueEx(value_name.c_str(), REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value));

    return {};
}
