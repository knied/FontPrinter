#!/usr/bin/python

import RPi.GPIO as GPIO
import subprocess, time
from time import sleep
from Adafruit_Thermal import *

ledPin       = 18
buttonPin    = 23
holdTime     = 5     # Duration for button hold (shutdown)
tapTime      = 0.01  # Debounce time for button taps

next_glyph_time = 10
running = False
#glyph_index = 0
max_glyph_index = 436
max_font_index = 16
#font = "font.otf"

# Called when the program should be reseted
def reset():
    global running
    print "Reset."
    running = False
    f = open("state.txt", "w")
    f.write("0 0")
    f.close()

# Called when button is briefly tapped.
def tap():
    global running
    if running == True:
        print "Pause printing."
        running = False
    else:
        print "Continue printing."
        running = True

# Called when button is held down.
def hold():
    reset()

# Called when the next glyph should be printed.
def print_next():
    f = open("state.txt", "r")
    content = f.read()
    f.close()
    content = content.split(" ")
    glyph_index = int(content[0])
    font_index = int(content[1])
    #global glyph_index
    if font_index < max_font_index:
        subprocess.call(["python","print_glyph.py","%d" % (glyph_index),"font%d.otf" % (font_index)])
        glyph_index = glyph_index + 1
        if glyph_index >= max_glyph_index:
            glyph_index = 0
            font_index = font_index + 1
        f = open("state.txt", "w")
        f.write("%d %d" % (glyph_index, font_index))
        f.close()
    else:
        print "Done."
        reset()
        
# Check if state file exists
try:
    f = open("state.txt")
    f.close()
except IOError:
    reset()

# is the state file in a valid form?
f = open("state.txt", "r")
content = f.read()
f.close()
content = content.split(" ")
if len(content) != 2:
    reset()

# Use Broadcom pin numbers (not Raspberry Pi pin numbers) for GPIO
GPIO.setmode(GPIO.BCM)

# Enable LED and button (w/pull-up on latter)
GPIO.setup(ledPin, GPIO.OUT)
GPIO.setup(buttonPin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

prevTime        = time.time()
prevButtonState = GPIO.input(buttonPin)
tapEnable       = False
holdEnable      = False
prevPrintTime   = time.time()

while(True):
    sleep(0.1)

    # Poll current button state and time
    buttonState = GPIO.input(buttonPin)
    t           = time.time()

    # Has button state changed?
    if buttonState != prevButtonState:
        prevButtonState = buttonState   # Yes, save new state/time
        prevTime        = t
    else:                             # Button state unchanged
        if (t - prevTime) >= holdTime:  # Button held more than 'holdTime'?
            # Yes it has.  Is the hold action as-yet untriggered?
            if holdEnable == True:        # Yep!
                hold()                      # Perform hold action (usu. shutdown)
                holdEnable = False          # 1 shot...don't repeat hold action
                tapEnable  = False          # Don't do tap action on release
        elif (t - prevTime) >= tapTime: # Not holdTime.  tapTime elapsed?
            # Yes.  Debounced press or release...
            if buttonState == True:       # Button released?
                if tapEnable == True:       # Ignore if prior hold()
                    tap()                     # Tap triggered (button released)
                    tapEnable  = False        # Disable tap and hold
                    holdEnable = False
            else:                         # Button pressed
                tapEnable  = True           # Enable tap and hold actions
                holdEnable = True
    
    # Should we print the next glyph?
    if running == True:
        if (t - prevPrintTime) >= next_glyph_time:
            prevPrintTime = t
            print_next()
    
#font = "font.otf"
#for c in range(0,5):
#    subprocess.call(["python","print_glyph.py","%d" % (c),"%s" % font])

