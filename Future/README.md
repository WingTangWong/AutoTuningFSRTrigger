Future Developments
===================

The original code I was working on was supposed to support
other chipsets as well. The desire for that was to get to 
the point where I could support 3 independent FSR inputs.

However, it turns out I can achieve that with the ATtiny85.

If I continue to develop the code, it would be for the following
reasons/features:

* Individual LED status indicators for each FSR sensor
* Indicator state when the pressure is too excessive on the FSR
* Indicator state when there isn't any pressure on the FSR
* Recalibration button, which doesn't reset the MCU and thus doesn't
  let the HIGH signal go to LOW on the output. But since it only
  matters during probing, this isn't a big deal.
* Ability to communicate with the 3D printer directly and allow
  Gcode commands to interact with the MCU. This would be nice 
  for being able to set and save pressure level tunnings and values
  by hand! Since the ATTINY85 has EEPROM, this means it can be saved
  for the next run.

Targets for future developement
===============================

* ATtiny84 ( 14 pin version of the ATtiny85 )
* ATmega328P ( The heart and soul of the Arduino Uno )

Potential Integration with Printer Firmware
============================================

Some folks have expressed interest in bringing the code/logic into the 
Marlin firmware. This is definitely interesting, but the complexity 
and issues it can cause puts it onto the todo list, but somewhat low on 
priority. :(  It's a greater undertaking than coding a self-contained
solution.

