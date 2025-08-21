/*
    HDRCmd - enable/disable "Use HDR" from command line
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

#include "DisplayUsing.hpp"

#include "resolve_display.hpp"

namespace subcommand {

const char* DisplayUsing::default_displays_option_name = "-d,--displays";

CLI::Option* DisplayUsing::add_displays_option(CLI::App* app, std::string option_name, std::string option_description)
{
    return app->add_option(option_name, display_ids, option_description)->type_name("# or NAME");
}

hdr::DisplayInfo_vec DisplayUsing::GetSelectedDisplays() const
{
    if (display_ids.empty())
        return hdr::GetEnabledDisplays();

    auto all_displays = hdr::GetDisplays();
    hdr::DisplayInfo_vec selected_displays;
    for(const auto& disp_id : display_ids) {
        if (const auto* disp = resolve_display(all_displays, disp_id)) {
            if (std::find_if(selected_displays.begin(), selected_displays.end(),
                             [disp](const hdr::DisplayInfo& already_selected) {
                                 return already_selected.GetID() == disp->GetID();
                             }) != selected_displays.end())
                continue;
            selected_displays.push_back(*disp);
        }
    }
    return selected_displays;
}

}
