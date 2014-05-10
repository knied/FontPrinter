#!/usr/bin/python

import os
from Adafruit_Thermal import *

printer = Adafruit_Thermal("/dev/ttyAMA0", 19200, timeout=5)

os.system("./FontPrinter -s 32 -c 65 Font.ttf > ./bitmap.py")

import bitmap as bm
printer.printBitmap(bm.width, bm.height, bm.data)

printer.setDefault()

