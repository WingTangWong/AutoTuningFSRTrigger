Fuse settings acquired from:

* http://www.engbedded.com/fusecalc

Hardware:

* USB Tiny AVR programmer ( sparkfun or Adafruit )
* ATtiny85 bare chip

Command Line options:

  avrdude -p t85 -P usb -c usbtiny -U lfuse:w:0xe2:m -U hfuse:w:0xd7:m -U efuse:w:0xff:m

Settings from fuse calculation page:

* int RC Osc 8Mhz ,Startup Time, PWRDWN/RESET; 6 CK/14CK + 64ms.....
* Unchecked "Divide Clock By 8 internally"
* Checked "Preserve EEPROM memory through the chip erase cycle"

Settings as understood from the page:

* Brown out detection disabled
* Clock output disabled.

The goal is to get the ATtiny85 to run natively at 8Mhz as opposed to the 1Mhz(8Mhz internal OSC with Clock Divider set to 8).
This allows for better response times, but also allows one to clock the ADC at 4Mhz 
