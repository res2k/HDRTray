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

#include "Select.hpp"

#include "DisplayConfig.hpp"
#include "display_name.hpp"
#include "display_status.hpp"
#include "resolve_display.hpp"

namespace subcommand {

Select::Select(CLI::App* parent) : DisplayUsing("Select displays to include in HDR toggling", "select", parent)
{
    add_subcommand("list", "Print list of displays");
    auto on_cmd = add_subcommand("on", "Include a display in HDR toggling");
    add_displays_option(on_cmd, "DISPLAY", "displays to include in HDR toggling")->required();
    auto off_cmd = add_subcommand("off", "Exclude a display from HDR toggling");
    add_displays_option(off_cmd, "DISPLAY", "displays to exclude from HDR toggling")->required();
}

int Select::toggle_displays(bool flag) const
{
    std::optional<int> result;
    auto all_displays = hdr::GetDisplays();
    for(const auto& disp_id : display_ids) {
        if (const auto* disp = resolve_display(all_displays, disp_id)) {
            auto stable_id = disp->GetStableID();
            if (!stable_id) // FIXME print an error?
                continue;
            auto enable_res = DisplayConfig::instance().SetEnabledFlag(*stable_id, flag);
            if (!enable_res) {
                if (!result)
                    *result = int(enable_res.error());
                std::println(std::cerr, "Could not store selection for display '{}': error {}", display_name(*disp), enable_res.error());
            }
        }
    }
    return result.value_or(0);
}

int Select::run() const
{
    bool has_list = got_subcommand("list");
    bool has_on = got_subcommand("on");
    bool has_off = got_subcommand("off");

    if (!has_on && !has_off && !has_list)
    {
        // No explicit command given: also include help text
        std::cout << help() << std::endl;
    }

    int result = 0;
    if (has_on || has_off) {
        result = toggle_displays(has_on);
    }

    // Always print list at the end
    display_status::print_status_long();
    return result;
}

CLI::App* Select::add(CLI::App& app)
{
    return app.add_subcommand(std::shared_ptr<Select>(new Select(&app)));
}

} // namespace subcommand
