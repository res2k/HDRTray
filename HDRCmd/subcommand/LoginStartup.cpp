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

#include "LoginStartup.hpp"

#include "LoginStartupConfig.hpp"

#include <array>
#include <format>
#include <print>

namespace subcommand {

LoginStartup::LoginStartup(CLI::App* parent) : Base("Change 'Start when logging in'", "startup", parent)
{
    add_subcommand("status", "Print current status of option");
    add_subcommand("on", "Turn 'Start when logging in' on");
    add_subcommand("off", "Turn 'Start when logging in' off");
}

int LoginStartup::print_status(bool active)
{
    auto status_result = LoginStartupConfig::instance().IsEnabled();
    if (!status_result) {
        std::println("Could not get login startup state, error {}", status_result.error());
        return status_result.error();
    }

    auto status_str = *status_result ? "ON" : "OFF";
    if (active)
        std::println("'Start when logging in' is now {}", status_str);
    else
        std::println("'Start when logging in' is {}", status_str);
    return 0;
}

int LoginStartup::run() const
{
    bool has_status = got_subcommand("status");
    bool has_on = got_subcommand("on");
    bool has_off = got_subcommand("off");

    if (has_on || has_off) {
        auto enable_result = LoginStartupConfig::instance().SetEnabled(has_on);
        if (!enable_result) {
            std::println("Could not change login startup state, error {}", enable_result.error());
            return enable_result.error();
        }
    }

    if (!has_status && !has_on && !has_off) {
        // No explicit command given: also include help text
        std::cout << help() << std::endl;
    }
    return print_status(has_on || has_off);
}

CLI::App* LoginStartup::add(CLI::App& app)
{
    return app.add_subcommand(std::shared_ptr<LoginStartup>(new LoginStartup(&app)));
}

} // namespace subcommand
