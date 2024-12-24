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

#include "HDR.h"

namespace winrt::HDRTrayConfig::implementation
{
    HDRViewModel::HDRViewModel()
    {
        UpdateDisplays();
    }

    void HDRViewModel::UpdateHDRStatus()
    {
        bool hdr_enabled = hdr::GetWindowsHDRStatus(hdr::GetEnabledDisplays()) == hdr::Status::On;
        IsHDREnabled(hdr_enabled);
    }

    void HDRViewModel::RequestHDREnabled(bool flag)
    {
        auto hdr_status = hdr::SetWindowsHDRStatus(hdr::GetEnabledDisplays(), flag);
        bool hdr_enabled = hdr_status && *hdr_status == hdr::Status::On;
        IsHDREnabled(hdr_enabled);
    }

    void HDRViewModel::UpdateDisplays()
    {
        auto displays = hdr::GetDisplays();
        std::vector<IInspectable> winrt_disps;
        winrt_disps.reserve(displays.size());
        for(auto& disp : displays)
        {
            winrt_disps.emplace_back(winrt::make<HDRDisplay>(disp.GetIndex(), disp.GetID()));
        }
        Displays(winrt::multi_threaded_vector(std::move(winrt_disps)));
    }
}
