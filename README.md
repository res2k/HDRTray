HDRTray
=======
Windows Notification Area icon to show and change HDR status.
Useful if you quickly want to check the current setting, maybe because it might been changed automatically by some applications, or maybe just because you're forgetful.
Also allows quick toggling.

Usage
-----
After starting the program you can see a new notification area icon, displaying
either “SDR” or “HDR” depending on whether HDR is off or on.

HDR can be toggled on or off with a left-click on the icon.

Right-clicking opens the context menu offering an option to automatically start
the program when you log in to Windows.

Command line utility
--------------------
Since version 0.5, the `HDRCmd` command line utility is included. It can be used to toggle HDR on and off from scripts and check it's status.

Syntax:

    HDRCmd [SUBCOMMAND] [SUBCOMMAND-OPTIONS]

## `on` command
Turns HDR on on all supported displays.

The affected displays are determined by the configuration made with the [`select` command](#select-command) or,
if given, a list of displays specified with the `-d` argument ([see "Display arguments"](#display-arguments)).

## `off` command
Turns HDR off on all supported displays.

The affected displays are determined by the configuration made with the [`select` command](#select-command) or,
if given, a list of displays specified with the `-d` argument ([see "Display arguments"](#display-arguments)).

## `status` command
Prints the current HDR status to the console. Has a special mode that returns an exit code depending on the status.

### `--mode` (`-m`) option
Specifies how the status should be reported. Accepts the following values:

* `short`, `s` (default): Print a single line indicating the overall HDR status.
* `long`, `l`: Print the overall HDR status and status per display.
* `exitcode`, `x`: Special mode for scripting. Exit code is 0 if HDR is on, 1 if HDR is off, and 2 if HDR is unsupported. (Other values indicate some error.)

## `select` command
This command selects the displays that are affected by HDRCmd commands or HDRTry actions.

### `list` subcommand
Prints a list of all displays, their current HDR status, and whether they're currently selected.

### `on` subcommand
Includes displays specified by the arguments ([see "Display arguments"](#display-arguments)) when enabling or disabling HDR.

### `off` subcommand
Excludes displays specified by the arguments ([see "Display arguments"](#display-arguments)) when enabling or disabling HDR.

## `startup` command
Prints or changes the 'Start when logging in' option for the notification area icon to the console.

### `status` subcommand
Prints the status of 'Start when logging in' option for the notification area icon to the console. Default if no other subcommand was specified.

### `on` subcommand
Turn the 'Start when logging in' option on. HDRTray will be automatically started beginning with the next login. Prints new status to the console.

### `off` subcommand
Turn the 'Start when logging in' option off. HDRTray will no longer be automatically started upon login. Prints new status to the console.

## Display arguments
Some commands require, or can optionally accept, a set of displays.

Displays can be specified in the following ways:

* Display number. This is simply the value in the `Display #` column when displaying the long HDR status or using `select list`. _Note: The display number is not persistent and may change if displays are connected, disconnected, or even on HDR mode changes. This makes the display number somewhat unsuitable for scripting._
* Display name or part of name. A display is used if either the full display name (the value in the `Name` column when displaying the long HDR status or using `select list`) is a case-insensitive match, or the given string is part of the display name (again, case-insensitively). Any match must be unambiguous (ie only a single display matches).

Latest Release
--------------
The latest release can be found on the [“Releases” tab of the GitHub project page](https://github.com/res2k/HDRTray/releases).

Feedback, Issues
----------------
Please use the [“Issues” tab of the GitHub project page](https://github.com/res2k/HDRTray/issues) to provide feedback or report any issues you may encounter.

License
-------
HDRTray, a notification icon for the “Use HDR” option

Copyright (C) 2022-2023 Frank Richter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

[Full text of GNU General Public License, Version 3](LICENSE.md).
