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
#include "HDRPage.xaml.h"
#if __has_include("HDRPage.g.cpp")
#include "HDRPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::HDRTrayConfig::implementation
{
    void HDRPage::InitializeComponent()
    {
        HDRPageT::InitializeComponent();

        auto display_strings = winrt::multi_threaded_vector<winrt::Windows::Foundation::IInspectable>();

        for (const auto& s : { L"Display 1" , L"Display 2" })
        {
            display_strings.Append(winrt::box_value(s));
        }

        displaysRepeater().ItemsSource(display_strings);
    }
}
