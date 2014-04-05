/*

  Auto Tuning FSR Trigger
  Author: Wing Tang Wong
  GitHub: https://github.com/WingTangWong/AutoTuning-FSR-Trigger

  Takes readings from one or more FSR(Force Sensing Resistor) sensors and produces a signal event
  when contact is made. The code will calibrate itself in either one-shot mode or in continous 
  mode.

  Features:
  - auto calibration of 1(attiny85) or more(attiny84/atmega328) FSR units
  - daisy chaining modules
  - one-shot, push-button, or continous re-calibration 
  - LED status indicators
  - Output signal defaults to emulate a Normally Closed switch (NC)

*/

#include "boards.h"    // Board definitions and related variables
#include "globals.h"    // Various global variable definitions
#include "functions.h" // Various support functions

// 
// Main Setup 
//
setup() {
  BOARD = ATTINY85;         // Choose your board!
  DEBUG = true;             // Are we doing debug?

  do_pinsetup();            // Let's setup the pins. Using function, want to keep this file clean.
  do_calibration();         // Let's do the initial calibration

  // Are we running on a board that supports Serial output? 
  if ( ( DEBUG == true )  && ( ( BOARD == ARDUINO_UNO ) || ( BOARD == ARDUINO_MEGA ) ) ) {
    Serial.begin(SERIAL_SPEED);
    Serial.print( "# " );  // Non-metric lines are prefixed with a "#"
    Serial.print( millis() );
    Serial.print( " : " );
    Serial.println(">> Board Startup");

    do_show_state();        // Display the current values/etc. to the serial connection.
  };
};


//
// Main program loop
//
loop() {
  do_calibration();  // If calibration required, enabled, or triggered, do it.
  do_sensor();       // Get readings. If readings sufficient, act on them.
  do_updates();      // Get readings from other pins and update global variables
};

