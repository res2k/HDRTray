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

#include <array>
#include <format>
#include <print>

namespace subcommand {

class StatusModeValidator : public CLI::Validator
{
    static constexpr const char* descr_string = "(s)hort,(l)ong,e(x)itcode";

    static std::string validate_func(std::string& item)
    {
        item = CLI::detail::to_lower(item);
        item = CLI::detail::remove_underscore(item);
        if (std::string("short").starts_with(item)) {
            item = "short";
            return {};
        }
        if (std::string("long").starts_with(item)) {
            item = "long";
            return {};
        }
        if (std::string("exitcode").starts_with(item) || item == "x") {
            item = "exitcode";
            return {};
        }
        return std::format("\"{}\" not in {} (abbreviation is allowed)", item, descr_string);
    }

public:
    StatusModeValidator() : Validator(descr_string, &validate_func) { }
};

Status::Status(CLI::App* parent) : Base("Print current HDR status", "status", parent)
{
    auto mode_option = add_option("-m,--mode", mode, "How to report status mode");
    mode_option->type_name("MODE");
    mode_option->transform(StatusModeValidator());
}

static std::string_view status_string(hdr::Status status)
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

void Status::print_status_short()
{
    auto status = hdr::GetWindowsHDRStatus();
    std::println("HDR is {}", status_string(status));
}

void Status::print_status_long()
{
    auto displays = hdr::GetDisplays();

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
        widths[1] = std::max(widths[1], disp.name.size());
        widths[2] = std::max(widths[2], status_string(disp.status).size());
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
        std::println("{:>{}}\t{:<{}}\t{:<{}}", i, widths[0], CLI::narrow(disp.name), widths[1],
                     status_string(disp.status), widths[2]);
    }
}

int Status::run() const
{
    if (mode.empty() || stricmp(mode.c_str(), "short") == 0) {
        print_status_short();
        return 0;
    } else if (stricmp(mode.c_str(), "long") == 0) {
        print_status_short();
        std::cout << std::endl;
        print_status_long();
        return 0;
    } else if (stricmp(mode.c_str(), "exitcode") == 0) {
        auto status = hdr::GetWindowsHDRStatus();
        switch(status)
        {
        case hdr::Status::On:
            return 0;
        case hdr::Status::Off:
            return 1;
        case hdr::Status::Unsupported:
            return 2;
        }
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
