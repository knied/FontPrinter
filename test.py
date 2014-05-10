#!/usr/bin/python

import subprocess, time

font = "font.otf"
for c in range(0,5):
    subprocess.call(["python","print_glyph.py","%d" % (c),"%s" % font])

