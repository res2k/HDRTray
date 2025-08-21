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

#include "pch.h"
#include "SettingsViewModel.h"
#include "SettingsViewModel.g.cpp"

#include "LoginStartupConfig.hpp"

namespace winrt::HDRTrayConfig::implementation
{
    SettingsViewModel::SettingsViewModel()
    {
        config_watcher.emplace([&, disp_queue = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread()]() {
            disp_queue.TryEnqueue([&]()
            {
                UpdateLoginStartup();
            });
        });
        UpdateLoginStartup();
    }

    void SettingsViewModel::UpdateLoginStartup()
    {
        auto login_startup = LoginStartupConfig::instance().IsEnabled();
        SetIsLoginStartupEnabled(login_startup.has_value() && *login_startup);
    }

    void SettingsViewModel::RequestIsLoginStartupEnabled(bool flag)
    {
        if (!LoginStartupConfig::instance().SetEnabled(flag))
            SetIsLoginStartupEnabled(false);
        // else: let watcher update state
    }
}
