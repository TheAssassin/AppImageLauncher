#! /bin/bash

set -e

if ! (which inkscape &>/dev/null); then
    echo "Error: this tool requires inkscape" >&2
    exit 1
fi

cd $(dirname "$0")

# create directory structure
mkdir -p hicolor/scalable/apps
inkscape --export-plain-svg=hicolor/scalable/apps/AppImageLauncher.svg AppImageLauncher_inkscape.svg

for res in 16 32 64 128 160 192 256 384 512; do
    fullres="${res}x${res}"
    echo "Generating PNG for resolution $fullres"

    mkdir -p hicolor/"$fullres"/apps

    inkscape -w "$res" -h "$res" -e hicolor/"$fullres"/apps/AppImageLauncher.png AppImageLauncher_inkscape.svg
#    rsvg-convert -w "$res" -h "$res" AppImageLauncher_inkscape.svg > hicolor/"$fullres"/apps/AppImageLauncher.png
done
