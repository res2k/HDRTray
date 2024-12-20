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

#pragma once

#include "ToggleCard.g.h"

namespace winrt::HDRTrayConfig::implementation
{
    struct ToggleCard : ToggleCardT<ToggleCard>
    {
        ToggleCard()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        void InitializeComponent();

        hstring Text();
        void Text(const winrt::hstring& value);

        static winrt::Microsoft::UI::Xaml::DependencyProperty TextProperty() noexcept
        {
          return *TextProperty_;
        }
    
        bool IsOn() const;
        void IsOn(bool value);

        static winrt::Microsoft::UI::Xaml::DependencyProperty IsOnProperty() noexcept
        {
          return *IsOnProperty_;
        }

    private:
        bool ison_ = false;

        static std::optional<winrt::Microsoft::UI::Xaml::DependencyProperty> TextProperty_;
        static std::optional<winrt::Microsoft::UI::Xaml::DependencyProperty> IsOnProperty_;
    };
}

namespace winrt::HDRTrayConfig::factory_implementation
{
    struct ToggleCard : ToggleCardT<ToggleCard, implementation::ToggleCard>
    {
    };
}
