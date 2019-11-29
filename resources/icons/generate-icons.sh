#! /bin/bash

set -e

if ! (which inkscape &>/dev/null); then
    echo "Error: this tool requires inkscape" >&2
    exit 1
fi

cd $(dirname "$0")

in_file=$(readlink -f $(dirname "$0"))/appimagelauncher.inkscape.svg

# create directory structure
mkdir -p hicolor/scalable/apps
inkscape --export-plain-svg=hicolor/scalable/apps/AppImageLauncher.svg "$in_file"

for res in 16 32 64 128 160 192 256 384 512; do
    fullres="${res}x${res}"
    echo "Generating PNG for resolution $fullres"

    mkdir -p hicolor/"$fullres"/apps

    inkscape -w "$res" -h "$res" -e hicolor/"$fullres"/apps/AppImageLauncher.png "$in_file"
#    rsvg-convert -w "$res" -h "$res" "$in_file" > hicolor/"$fullres"/apps/AppImageLauncher.png
done
