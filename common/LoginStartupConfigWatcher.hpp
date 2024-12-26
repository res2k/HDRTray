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

#ifndef LOGINSTARTUPCONFIGWATCHER_HPP_
#define LOGINSTARTUPCONFIGWATCHER_HPP_

#include "RegistryWatcher.hpp"

/// Helper to watch for potential changes to the Login Startup configuration
class LoginStartupConfigWatcher : public RegistryWatcher
{
public:
    /// Create a watcher that calls the notification function when a change was detected
    LoginStartupConfigWatcher(change_notification_func notify_func);
};

#endif // LOGINSTARTUPCONFIGWATCHER_HPP_
