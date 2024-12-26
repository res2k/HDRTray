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

#ifndef REGISTRYWATCHER_HPP_
#define REGISTRYWATCHER_HPP_

#include "framework.h"

#include <functional>

/// Helper to watch for changes to a registry key configuration
class RegistryWatcher
{
protected:
    using change_notification_func = std::function<void()>;

    /// Create a watcher that calls the notification function when a change was detected
    RegistryWatcher(change_notification_func notify_func, const wchar_t* reg_path, REGSAM sam_extra = 0);

public:
    ~RegistryWatcher();

private:
    change_notification_func notify_func;
    const wchar_t* reg_path;
    REGSAM sam_extra;

    HANDLE watcher_thread;
    HANDLE change_event;
    HANDLE stop_watching_event;

    static DWORD WINAPI watch_thread_proc(void* parameter);
    DWORD watch_thread_run();
};

#endif // REGISTRYWATCHER_HPP_
