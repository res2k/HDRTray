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

#include "Enable.hpp"

#include "HDR.h"

namespace subcommand {

Enable::Enable(CLI::App* parent) : DisplayUsing("Turn HDR on", "on", parent) 
{
    add_displays_option(this, default_displays_option_name, "Displays to consider");
}

int Enable::run() const
{
    auto result = hdr::SetWindowsHDRStatus(GetSelectedDisplays(), true);
    if (!result)
        return -1;
    return *result == hdr::Status::On ? 0 : 1;
}

CLI::App* Enable::add(CLI::App& app)
{
    return app.add_subcommand(std::shared_ptr<Enable>(new Enable(&app)));
}

} // namespace subcommand
