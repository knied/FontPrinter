#!/usr/bin/python

import subprocess, sys
from Adafruit_Thermal import *

size = 54
base_line = 90
spacing = 16

printer = Adafruit_Thermal("/dev/ttyAMA0", 19200, timeout=5)
#printer.setTimes(120000, 8400)
p = subprocess.Popen(["./FontPrinter", "-c", sys.argv[1], "-s", "%d" % (size), "-b", "%d" % (base_line), "-d", "%d" % (spacing), sys.argv[2]], stdout=subprocess.PIPE)
out, err = p.communicate()
data = out.split(",")
width = int(data.pop(0))
height = int(data.pop(0))
data = map(ord,data[0].decode("hex"))
print "font: ", sys.argv[2], " c: ", sys.argv[1], " width: ", width, " height: ", height
printer.printBitmap(width, height, data)

