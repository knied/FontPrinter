#!/usr/bin/python

import RPi.GPIO as GPIO
import subprocess, time

ledPin       = 18
buttonPin    = 23
holdTime     = 2     # Duration for button hold (shutdown)
tapTime      = 0.01  # Debounce time for button taps

# Called when button is briefly tapped.
def tap():
    print "tab"

# Called when button is held down.
def hold():
    print "hold"

# Use Broadcom pin numbers (not Raspberry Pi pin numbers) for GPIO
GPIO.setmode(GPIO.BCM)

# Enable LED and button (w/pull-up on latter)
GPIO.setup(ledPin, GPIO.OUT)
GPIO.setup(buttonPin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

prevTime        = time.time()
prevButtonState = GPIO.input(buttonPin)
tapEnable       = False
holdEnable      = False

while(True):
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

    
#font = "font.otf"
#for c in range(0,5):
#    subprocess.call(["python","print_glyph.py","%d" % (c),"%s" % font])
