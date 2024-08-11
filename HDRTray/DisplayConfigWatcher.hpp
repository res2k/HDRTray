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

#ifndef DISPLAYCONFIGWATCHER_HPP_
#define DISPLAYCONFIGWATCHER_HPP_

#include "framework.h"

#include <functional>

/// Helper to watch for changes to the Display configuration
class DisplayConfigWatcher
{
public:
    using change_notification_func = std::function<void()>;

    /// Create a watcher that calls the notification function when a change was detected
    DisplayConfigWatcher(change_notification_func notify_func);
    ~DisplayConfigWatcher();

private:
    change_notification_func notify_func;

    HANDLE watcher_thread;
    HANDLE change_event;
    HANDLE stop_watching_event;

    static DWORD WINAPI watch_thread_proc(void* parameter);
    DWORD watch_thread_run();
};

#endif // DISPLAYCONFIGWATCHER_HPP_
