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

#include "CLI/CLI.hpp"

#include "subcommand/Disable.hpp"
#include "subcommand/Enable.hpp"
#include "subcommand/Status.hpp"
#include "version.h"
#include "WinVerCheck.hpp"

#include <format>

static std::string failure_message(const CLI::App *app, const CLI::Error &e) {
    return std::format("Invalid command line arguments: {}\n\n{}", e.what(), app->help());
}

int wmain(int argc, const wchar_t* const argv[])
{
    CLI::App app{"HDRCmd " VERSION_FULL " - turn \"Use HDR\" on or off from command line"};

    // if Windows 10 < version 1803 refuse to start
    if (!IsWindows10_1803OrGreater()) {
        std::cerr << "Sorry, HDRCmd only works on Windows 10, version 1803 and above" << std::endl;
        return -2;
    }

    app.allow_windows_style_options();
    app.ignore_case();
    app.require_subcommand(1);
    app.failure_message(failure_message);

    subcommand::Status::add(app);
    subcommand::Enable::add(app);
    subcommand::Disable::add(app);

    CLI11_PARSE(app, argc, argv);
    const auto* subcmd = app.get_subcommands()[0];
    return static_cast<const subcommand::Base*>(subcmd)->run();
}
