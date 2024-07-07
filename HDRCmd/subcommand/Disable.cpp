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

#include "Disable.hpp"

#include "HDR.h"

namespace subcommand {

Disable::Disable(CLI::App* parent) : Base("Turn HDR off", "off", parent) { }

int Disable::run() const
{
    auto result = hdr::SetWindowsHDRStatus(false);
    if (!result)
        return -1;
    return *result == hdr::Status::Off ? 0 : 1;
}

CLI::App* Disable::add(CLI::App& app)
{
    return app.add_subcommand(std::shared_ptr<Disable>(new Disable(&app)));
}

} // namespace subcommand
