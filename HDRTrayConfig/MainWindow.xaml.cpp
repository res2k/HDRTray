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
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include "App.xaml.h"
#include "Resource.h"

#include <commctrl.h>
#include <Microsoft.UI.Xaml.Window.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>

#include <format>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Windows::UI::Xaml::Interop;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::HDRTrayConfig::implementation
{
    void MainWindow::InitializeComponent()
    {
        MainWindowT::InitializeComponent();

        auto hInstance = GetModuleHandle(nullptr);
        auto icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
        AppWindow().SetIcon(Microsoft::UI::GetIconIdFromIcon(icon));

        auto windowNative = this->m_inner.as<::IWindowNative>();
        HWND hWnd = 0;
        windowNative->get_WindowHandle(&hWnd);

        SetWindowSubclass(hWnd, &SubclassProc, 0, reinterpret_cast<DWORD_PTR>(this));

        viewModel = Application::Current().as<App>()->ViewModel();
    }

    void MainWindow::NavigationView_SelectionChanged(IInspectable const&, Controls::NavigationViewSelectionChangedEventArgs const& args)
    {
        auto selectedItem = args.SelectedItem().as<Controls::NavigationViewItem>();
        auto selectedItemTag = selectedItem.Tag().as<hstring>();
        auto pageName = std::format(L"HDRTrayConfig.{}", selectedItemTag);
        auto pageType = TypeName{ winrt::hstring(pageName), TypeKind::Metadata};
        contentFrame().Navigate(pageType);
    }

    void MainWindow::NavigationView_Loaded(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        contentFrame().Navigate(winrt::xaml_typename<HDRPage>());
    }

    LRESULT MainWindow::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
        auto* This = reinterpret_cast<MainWindow*>(dwRefData);
        if (uMsg == WM_DISPLAYCHANGE)
            This->viewModel.UpdateHDRStatus();

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
}


