#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Jan  3 21:18:55 2019

@author: ds


"""

import serial
from matplotlib import pyplot as plt

# in case of some failures, the script aborts, but usb objects is still there
# then script is unable to open new usb, because old one exist

try:
    # close usb device
    usb.close()
    #it could go without 'try', but python would report an error if usb doesn't exist
except:
    pass

# again, likewise
# try USB0 or USB1 (USB0 can be replaced with USB1 if device failed)
try:
    usb=serial.Serial('/dev/ttyUSB0', baudrate=9600)
except:
    usb=serial.Serial('/dev/ttyUSB1', baudrate=9600)
    pass

cam=[] # camera data
pix=[]
# send any character to trigger transmission
usb.write(b"\xff")
plt.figure()
plt.show()
# every 128 pixels are split in 2 parts
cam=usb.read(256)
pix=[]
for j in range(128):
    # merge LSB and MSB parts
    # LSB is sent first
    x=(cam[2*j+1] << 8)+cam[2*j]
    #x=cam[j]
    pix.append(x);
plt.ylim((0,4096)) # my uC has a 12-bit ADC, so 4096 is max
plt.grid()
plt.plot(pix)
plt.draw()

usb.close()
#print(cam)
