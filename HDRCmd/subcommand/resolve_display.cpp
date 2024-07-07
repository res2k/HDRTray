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

#include "resolve_display.hpp"

#include "display_name.hpp"

#include "CLI/CLI.hpp"

#include <charconv>
#include <cstdio>
#include <print>
#include <ranges>

namespace subcommand {

static std::wstring fold_string(std::wstring_view input)
{
    constexpr DWORD lcmap_flags = LCMAP_LOWERCASE;
    int size_needed = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, lcmap_flags, input.data(), input.size(), nullptr, 0, nullptr, nullptr, 0);
    if (size_needed == 0)
        return {};
    std::wstring result;
    result.resize(size_needed);
    LCMapStringEx(LOCALE_NAME_USER_DEFAULT, lcmap_flags, input.data(), input.size(), result.data(), size_needed, nullptr, nullptr, 0);
    return result;
}

using DisplayInfo_ptr_vec = std::vector<const hdr::DisplayInfo*>;

static std::string join_names(const DisplayInfo_ptr_vec& displays)
{
    auto display_names_joined = displays
        | std::views::transform([](const hdr::DisplayInfo* disp) { return display_name(*disp); })
        | std::views::join_with(std::string(", "));
    return std::string(std::from_range, display_names_joined);
}

const hdr::DisplayInfo* resolve_display(const hdr::DisplayInfo_vec& displays, std::string_view id)
{
    // 1st attempt: see if id is a simple display index
    {
        int index;
        auto conv_result = std::from_chars(id.data(), id.data() + id.size(), index);
        if (conv_result.ec == std::errc{} && index >= 0 && index < displays.size())
            return &displays[index];
    }

    auto displays_and_fold_name = std::vector<std::tuple<const hdr::DisplayInfo*, std::wstring>>(
        std::from_range,
        displays | std::views::transform([](const hdr::DisplayInfo& disp) {
            return std::make_tuple(&disp, disp.GetName());
        }) | std::views::filter([](auto disp_and_name_result) -> bool {
            return std::get<1>(disp_and_name_result).has_value();
        }) | std::views::transform([](auto disp_and_name) {
            return std::make_tuple(std::get<0>(disp_and_name), fold_string(*std::get<1>(disp_and_name)));
        }));

    auto filter_displays = [&](auto pred) -> const hdr::DisplayInfo* {
        std::vector<const hdr::DisplayInfo*> candidates;
        for(const auto& disp : displays_and_fold_name) {
            if (pred(std::get<1>(disp)))
                candidates.push_back(std::get<0>(disp));
        }
        if (candidates.size() > 1) {
            std::println(stderr, "Display '{}' is ambiguous, could be any of: {}", id, join_names(candidates));
        } else if (candidates.size() == 1)
            return candidates[0];
        return nullptr;
    };

    auto fold_id = fold_string(CLI::widen(id));
    // 2nd attempt: look for case-insensitive name match
    {
        auto exact_match = filter_displays([&](const std::wstring& fold_name) { return fold_name == fold_id; });
        if (exact_match)
            return exact_match;
    }

    // 3rd attempt: look for case-insensitive substring match
    {
        auto substr_match = filter_displays(
            [&](const std::wstring& fold_name) { return fold_name.find(fold_id) != std::wstring::npos; });
        if (substr_match)
            return substr_match;
    }

    std::println(std::cerr, "Couldn't find a display fitting '{}'", id);
    return nullptr;
}

} // namespace subcommand
