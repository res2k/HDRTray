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
 * LoginStartup/"Start when logging in" configuration
 */
#ifndef LOGINSTARTUPCONFIG_HPP_
#define LOGINSTARTUPCONFIG_HPP_

#include "framework.h"

#include <expected>
#include <filesystem>

class LoginStartupConfig
{
protected:
    /// Path to executable to loginstartup
    std::filesystem::path loginstartup_exe;

    LoginStartupConfig();

public:
    /// Path of login startup entires below HKCU
    static const wchar_t loginstartup_hkcu_path[];

    /// Return reference to singleton
    static LoginStartupConfig& instance();

    /// Whether loginstartup is currently enabled
    std::expected<bool, LSTATUS> IsEnabled() const;
    /// Enable/disable loginstartup
    std::expected<void, LSTATUS> SetEnabled(bool flag);
};

#endif // LOGINSTARTUPCONFIG_HPP_
