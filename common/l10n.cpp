/*
    HDRTray, a notification icon for the "Use HDR" option
    Copyright (C) 2025 Frank Richter

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

#include "l10n.h"

#include <utility>

#include "framework.h"

namespace l10n {
static std::wstring_view LoadStringLang(int resource_id, WORD lang)
{
    HRSRC string_block = FindResourceExW(NULL, RT_STRING, MAKEINTRESOURCEW((resource_id / 16) + 1), lang);
    if (!string_block)
        return {};

    size_t res_size = SizeofResource(NULL, string_block);
    if (!res_size)
        return {};
    HGLOBAL data_handle = LoadResource(NULL, string_block);
    if (!data_handle)
        return {};
    const uint8_t* data_ptr = reinterpret_cast<uint8_t*>(LockResource(data_handle));
    if (!data_ptr)
        return {};
    const uint8_t* data_end = data_ptr + res_size;
    int string_block_idx = resource_id % 16;
    int current_idx = 0;
    while (data_ptr < data_end) {
        uint16_t string_len;
        memcpy(&string_len, data_ptr, sizeof(uint16_t));
        data_ptr += sizeof(uint16_t);
        if (current_idx == string_block_idx)
            return std::wstring_view(reinterpret_cast<const wchar_t*>(data_ptr), string_len);
        data_ptr += string_len * sizeof(wchar_t);
        ++current_idx;
    }
    return {};
}

std::wstring_view LoadString(int resource_id)
{
    auto str = LoadStringLang(resource_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    // Fall back to en-US
    if (str.empty())
        str = LoadStringLang(resource_id, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    return str;
}

void LoadString(int resource_id, std::span<wchar_t> dest)
{
    if (dest.empty())
        return;

    auto str = LoadString(resource_id);
    size_t num_copy = std::min(dest.size() - 1, str.size());
    memcpy(dest.data(), str.data(), num_copy * sizeof(wchar_t));
    dest[num_copy] = 0;
}
} // namespace l10n
