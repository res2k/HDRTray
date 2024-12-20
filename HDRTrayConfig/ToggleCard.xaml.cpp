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

#include "pch.h"
#include "ToggleCard.xaml.h"
#if __has_include("ToggleCard.g.cpp")
#include "ToggleCard.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::HDRTrayConfig::implementation
{
    void ToggleCard::InitializeComponent()
    {
        if (!TextProperty_)
        {
            TextProperty_.emplace(winrt::Microsoft::UI::Xaml::DependencyProperty::Register(
                L"Text", winrt::xaml_typename<winrt::hstring>(),
                winrt::xaml_typename<winrt::HDRTrayConfig::ToggleCard>(),
                winrt::Microsoft::UI::Xaml::PropertyMetadata { winrt::box_value(L"") }));
        }
        if (!IsOnProperty_)
        {
            IsOnProperty_.emplace(winrt::Microsoft::UI::Xaml::DependencyProperty::Register(
                L"IsOn", winrt::xaml_typename<bool>(),
                winrt::xaml_typename<winrt::HDRTrayConfig::ToggleCard>(),
                winrt::Microsoft::UI::Xaml::PropertyMetadata { winrt::box_value(false) }));
        }

        ToggleCardT::InitializeComponent();
    }

    hstring ToggleCard::Text()
    {
        return winrt::unbox_value<winrt::hstring>(GetValue(TextProperty()));
    }

    void ToggleCard::Text(const winrt::hstring& value)
    {
        SetValue(TextProperty(), winrt::box_value(value));
    }

    bool ToggleCard::IsOn() const
    {
        return winrt::unbox_value<bool>(GetValue(IsOnProperty()));
    }

    void ToggleCard::IsOn(bool value)
    {
        SetValue(IsOnProperty(), winrt::box_value(value));
    }

    std::optional<winrt::Microsoft::UI::Xaml::DependencyProperty> ToggleCard::TextProperty_;
    std::optional<winrt::Microsoft::UI::Xaml::DependencyProperty> ToggleCard::IsOnProperty_;
}
