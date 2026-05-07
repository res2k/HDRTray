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

#include "Status.hpp"

#include "display_status.hpp"
#include "HDR.h"

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

void Status::print_status_short()
{
    auto status = hdr::GetWindowsHDRStatus(hdr::GetEnabledDisplays());
    std::println("HDR is {}", display_status::status_string(status));
}

int Status::run() const
{
    if (mode.empty() || stricmp(mode.c_str(), "short") == 0) {
        print_status_short();
        return 0;
    } else if (stricmp(mode.c_str(), "long") == 0) {
        print_status_short();
        std::cout << std::endl;
        display_status::print_status_long();
        return 0;
    } else if (stricmp(mode.c_str(), "exitcode") == 0) {
        auto status = hdr::GetWindowsHDRStatus(hdr::GetEnabledDisplays());
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

} // namespace subcommand
