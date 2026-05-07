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
#include "AboutPage.xaml.h"
#if __has_include("AboutPage.g.cpp")
#include "AboutPage.g.cpp"
#endif

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.System.h>

#include "version.h"

#include <filesystem>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::HDRTrayConfig::implementation
{
    void AboutPage::OnDocLinkClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        auto sender_element = sender.as<FrameworkElement >();
        auto doc_tag = sender_element.Tag().as<hstring>();

        wchar_t* exe_path = nullptr;
        _get_wpgmptr(&exe_path);
        auto exe_dir = std::filesystem::path(exe_path).parent_path();

        auto doc_path = winrt::format(L"{}\\{}.html", exe_dir.native(), static_cast<std::wstring_view>(doc_tag));
        [=]() -> winrt::fire_and_forget {
            auto file = co_await Windows::Storage::StorageFile::GetFileFromPathAsync(doc_path);
            co_await Windows::System::Launcher::LaunchFileAsync(file);
        }();
    }

    winrt::hstring AboutPage::VersionString()
    {
        return winrt::format(L"Version {} ({})", L"" VERSION_SHORT, L"" VERSION_COMMIT);
    }
}
