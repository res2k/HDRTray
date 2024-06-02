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

#include "Subcommands.hpp"

#include "HDR.h"

namespace subcommand {
Status::Status(CLI::App* parent) : Base("Print current HDR status", "status", parent)
{
    auto mode_option = add_option("-m,--mode", mode, "How to report status mode");
    mode_option->type_name("MODE");
    mode_option->check(CLI::IsMember({ "short", "long", "exitcode" }, CLI::ignore_case, CLI::ignore_underscore));
}

void Status::print_status_short()
{
    auto status = hdr::GetWindowsHDRStatus();
    switch (status) {
    case hdr::Status::Off:
        std::cout << "HDR is off" << std::endl;
        break;
    case hdr::Status::On:
        std::cout << "HDR is on" << std::endl;
        break;
    case hdr::Status::Unsupported:
        std::cout << "HDR is unsupported" << std::endl;
        break;
    }
}

void Status::print_status_long()
{
    // TODO: print status for each display
}

int Status::run() const
{
    if (mode.empty() || stricmp(mode.c_str(), "short") == 0) {
        print_status_short();
        return 0;
    } else if (stricmp(mode.c_str(), "long") == 0) {
        print_status_short();
        print_status_long();
        return 0;
    } else if (stricmp(mode.c_str(), "exitcode") == 0) {
        // TODO: return exit code based on HDR status
    }
    // Validation should've caught other cases...
    return -1;
}

CLI::App* Status::add(CLI::App& app)
{
    return app.add_subcommand(std::shared_ptr<Status>(new Status(&app)));
}

//---------------------------------------------------------------------------

Enable::Enable(CLI::App* parent) : Base("Turn HDR on", "on", parent) { }

int Enable::run() const
{
    auto result = hdr::SetWindowsHDRStatus(true);
    if (!result)
        return -1;
    return *result == hdr::Status::On ? 0 : 1;
}

CLI::App* Enable::add(CLI::App& app)
{
    return app.add_subcommand(std::shared_ptr<Enable>(new Enable(&app)));
}

//---------------------------------------------------------------------------

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
