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

#include "LoginStartupConfig.hpp"

#include "RegistryKey.hpp"

#include <optional>
#include <string_view>

static std::optional<LoginStartupConfig> loginstartup_singleton;

LoginStartupConfig& LoginStartupConfig::instance()
{
    if (!loginstartup_singleton)
        loginstartup_singleton.emplace(LoginStartupConfig{});
    return *loginstartup_singleton;
}

LoginStartupConfig::LoginStartupConfig()
{
    wchar_t* exe_path = nullptr;
    _get_wpgmptr(&exe_path);

    loginstartup_exe = exe_path;
    // Always use HDRTray.exe as the loginstartup path, so even HDRCmd will control the tray icon behavior
    loginstartup_exe = loginstartup_exe.parent_path() / "HDRTray.exe";
}

const wchar_t LoginStartupConfig::loginstartup_hkcu_path[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t loginstartup_registry_key[] = L"HDRTray";

std::expected<bool, LSTATUS> LoginStartupConfig::IsEnabled() const
{
    RegistryKey key_loginstartup;
    auto create_result =
        key_loginstartup.Create(HKEY_CURRENT_USER, loginstartup_hkcu_path, 0, KEY_READ | KEY_QUERY_VALUE, nullptr);
    if (create_result != ERROR_SUCCESS)
        return std::unexpected(create_result);

    DWORD value_type = 0;
    DWORD value_size = 0;
    auto query_value_res = key_loginstartup.QueryValueEx(loginstartup_registry_key, &value_type, nullptr, &value_size);
    if (query_value_res == ERROR_FILE_NOT_FOUND)
        return false;
    else if (query_value_res != ERROR_SUCCESS && query_value_res != ERROR_MORE_DATA)
        return std::unexpected(query_value_res);
    if (value_type != REG_SZ)
        return false;

    DWORD value_len = value_size / sizeof(WCHAR);
    DWORD buf_size = (value_len + 1) * sizeof(WCHAR);
    auto* buf = reinterpret_cast<WCHAR*>(_alloca(buf_size));
    query_value_res =
        key_loginstartup.QueryValueEx(loginstartup_registry_key, nullptr, reinterpret_cast<BYTE*>(buf), &buf_size);
    if (query_value_res != ERROR_SUCCESS)
        return std::unexpected(query_value_res);

    buf[value_len] = 0;
    auto loginstartup_value = std::wstring_view(buf);
    // Strip surrounding '"'
    if (loginstartup_value.size() >= 2 && *loginstartup_value.begin() == '"' && *loginstartup_value.rbegin() == '"')
        loginstartup_value = loginstartup_value.substr(1, loginstartup_value.size() - 2);
    if (loginstartup_value.empty())
        return false;

    std::error_code ec;
    bool loginstartup_enabled = std::filesystem::equivalent(loginstartup_exe, loginstartup_value, ec);
    if (ec)
        return std::unexpected(ec.value()); // Works b/c MSVC std::filesystem returns Windows error codes
    return loginstartup_enabled;
}

std::expected<void, LSTATUS> LoginStartupConfig::SetEnabled(bool flag)
{
    RegistryKey key_loginstartup;
    auto create_result = key_loginstartup.Create(HKEY_CURRENT_USER, loginstartup_hkcu_path, 0,
                                              KEY_READ | KEY_WRITE | KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr);
    if (create_result != ERROR_SUCCESS)
        return std::unexpected(create_result);

    if(flag) {
        std::wstring loginstartup_value;
        loginstartup_value.reserve(loginstartup_exe.native().size() + 2);
        loginstartup_value.push_back('"');
        loginstartup_value.append(loginstartup_exe.native());
        loginstartup_value.push_back('"');

        key_loginstartup.SetValueEx(loginstartup_registry_key, REG_SZ, reinterpret_cast<const BYTE*>(loginstartup_value.c_str()),
                                 static_cast<DWORD>((loginstartup_value.size() + 1) * sizeof(WCHAR)));
    } else {
        key_loginstartup.DeleteValue(loginstartup_registry_key);
    }

    return {};
}
