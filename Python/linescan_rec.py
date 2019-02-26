#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Jan  3 21:18:55 2019

@author: ds
"""

import serial
from matplotlib import pyplot as plt

try:
    usb.close()
except:
    pass
try:
    usb=serial.Serial('/dev/ttyUSB0', baudrate=9600)
except:
    usb=serial.Serial('/dev/ttyUSB1', baudrate=9600)
    pass

cam=[]
pix=[]
usb.write(b"\xff")
plt.figure()
plt.show()

cam=usb.read(256)
pix=[]
for j in range(128):
    x=(cam[2*j+1] << 8)+cam[2*j]
    #x=cam[j]
    pix.append(x);
plt.ylim((0,4096)) #4096
plt.grid()
plt.plot(pix)
plt.draw()

    

#print(cam)
