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

#ifndef SUBCOMMAND_DISPLAYUSING_HPP_
#define SUBCOMMAND_DISPLAYUSING_HPP_

#include "Base.hpp"

#include "HDR.h"

namespace subcommand {

class DisplayUsing : public Base
{
protected:
    /// Default name for "optional" display specification
    static const char* default_displays_option_name;
    /// Display IDs given on command line
    std::vector<std::string> display_ids;

    /// Add an option for explicit display selection. Fills display_ids
    CLI::Option* add_displays_option(CLI::App* app, std::string option_name, std::string option_description);

    /// Return selected displays, from command line, if explicitly given, or configuration by default
    hdr::DisplayInfo_vec GetSelectedDisplays() const;

    template<typename... Arg>
    DisplayUsing(Arg&&... arg) : Base(std::forward<Arg>(arg)...)
    {
    }
};

} // namespace subcommand

#endif // SUBCOMMAND_DISPLAYUSING_HPP_
