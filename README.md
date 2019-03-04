# linescan
Using linear CCD sensor array with STM32 and Python

Reading a linear CCD (Charge-Coupled Device) camera based on TSL1401CL IC with STM32F1 microprocessor.
This code can be used to capture data and tell bright surfaces from dark ones (for example: line detection in line-follower).

**Camera**
It's a simple linear camera based on CCD. When a photon hits an atom, it's possible that an electron is released. These are stored in capacitors. During a read, the IC "measures" quantity of charge in capacitors and therefore - ammount of light falling on each "pixel". To read data, CLK and SI need to be manipulated and the output is an analog signal.

**uC**
The microprocessor's tasks are:
1. Camera data reading
For each pixel: set CLK and SI pins (GPIO), read voltage using ADC and save values in memory.
This task has to be done periodically (with constant frequency) and is done in Systick interrupt.
Results depend heavily on Systick period. If it's too high, the capacitors will saturate and all the voltages will be near VCC. If too low - the values corresponding to dark/bright surfaces will differ too little to be useful or it'll be impossible to generate such short impulses on GPIO pins.
2. Data transmission
The uC waits for a command to send data via UART (currently: it's waiting for any character on UART0). Upon detection it sends stored values. It has a 12-bit ADC, therefore each value is transmitted in 2 parts. 
