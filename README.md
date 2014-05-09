AutoTuning FSR Trigger
======================

Arduino code to facilitate auto-tuning of a force sensing resistor for triggering end stops and auto probes for 3d printer auto-levelling.


ATTINY85_TRINKET_AUTOFSR  aka Version 1.0
==========================================
* ATTINY85 and AdaFruit Trinket version of the code.
* Supports standalone 1Mhz internal OSC ATTINY85
* Supports the AdaFruit Trinket board in 8Mhz or 16Mhz mode(really doesn't matter)
* Takes reading from 3 FSR sensors
* Auto calibrates against pressure already on FSR at boot. Hit reset to force a recalibration.

AutoFSRStandard
================
* Slimmed down version of the ATTINY85_TRINKET_AUTOFSR code
* Does not try to take into account the other board types.
* 3 x FSR inputs from ADC1, ADC2, and ADC3
* output on pins 0 and 1
 

AutoFSRFast
============
* Further slimmed down version of the code base
* Single FSR input via ADC1
* output pins on 0 and 1
* Tight loop
 

FSR Module In Action
====================
* http://youtu.be/XGlE2bxdP1I
* http://youtu.be/ziHav-B4uYc


Description & Background
========================

I wrote this code to solve a specific problem: not wanting to turn dials on a potentiometer to adjust for different weights and pressures
on the FRS sensor when placed under a print plate. My goal was to have a nearly drop in solution for a Z-axis Min End stop.

After some discussion on the DeltaBot Google Group, I decided to write some code and test my idea out on an Arduino Uno board.

However, that seemed like overkill, so I retooled the code to work on an ATtiny85 MCU. Apparently, there are several boards out there 
that uses that MCU as the processor on their tiny boards. Ie, the AdaFruit Trinket. Too cool.

While writing the code and testing, someone asked me whether I could have independent inputs for each of the three FSR. I started tinkering
with the code and the result is this code.



What Are FSR(Force Sensing Resistors)?
======================================

FSR(s) are resistors that have conductive material sandwiched between two layers. When pressure is applied, the conductive bits come into contact in degrees relative to the amount of pressure exerted. The result is a resistor that starts with a high level of resistance and as presure is applied, the resistance drops.


How Are FSR(s) Used In 3D Printers?
===================================

In an attempt to make printing onto potentially unlevel beds easier, folks devised a way to probe the bed and based on those measurements,
determine how to compensate for any deviations of an unlevel bed. Mechanical switches and other solutions are in use, but FSR(s) allow the
entire bed to be a sensor, and thus negate the need for a probe on the effector, lowering its weight and reducing cables running to the
print head.


Credits/References/Shout Outs
=============================

* Thanks to the DeltaBot Google Group for introducing me to FSR(s)!
 * https://groups.google.com/forum/#!forum/deltabot
* Thanks to Rich for the awesome mod to the Marlin firmware to support the auto-probing.
 * https://github.com/RichCattell/Marlin

Getting Arduino To Work with ATTINY chips
=========================================

You can use the Arduino/Trinket IDE that AdaFruit has published. The code will work with that IDE setup.

Alternatively, you can use the Arduino Tiny project to add the ATTINY85 cores to your Arduino IDE environment:

https://code.google.com/p/arduino-tiny/


Disclaimers
===========

I wrote this code for my own personal consumption. And while I have worked hard to ensure that the code is sane, bad things can happen, nonetheless.

Folks who have been following my development of this code should understand that this code, if it malfunctions, can result in damage to electronics that it has been wired up to including, but not limited to: burning out IO pins on your printer control board, causing your printer to ram the print head into the print bed, having your printer self destruct if it decides to home like crazy, etc. etc. 

In any case, you've been warned. I take no responsibility for any potential or any actual damages that may happen. Maker beware. 

