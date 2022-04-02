#!/bin/bash

# HDRTray, a notification icon for the "Use HDR" option
# Copyright (C) 2022 Frank Richter

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


# Generate the app icon file from an SVG
# This was written for use with WSL and requires inkscape, icoutils and imagemagick installed

mkdir -p tmp

app_icon_sizes=(16 32 48 64 256)
icotool_args=()
for s in ${app_icon_sizes[@]}; do
    size_png=tmp/app-$s.png
    inkscape --export-png $size_png -w $s -h $s app.svg
    if (( s <= 64 )); then
        icotool_args+=($size_png)
    else
        icotool_args+=(--raw $size_png)
    fi
done
icotool -c -o app.ico ${icotool_args[@]}
