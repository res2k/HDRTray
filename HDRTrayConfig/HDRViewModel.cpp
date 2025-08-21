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
#include "HDRViewModel.h"
#include "HDRViewModel.g.cpp"

#include "HDRDisplay.h"

namespace winrt::HDRTrayConfig::implementation
{
    HDRViewModel::HDRViewModel()
    {
        config_watcher.emplace([&, disp_queue = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread()]() {
            disp_queue.TryEnqueue([&]()
            {
                UpdateDisplays();
            });
        });
        UpdateDisplays();
    }

    bool HDRViewModel::UpdateHDRStatus()
    {
        auto hdr_status = hdr::GetWindowsHDRStatus(hdr::GetEnabledDisplays());
        IsHDRAvailable(hdr_status != hdr::Status::Unsupported);
        SetIsHDREnabled(hdr_status == hdr::Status::On);
        bool status_changed = hdr_status != last_hdr_status;
        last_hdr_status = hdr_status;
        return status_changed;
    }

    void HDRViewModel::RequestIsHDREnabled(bool flag)
    {
        auto hdr_status = hdr::SetWindowsHDRStatus(hdr::GetEnabledDisplays(), flag);
        bool hdr_available = hdr_status && *hdr_status != hdr::Status::Unsupported;
        bool hdr_enabled = hdr_status && *hdr_status == hdr::Status::On;
        IsHDRAvailable(hdr_available);
        SetIsHDREnabled(hdr_enabled);
    }

    void HDRViewModel::UpdateDisplays()
    {
        display_map new_displays;

        auto displays = hdr::GetDisplays();
        std::vector<IInspectable> winrt_disps;
        winrt_disps.reserve(displays.size());
        for(auto& disp : displays)
        {
            auto display_key = std::make_pair(disp.GetIndex(), disp.GetID());
            auto display_it = displays_map.find(display_key);
            HDRTrayConfig::HDRDisplay disp_obj{ nullptr };
            if (display_it != displays_map.end())
            {
                disp_obj = display_it->second;
                disp_obj.UpdateSelected();
            }
            else
                disp_obj = winrt::make<HDRDisplay>(disp.GetIndex(), disp.GetID());
            winrt_disps.emplace_back(disp_obj);
            new_displays.emplace(display_key, disp_obj);
        }
        Displays(winrt::multi_threaded_vector(std::move(winrt_disps)));

        displays_map = new_displays;

        UpdateHDRStatus();
    }
}
