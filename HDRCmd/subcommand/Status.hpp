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

#ifndef SUBCOMMAND_STATUS_HPP_
#define SUBCOMMAND_STATUS_HPP_

#include "Base.hpp"

namespace subcommand {
class Status : public Base
{
    static void print_status_short();
    static void print_status_long();

protected:
    std::string mode;

    Status(CLI::App* parent);

public:
    int run() const override;

    static CLI::App* add(CLI::App& app);
};

} // namespace subcommand

#endif // SUBCOMMAND_STATUS_HPP_
