/*
    HDRTrayConfig - graphical configuration for HDRTray
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

#ifndef NOTIFYING_PROPERTY_IMPL_HPP_
#define NOTIFYING_PROPERTY_IMPL_HPP_

#include <wil/wistd_type_traits.h>
#include <wil/cppwinrt_authoring.h>

/// Provide default implementations of property methods
#define NOTIFYING_PROPERTY_IMPL(TYPE, NAME)                                                                            \
    TYPE m_##NAME = {};                                                                                                \
    auto NAME() const noexcept                                                                                         \
    {                                                                                                                  \
        return m_##NAME;                                                                                               \
    }                                                                                                                  \
    /* void Request##NAME(TYPE value); */                                                                              \
    auto& NAME(TYPE value)                                                                                             \
    {                                                                                                                  \
        Request##NAME(std::move(value));                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    void Set##NAME(TYPE value)                                                                                         \
    {                                                                                                                  \
        if (m_##NAME != value) {                                                                                       \
            m_##NAME = std::move(value);                                                                               \
            RaisePropertyChanged(L"" #NAME);                                                                           \
        }                                                                                                              \
    }

#endif // NOTIFYING_PROPERTY_IMPL_HPP_
