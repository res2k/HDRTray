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

#include "DisplayConfigWatcher.hpp"

#include "DisplayConfig.hpp"
#include "RegistryKey.hpp"

// FIXME: better error handling...

DisplayConfigWatcher::DisplayConfigWatcher(change_notification_func notify_func) : notify_func(std::move(notify_func))
{
    change_event = CreateEvent(nullptr, true, false, nullptr);
    stop_watching_event = CreateEvent(nullptr, true, false, nullptr);
    watcher_thread = CreateThread(nullptr, 0, &watch_thread_proc, this, 0, nullptr);
}

DisplayConfigWatcher::~DisplayConfigWatcher()
{
    SetEvent(stop_watching_event);
    WaitForSingleObject(watcher_thread, INFINITE);
    CloseHandle(watcher_thread);
    CloseHandle(stop_watching_event);
    CloseHandle(change_event);
}

DWORD DisplayConfigWatcher::watch_thread_proc(void* parameter)
{
    return reinterpret_cast<DisplayConfigWatcher*>(parameter)->watch_thread_run();
}

DWORD DisplayConfigWatcher::watch_thread_run()
{
    RegistryKey key_displayconfig;
    // Always create the key so we don't have to deal with its absence.
    auto create_result = key_displayconfig.Create(HKEY_CURRENT_USER, DisplayConfig::displayconfig_hkcu_path, 0,
                                                  KEY_READ | KEY_QUERY_VALUE | KEY_NOTIFY, nullptr);
    if (create_result != ERROR_SUCCESS)
        return create_result;

    const HANDLE wait_events[] = { stop_watching_event, change_event };
    bool create_notification = true;
    while (true)
    {
        if (create_notification)
        {
            LSTATUS err = RegNotifyChangeKeyValue(key_displayconfig, false, REG_NOTIFY_CHANGE_LAST_SET, change_event, true);
            if (err != ERROR_SUCCESS)
                return err;
            create_notification = false;
        }

        DWORD wait_res = WaitForMultipleObjects(std::size(wait_events), wait_events, false, INFINITE);
        if (wait_res == WAIT_OBJECT_0)
            break;
        else if (wait_res == WAIT_OBJECT_0 + 1)
        {
            notify_func();
            create_notification = true;
        }
    }

    return 0;
}
