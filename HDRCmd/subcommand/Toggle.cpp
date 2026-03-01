/*
    HDRCmd - enable/disable "Use HDR" from command line
    Copyright (C) 2026 Frank Richter, flameshikari

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

#include "Toggle.hpp"

#include "HDR.h"

namespace subcommand {

Toggle::Toggle(CLI::App* parent) : Base("Toggle HDR on/off", "toggle", parent) { }

int Toggle::run() const
{
    auto status = hdr::GetWindowsHDRStatus();
    if (status == hdr::Status::Unsupported)
        return -1;
    auto result = hdr::SetWindowsHDRStatus(status != hdr::Status::On);
    if (!result)
        return -1;
    return *result != status ? 0 : 1;
}

CLI::App* Toggle::add(CLI::App& app)
{
    return app.add_subcommand(std::shared_ptr<Toggle>(new Toggle(&app)));
}

} // namespace subcommand
