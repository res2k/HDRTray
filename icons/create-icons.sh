#!/bin/bash

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
