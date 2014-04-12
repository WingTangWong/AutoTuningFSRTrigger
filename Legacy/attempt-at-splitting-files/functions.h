#ifndef FUNCTIONS_H
  #define FUNCTIONS_H

  #include "boards.h"
  #include "globals.h"

  // do_pinsetup();            // Let's setup the pins. Using function, want to keep this file clean.
  // do_calibration();         // Let's do the initial calibration
  // do_show_state();        // Display the current values/etc. to the serial connection.
  // do_calibration();  // If calibration required, enabled, or triggered, do it.
  // do_sensor();       // Get readings. If readings sufficient, act on them.
  // do_updates();      // Get readings from other pins and update global variables




void performSettle() {
  int reading=0;
  int newambient=0;
  int newdelta=0;
  int delta=0;
  int analogPIN=0;
  int ambient=0;

  // Let's loop through the sensors and run the calibrations
  for ( analogIDX = 0; analogPIN < sensors ; analogPIN++ ) {
    PIN = analogPIN[analogIDX];

    // Let's prime the average first...
    for( int x=0; x<=10; x++) {
      reading = analogRead(PIN);
      ambient = int( (ambient + reading) / 2.00 );
      ambient = newambient;
    };

    // Now, let's determine the variations in the readings
    while ( true ) {
      reading = analogRead(PIN);
      newambient = int( (ambient + reading) / 2.00 );

      newdelta = newambient - ambient;
      if ( newdelta < 0 ) {
        newdelta = newdelta * -1;
      };

      // Running average of the noise delta.
      // How narrow would this be? And how 
      // does the probing process impact this?
      //
      delta = int((newdelta + delta) / 2.00);

      if ( delta <= noiseLEVEL ) {
        break;
      };
    };

  };
  sinceLastSettle=millis();
};


// processSettle()
// 
// When this is called, we determine if we need to perform a settle.
//
void processSettle() {
    reading = analogRead(analogPIN) + analogRead(analogPIN) + analogRead(analogPIN);
    reading = int(reading / 3.000);

    if ( oneShotSettle == false ) {
        if ( ( reading + noiseLEVEL ) < ambient ) {
          performSettle();
        };
        if ( ( millis() - sinceLastSettle ) > settleTIMEOUT ) {
          performSettle();
        };
    };
};



// Send an output signal from a triggering event
void performSignal() {

  int newReading=0;
  long startTime = millis();
  int SIGNAL = LOW; 

  // How are we going to signal the outside world?
  if ( normallyCLOSED ) {
    signalON = LOW;
    signalOFF = HIGH;
  } else {
    signalON = HIGH;
    signalOFF = LOW;
  };

  // Going to set this high until system re-settles.
  digitalWrite( outputPIN, signalON );

  while( true ) {
    // Take a sample reading
    reading = analogRead(analogPIN) + analogRead(analogPIN) + analogRead(analogPIN);
    reading = int(reading / 3.000);

    // Has the pressure returned to normal?
    if ( reading <= ( ambient + threshold ) ) {
      // Okay, we've returned to normal!
      break;
    };
   
    // Did we timeout? If so, let's exit. 
    if ( ( millis() - startTime ) > TIMEOUT ) {
      break;
    };
   
  };

  // Drop the signal before we leave
  digitalWrite( outputPIN, signalOFF);    
};


// Setup pins
void configurePins() {
    pinMode( outputPIN, OUTPUT);
    pinMode( analogPIN, INPUT);
    digitalWrite( analogPIN, LOW);

  // Set the default state of the output pins
  if ( normallyCLOSED ) {
    digitalWrite( outputPIN, HIGH);
  } else {
    digitalWrite( outputPIN, LOW);
  };
}


void processInput() {
  // Take an averaged reading.
  reading = analogRead(analogPIN) + analogRead(analogPIN) + analogRead(analogPIN);
  reading = int(reading / 3.000);

  // Determine if the reading counts as a hit.
  if ( reading > ( ambient + threshold ) ) {
    performSignal();
  };
}


// This was just for debugging!
void blink() {
    digitalWrite(outputPIN, HIGH);
    delay(50);
    digitalWrite(outputPIN, LOW);
    delay(50);
};

// Debug
void LIGHTON() {
      digitalWrite(outputPIN, HIGH);
};

// Debug
void LIGHTOFF() {
      digitalWrite(outputPIN, LOW);
};



void boards() {
  switch(BOARD) {
    case 0:
      //
      // Arduino UNO / ATmega328P
      //
      outputPIN  = 13; 
      analogPIN1   = A0; 
      analogPIN2   = A1;
      analogPIN3   = A2;
      ledPIN1  = 12;
      ledPIN2  = 11;
      ledPIN3  = 10;
      calibratePIN = 9;
      triggerPIN  = 8;
      sensors = 3;
      break;
    case 1:
      //
       // Arduino MEGA
      //
      outputPIN  = 13; 
      analogPIN1   = A0; 
      analogPIN2   = A1;
      analogPIN3   = A2;
      ledPIN1  = 12;
      ledPIN2  = 11;
      ledPIN3  = 10;
      calibratePIN = 9;
      triggerPIN  = 8;
      sensors = 3;
      break;
    case 2:
      //
      // ATTINY85(bare) and AdaFruit Trinket Board
      //
      outputPIN  = 0;  // PB0 / DIO 0
      analogPIN1   = A1; // PB2 / DIO 2 / A1
      analogPIN2   = A1; 
      analogPIN3   = A1;
      ledPIN1  = 1;  // PB1 / DIO 1 - Trinket has onboard LED on this 
      ledPIN2  = 1;
      ledPIN3  = 1;
      calibratePIN = 3; // PB3 / DIO 3 / A3
      triggerPIN  = 4; // PB4 / DIO 4 / A2
      sensors = 1;
      break;
    case 3:
      // 
      // ATTINY84 - 14 pins!
      // 
      outputPIN  = 0; 
      analogPIN1   = A1; 
      analogPIN2   = A2;
      analogPIN3   = A3;
      ledPIN1  = 1;
      ledPIN2  = 2;
      ledPIN3  = 3;
      calibratePIN = 4;
      triggerPIN  = 5;
      sensors = 3;
      break;

    default:
      //
      // default 
      //
      outputPIN  = 13; 
      analogPIN1   = A0; 
      analogPIN2   = A1;
      analogPIN3   = A2;
      ledPIN1  = 12;
      ledPIN2  = 11;
      ledPIN3  = 10;
      calibratePIN = 9;
      triggerPIN  = 8;
      sensors = 3;
  };  
};

void setup()
{
    // Load the board configurations
    boards();
    
    // Let's setup the output pin
    pinMode( outputPIN, OUTPUT);
    digitalWrite( outputPIN, signalOFF );

    // Let's setup the pin modes and default states    
    pinMode( analogPIN, INPUT);
    pinMode( analogPIN1, INPUT);
    pinMode( analogPIN2, INPUT);
    pinMode( analogPIN3, INPUT);
    
    digitalWrite( analogPIN, LOW);
    digitalWrite( analogPIN1, LOW);
    digitalWrite( analogPIN2, LOW);
    digitalWrite( analogPIN3, LOW);
    
    // We will settle the input once, at startup.
    performSettle();
   
  
};

void loop()
{
  processInput();
  // processSettle();
  //blink(); // Using this to test whether the code was running...
};

#endif
