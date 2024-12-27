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
#include "HDRDisplay.h"
#include "HDRDisplay.g.cpp"

#include "DisplayConfig.hpp"

namespace winrt::HDRTrayConfig::implementation
{
    HDRDisplay::HDRDisplay(size_t display_idx, const hdr::DisplayID& display_id) : display(display_idx, display_id)
    {
        auto name_result = display.GetName();
        if(!name_result.has_value())
            throw_hresult(name_result.error());
        Name(hstring(name_result.value()));

        auto hdr_status = display.GetStatus();
        IsHDRAvailable(hdr_status.has_value() && hdr_status.value() != hdr::Status::Unsupported);

        UpdateSelected();
    }

    void HDRDisplay::UpdateSelected()
    {
        bool selected = false;
        if (auto stable_id = display.GetStableID(); stable_id.has_value())
        {
            selected = DisplayConfig::instance().IsEnabled(*stable_id);
        }
        SetIsSelected(selected);
    }

    void HDRDisplay::RequestIsSelected(bool flag)
    {
        if (auto stable_id = display.GetStableID(); stable_id.has_value())
        {
            auto result = DisplayConfig::instance().SetEnabledFlag(*stable_id, flag);
            if (!result)
                UpdateSelected();
            // else: let config watcher in HDRViewModel handle change
        }
    }
}
