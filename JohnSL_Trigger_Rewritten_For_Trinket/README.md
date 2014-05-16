This is a modified version of JohnSL's FSR handling code: 
* https://github.com/JohnSL/FSR_Endstop

Changes:
* The per-FSR status LED(s) removed since the ATtiny85 doesn't have the pins for it.
* The endstop output pin and the LED trigger pin have been changed to pins 0 and 1 respectively.
* The FSR ADC pin definitions changed from A0, A1, and A2 to simply 01, 02, 03 to match the pins on the Trinket and to be in accordance with the Trinket's numbering system.

NOTE:
* JohnSL's logic presumes that there are 10K ohm pull up resistors on each of the FSR inputs.
* The FSR pressure is supposed to bring down the value of the ADC reading.
* So if you have wired up your circuit for one of my other FSR designs, where the resistor is pulling down and the FSR is pulling up, you'll need to reverse the wiring.
* Or I can reverse the logic. :p
