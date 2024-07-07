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

#include "display_status.hpp"

#include "CLI/CLI.hpp"

#include <array>
#include <iostream>
#include <print>

namespace subcommand::display_status {
std::string_view status_string(hdr::Status status)
{
    switch (status) {
    case hdr::Status::Off:
        return "off";
    case hdr::Status::On:
        return "on";
    case hdr::Status::Unsupported:
        return "unsupported";
    }
    return "???";
}

void print_status_long()
{
    auto displays = hdr::GetDisplays();
    // Filter out all displays w/o name or status
    std::erase_if(displays, [](const hdr::DisplayInfo& info) { return !info.GetStatus() || !info.GetName(); });

    // Tabulate.
    // Columns: #, Display name, Status
    static constexpr size_t num_cols = 3;
    static constexpr std::string_view col_headings[num_cols] = { "Display #", "Name", "Status" };
    std::array<size_t, num_cols> widths;
    widths[0] = std::max(size_t(ceil(log10(displays.size() + 1))) + 1, col_headings[0].size());
    widths[1] = col_headings[1].size();
    widths[2] = col_headings[2].size();
    for(const auto& disp : displays)
    {
        widths[1] = std::max(widths[1], disp.GetName()->size());
        widths[2] = std::max(widths[2], status_string(*disp.GetStatus()).size());
    }

    // Print heading
    for (size_t i = 0; i < num_cols; i++)
    {
        if (i > 0)
            std::cout << "\t";
        std::print("{:<{}}", col_headings[i], widths[i]);
    }
    std::cout << std::endl;
    for (size_t i = 0; i < num_cols; i++)
    {
        if (i > 0)
            std::cout << "\t";
        std::print("{:-<{}}", "", widths[i]);
    }
    std::cout << std::endl;
    for (size_t i = 0; i < displays.size(); i++)
    {
        const auto& disp = displays[i];
        std::println("{:>{}}\t{:<{}}\t{:<{}}", i, widths[0], CLI::narrow(*disp.GetName()), widths[1],
                     status_string(*disp.GetStatus()), widths[2]);
    }
}

} // namespace subcommand::display_status
