#! /usr/bin/env python3

import sys

from xml.etree import ElementTree as ET


def copy_sources_to_translations(root: ET.Element):
    for context in root.findall("context"):
        for message in context.findall("message"):
            source = message.find("source")
            translation = message.find("translation")

            translation.text = source.text

            if "type" in translation.attrib:
                if translation.attrib["type"] in ("", "unfinished"):
                    del translation.attrib["type"]


def main():
    with open(sys.argv[1]) as f:
        et = ET.parse(f)

    root = et.getroot()

    copy_sources_to_translations(root)

    with open(sys.argv[1], "wb") as f:
        f.write(ET.tostring(root))


if __name__ == "__main__":
    main()
