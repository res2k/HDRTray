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
#include "HDRViewModel.g.h"

#include "DisplayConfigWatcher.hpp"
#include "HDR.h"
#include "NotifyingPropertyImpl.hpp"

#include <wil/wistd_type_traits.h>
#include <wil/cppwinrt_authoring.h>

namespace winrt::HDRTrayConfig::implementation
{
    struct HDRViewModel : HDRViewModelT<HDRViewModel>,
                          wil::notify_property_changed_base<HDRViewModel>
    {
        WIL_NOTIFYING_PROPERTY(bool, IsHDRAvailable, false);
        NOTIFYING_PROPERTY_IMPL(bool, IsHDREnabled);
        WIL_NOTIFYING_PROPERTY(Windows::Foundation::Collections::IVector<IInspectable>,
                               Displays,
                               { });

        HDRViewModel();

        void UpdateHDRStatus();
        void RequestIsHDREnabled(bool flag);

    private:
        std::optional<DisplayConfigWatcher> config_watcher;

        using display_idx_id_pair = std::pair<size_t, hdr::DisplayID>;
        using display_map = std::map<display_idx_id_pair, HDRTrayConfig::HDRDisplay>;
        display_map displays_map;

        void UpdateDisplays();
    };
}
