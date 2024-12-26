/*
    HDRTrayConfig - graphical configuration for HDRTray
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

#pragma once
#include "SettingsViewModel.g.h"

#include "LoginStartupConfigWatcher.hpp"

#include <wil/wistd_type_traits.h>
#include <wil/cppwinrt_authoring.h>

namespace winrt::HDRTrayConfig::implementation
{
    struct SettingsViewModel : SettingsViewModelT<SettingsViewModel>,
                               wil::notify_property_changed_base<SettingsViewModel>
    {
        WIL_NOTIFYING_PROPERTY(bool, IsLoginStartupEnabled, false);
        
        SettingsViewModel();

        void UpdateLoginStartup();
        void RequestLoginStartupEnabled(bool flag);

    private:
        std::optional<LoginStartupConfigWatcher> config_watcher;

    };
}
