/*

  Auto Tuning FSR Trigger
  Author: Wing Tang Wong
  GitHub: https://github.com/WingTangWong/AutoTuning-FSR-Trigger
 
  Wrote the code testing on an Arduino Mega/Uno set, but the intended destination is to 
  run it from an ATTINY85. 
  
  Inputs: ADC
  Output: Digital IO High/Low

*/

void setup()
{
  configurePins();
  performSettle();
};

void loop()
{
  processInput();
  processSettle();
};


// The board that you are using...
//  BOARD == 0 "Arduino Uno"
//  BOARD == 1 "Arduino Mega"
//  BOARD == 2 "ATMEGA328P"
//  BOARD == 3 "ATTINY85"
int BOARD = 0;

// By default, the output pin is set to LOW. On trigger, set to HIGH.
// If you want this to be reversed, set this to false.
bool outputSignalHIGH = true;

// Continous or one-shot calibration?
bool oneShotSettle = true;
// If oneShotSettle is false, then settleTIMEOUT determines how long between each auto-settle session
long settleTIMEOUT = 250;
long sinceLastSettle=millis();

// How many sensors are you going to hookup?
// If you are hooking up three sensors to one input pin, then enter 1 here. Otherwise, 
// enter the number that are getting hooked up.
int sensorsToUse = 3;


// Timeout for various events
long TIMEOUT = 1500;


// How much of a wiggle room for sensor readings
long noiseLEVEL = 15;
// How much the pressure needs to increase to trigger an output signal change
long threshold = 25;

// Initialize starting values as globals
long ambient=0;
long reading=0;
long newambient=0;
long delta=0;
long range=0;
long sensorValue=0;
int signalON=HIGH;
int signalOFF=LOW;



// =======================================
// Arduino UNO board
// =======================================

#if BOARD == 0
  #define boardNAME "Arduino Uno"
  #define digitalOUT 13
  #define analogIN1 A3
  #define analogIN2 A4
  #define analogIN3 A5
#endif

#if BOARD == 1
  #define boardNAME "Arduino Mega"
  #define digitalOUT 13
  #define analogIN1 A3
  #define analogIN2 A4
  #define analogIN3 A5
#endif

#if BOARD == 2
  #define boardNAME "ATMEGA328P"
  #define digitalOUT 13
  #define analogIN1 A3
  #define analogIN2 A4
  #define analogIN3 A5
#endif

#if BOARD == 3
  #define boardNAME "ATTINY85"
  #define digitalOUT 6
  #define analogIN1 A1
  #define analogIN2 A2
  #define analogIN3 A3
#endif

// placeholder for single sensor testing.
long analogIN = analogIN1;

// performSettle()
//
// Basically, this settles the sensor readings so that they are all good.
// 

void performSettle() {
  while ( true ) {
    reading = analogRead(analogIN) + analogRead(analogIN) + analogRead(analogIN);
    reading = int(reading / 3.000);
    newambient = int( (ambient + reading) / 2.00 );
    delta = newambient - ambient;
    ambient = newambient;
    if ( delta < 0 ) {
      delta = delta * -1;
    };

    if ( delta <= noiseLEVEL ) {
      break;
    };
  };
  sinceLastSettle=millis();
};


// processSettle()
// 
// When this is called, we determine if we need to perform a settle.
//
void processSettle() {
    reading = analogRead(analogIN) + analogRead(analogIN) + analogRead(analogIN);
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
  long SIGNAL = LOW; 

  // How are we going to signal the outside world?
  if ( outputSignalHIGH ) {
    signalON = HIGH;
    signalOFF = LOW;
  } else {
    signalON = LOW;
    signalOFF = HIGH;
  };

  // Going to set this high until system re-settles.
  digitalWrite( digitalOUT, signalON );

  while( true ) {
    // Take a sample reading
    reading = analogRead(analogIN) + analogRead(analogIN) + analogRead(analogIN);
    reading = int(reading / 3.000);

    // Has the pressure returned to normal?
    if ( reading <= ( ambient + noiseLEVEL ) ) {
      // Okay, we've returned to normal!
      break;
    };
   
    // Did we timeout? If so, let's exit. 
    if ( ( millis() - startTime ) > TIMEOUT ) {
      break;
    };
   
  };

  // Drop the signal before we leave
  digitalWrite( digitalOUT, signalOFF);    
};


// Setup pins
void configurePins() {
    pinMode( digitalOUT, OUTPUT);
    pinMode( analogIN, INPUT);
    digitalWrite( analogIN, LOW);

  // Set the default state of the output pins
  if ( outputSignalHIGH ) {
    digitalWrite( digitalOUT, LOW);
  } else {
    digitalWrite( digitalOUT, HIGH);
  };
}


void processInput() {
  // Take an averaged reading.
  reading = analogRead(analogIN) + analogRead(analogIN) + analogRead(analogIN);
  reading = int(reading / 3.000);

  // Determine if the reading counts as a hit.
  if ( reading > ( ambient + threshold ) ) {
    performSignal();
  };
}
