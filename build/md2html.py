#!python3

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

# Convert Markdown files to HTML

import argparse
import marko
import urllib.parse

arguments = argparse.ArgumentParser()
arguments.add_argument('infile', metavar='MDFILE', help='input Markdown file name')
arguments.add_argument('outfile', metavar='HTMLFILE', help='output HTML file name')
arg_values = arguments.parse_args()

md = marko.Markdown()
parsed = md.parse(open(arg_values.infile, "r", encoding="utf-8").read())

# Helper: Get first child of a parsed MD document of the given type
def first_child_of_type(element, types):
    if not isinstance(types, list):
        types = [types]
    if element is not None:
        for c in element.children:
            for t in types:
                if isinstance(c, t):
                    return c
    return None

# Look for relative links, make sure they have the proper file extension
def fixup_links(element):
    for c in getattr(element, "children", []):
        if isinstance(c, marko.inline.Link):
            dest = urllib.parse.urlsplit(c.dest)
            if len(dest.netloc) == 0 and not dest.path.startswith("/"):
                new_dest = [dest.scheme, dest.netloc, dest.path + ".html", dest.query, dest.fragment]
                c.dest = urllib.parse.urlunsplit(new_dest)
        else:
            fixup_links(c)

fixup_links(parsed)

# Try to extract the title by looking for a heading
heading_block = first_child_of_type(parsed, [marko.block.SetextHeading, marko.block.Heading])
title_block = first_child_of_type(heading_block, marko.inline.RawText)
title = title_block.children if title_block else None

# Print converted MD, with some extra HTML fluff
with open(arg_values.outfile, "w") as outfile:
    print("<html>", file=outfile)
    print("<head>", file=outfile)
    if title is not None:
        print(f"<title>{title}</title>", file=outfile)
    print("<style>body { font-family: sans-serif; }</style>", file=outfile)
    print("</head>", file=outfile)
    print("<body>", file=outfile)
    outfile.write(md.render(parsed))
    print("</body>", file=outfile)
    print("</html>", file=outfile)
