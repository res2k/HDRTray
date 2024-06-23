/*
    HDRTray, a notification icon for the "Use HDR" option
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

/**\file
 * Convenience class for registry access
 */
#ifndef REGISTRYKEY_HPP_
#define REGISTRYKEY_HPP_

#include "framework.h"

class RegistryKey
{
    HKEY key = nullptr;

public:
    ~RegistryKey()
    {
        if (key)
            RegCloseKey(key);
    }

    operator HKEY() const { return key; }

    /// Wrapper around RegCreateKeyExW(), storing the created key internally
    LRESULT Create(HKEY key, LPCWSTR subkey, DWORD options, REGSAM samDesired, const LPSECURITY_ATTRIBUTES security_attributes)
    {
        return RegCreateKeyExW(key, subkey, 0, nullptr, options, samDesired, security_attributes, &this->key, nullptr);
    }
    /// Wrapper around RegQueryValueExW(), using the internally stored key
    LRESULT QueryValueEx(LPCWSTR valueName, LPDWORD type, LPBYTE data, LPDWORD data_size) const
    {
        return RegQueryValueExW(key, valueName, nullptr, type, data, data_size);
    }
    /// Wrapper around RegSetValueExW(), using the internally stored key
    LSTATUS SetValueEx(LPCWSTR valueName, DWORD type, const BYTE* data, DWORD data_size) const
    {
        return RegSetValueExW(key, valueName, 0, type, data, data_size);
    }
    /// Wrapper around RegDeleteValueW(), using the internally stored key
    LSTATUS DeleteValue(LPCWSTR valueName) const { return RegDeleteValueW(key, valueName); }
};

#endif // REGISTRYKEY_HPP_