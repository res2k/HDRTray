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

#ifndef COMMON_L10N_H_
#define COMMON_L10N_H_

#include <span>
#include <string_view>

namespace l10n {
//@{
/**
 * Load a string from a resource.
 * Unlike Win32 load string, falls back to en-US if _any_ string isn't found.
 * \param dest Buffer that receives the string.
 */
std::wstring_view LoadString(int resource_id);
void LoadString(int resource_id, std::span<wchar_t> dest);
//@}
} // namespace l10n

#endif // COMMON_L10N_H_
