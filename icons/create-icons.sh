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


# Generate the icon files from SVGs
# This was written for use with WSL and requires inkscape, icoutils and imagemagick installed

icon_from_svg()
{
    fn_base=$1

    # Icon sizes to generate.
    # Base notification icon size is 16x16, but allow for scaling up to 500%,
    # with 25% increments until 250%, and 50% increments after that
    sizes=(16 20 24 28 32 36 40 48 56 64 72 80)

    mkdir -p tmp
    args_bow=()
    args_wob=()
    for s in ${sizes[@]}; do
        # "Black on White" images
        png_bow=tmp/$fn_base-bow-$s.png
        inkscape --export-png $png_bow -w $s -h $s $fn_base.svg
        args_bow+=(--raw $png_bow)

        # Create "White on Black" images by negating HSL luminance
        png_wob=tmp/$fn_base-wob-$s.png
        convert $png_bow -colorspace HSL -channel B -negate -colorspace sRGB png32:$png_wob
        args_wob+=(--raw $png_wob)
    done

    icotool -c -o ${fn_base}-lightmode.ico ${args_bow[@]}
    icotool -c -o ${fn_base}-darkmode.ico ${args_wob[@]}
}

icon_from_svg hdr
icon_from_svg sdr
