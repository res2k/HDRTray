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
#include "HDR.h"

#include <iostream>

class SubcmdStatus
{
    static std::string mode;

    static void print_status_short()
    {
        auto status = hdr::GetWindowsHDRStatus();
        switch(status)
        {
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

    static void print_status_long()
    {
        // TODO: print status for each display
    }

    static void run(CLI::App& app)
    {
        if (mode.empty() || stricmp(mode.c_str(), "short") == 0)
        {
            print_status_short();
        }
        else if (stricmp(mode.c_str(), "long") == 0)
        {
            print_status_short();
            print_status_long();
        }
        else if (stricmp(mode.c_str(), "exitcode") == 0)
        {
            // TODO: return exit code based on HDR status
        }
        // Validation should've caught other cases...
    }

public:
    static void add(CLI::App& app)
    {
        auto subcmd_status = app.add_subcommand("status", "Print current HDR status");
        auto mode_option = subcmd_status->add_option("-m,--mode", mode, "How to report status mode");
        mode_option->type_name("MODE");
        mode_option->check(CLI::IsMember({"short", "long", "exitcode"}, CLI::ignore_case, CLI::ignore_underscore));
        subcmd_status->parse_complete_callback([&]() { run(*subcmd_status); });
    }
};

std::string SubcmdStatus::mode;

class SubcmdEnable
{
    static void run(CLI::App& /*app*/)
    {
        hdr::SetWindowsHDRStatus(true);
        // TODO: return error/success state
    }

public:
    static void add(CLI::App& app)
    {
        auto subcmd_enable = app.add_subcommand("on", "Turn HDR on");
        subcmd_enable->parse_complete_callback([&]() { run(*subcmd_enable); });
    }

};

class SubcmdDisable
{
    static void run(CLI::App& /*app*/)
    {
        hdr::SetWindowsHDRStatus(false);
        // TODO: return error/success state
    }

public:
    static void add(CLI::App& app)
    {
        auto subcmd_disable = app.add_subcommand("off", "Turn HDR off");
        subcmd_disable->parse_complete_callback([&]() { run(*subcmd_disable); });
    }

};

int wmain(int argc, const wchar_t* const argv[])
{
    CLI::App app{"HDRCmd - enable/disable \"Use HDR\" from command line"};
    app.allow_windows_style_options();
    app.ignore_case();
    app.require_subcommand(1);

    SubcmdStatus::add(app);
    SubcmdEnable::add(app);
    SubcmdDisable::add(app);

    CLI11_PARSE(app, argc, argv);
    return 0;
}
