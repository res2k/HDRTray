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

#include "RegistryWatcher.hpp"

#include "RegistryKey.hpp"

// FIXME: better error handling...

RegistryWatcher::RegistryWatcher(change_notification_func notify_func, const wchar_t* reg_path, REGSAM sam_extra)
    : notify_func(std::move(notify_func)), reg_path(reg_path), sam_extra(sam_extra)
{
    change_event = CreateEvent(nullptr, true, false, nullptr);
    stop_watching_event = CreateEvent(nullptr, true, false, nullptr);
    watcher_thread = CreateThread(nullptr, 0, &watch_thread_proc, this, 0, nullptr);
}

RegistryWatcher::~RegistryWatcher()
{
    SetEvent(stop_watching_event);
    WaitForSingleObject(watcher_thread, INFINITE);
    CloseHandle(watcher_thread);
    CloseHandle(stop_watching_event);
    CloseHandle(change_event);
}

DWORD RegistryWatcher::watch_thread_proc(void* parameter)
{
    return reinterpret_cast<RegistryWatcher*>(parameter)->watch_thread_run();
}

DWORD RegistryWatcher::watch_thread_run()
{
    RegistryKey key_watched;
    // Always create the key so we don't have to deal with its absence.
    auto create_result = key_watched.Create(HKEY_CURRENT_USER, reg_path, 0,
                                            sam_extra | KEY_READ | KEY_QUERY_VALUE | KEY_NOTIFY, nullptr);
    if (create_result != ERROR_SUCCESS)
        return create_result;

    const HANDLE wait_events[] = { stop_watching_event, change_event };
    bool create_notification = true;
    while (true)
    {
        if (create_notification)
        {
            LSTATUS err = RegNotifyChangeKeyValue(key_watched, false, REG_NOTIFY_CHANGE_LAST_SET, change_event, true);
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
