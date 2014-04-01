/*

  Auto Tuning FSR Trigger
  Author: Wing Tang Wong
  GitHub: https://github.com/WingTangWong/AutoTuning-FSR-Trigger
 
  Wrote the code testing on an Arduino Mega/Uno set, but the intended destination is to 
  run it from an ATTINY85. 
  
  Inputs: ADC
  Output: Digital IO High/Low

*/

// The board that you are using...
//  BOARD == 0 "Arduino Uno"
//  BOARD == 1 "Arduino Mega"
//  BOARD == 2 "ATMEGA328P"
//  BOARD == 3 "ATTINY85"
int BOARD = 3;

// By default, the output pin is set to LOW. On trigger, set to HIGH.
// If you want this to be reversed, set this to false.
bool outputSignalHIGH = true;

// Continous or one-shot calibration?
// If this is set to true, then the code will ONLY calibrate the FSR once, at the time of startup.
bool oneShotSettle = true;

// If oneShotSettle is false, then settleTIMEOUT determines how long between each auto-settle session
long settleTIMEOUT = 500;
long sinceLastSettle=millis();

// How many sensors are you going to hookup?
// If you are hooking up three sensors to one input pin, then enter 1 here. Otherwise, 
// enter the number that are getting hooked up.
int sensorsToUse = 3;


// Timeout for various events
long TIMEOUT = 10000;


// Note, the level and threshold values are based on the ADC's 0-1024 values as understood from the FSR readings.
// How much of a wiggle room for sensor readings
long noiseLEVEL = 40;
// How much the pressure needs to increase to trigger an output signal change
long threshold = 40;

// Initialize starting values as globals
long ambient=0;
long reading=0;
long newambient=0;
long delta=0;
long range=0;
long sensorValue=0;
int signalON=HIGH;
int signalOFF=LOW;

bool STATE = false;
long lastToggle = 0;
long BLINKTIMEOUT = 100;

long digitalOUT = 13;
long analogIN1 = A1;
long analogIN2 = A2;
long analogIN3 = A3;
long statusLED1  = 12;
long statusLED2  = 11;
long statusLED3  = 10;
long calibrateIN = 9;
long signalHigh  = 8;

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


// This was just for debugging!
void blink() {
    digitalWrite(digitalOUT, HIGH);
    delay(50);
    digitalWrite(digitalOUT, LOW);
    delay(50);
};

// Debug
void LIGHTON() {
      digitalWrite(digitalOUT, HIGH);
};

// Debug
void LIGHTOFF() {
      digitalWrite(digitalOUT, LOW);
};
  

void boards() {
  switch(BOARD) {
    case 0:
      // Arduino UNO
      digitalOUT  = 13; 
      analogIN1   = A0; 
      analogIN2   = A1;
      analogIN3   = A2;
      statusLED1  = 12;
      statusLED2  = 11;
      statusLED3  = 10;
      calibrateIN = 9;
      signalHigh  = 8;
      break;
    case 1:
       // Arduino MEGA
      digitalOUT  = 13; 
      analogIN1   = A0; 
      analogIN2   = A1;
      analogIN3   = A2;
      statusLED1  = 12;
      statusLED2  = 11;
      statusLED3  = 10;
      calibrateIN = 9;
      signalHigh  = 8;
      break;
    case 2:
      // atmega328p 
      digitalOUT  = 13; 
      analogIN1   = A0; 
      analogIN2   = A1;
      analogIN3   = A2;
      statusLED1  = 12;
      statusLED2  = 11;
      statusLED3  = 10;
      calibrateIN = 9;
      signalHigh  = 8;
      break;
    case 3:
      // ATTINY85 - 8 pins!
      // Note, timings based on the ATtiny85 , 1Mhz internal OSC. Setting fuses at different speeds will impact the timings.
      digitalOUT  = 0; 
      analogIN1   = A3; 
      analogIN2   = A3;
      analogIN3   = A3;
      statusLED1  = 4;
      statusLED2  = 4;
      statusLED3  = 4;
      calibrateIN = 1;
      signalHigh  = 2;
      break;
    case 4:
      // ATTINY88 via Adafruit Tinker board - 8 pins with USB header
      // Note, timings based on the ATtiny85 , 1Mhz internal OSC. Setting fuses at different speeds will impact the timings.
      // Need to confirm these pinouts
      digitalOUT  = 0; 
      analogIN1   = A3; 
      analogIN2   = A3;
      analogIN3   = A3;
      statusLED1  = 4;
      statusLED2  = 4;
      statusLED3  = 4;
      calibrateIN = 1;
      signalHigh  = 2;
      break;
    case 5:
      // ATTINY84 - 14 pins!
      // Note, timings based on the ATtiny85 , 1Mhz internal OSC. Setting fuses at different speeds will impact the timings.
      // Need to confirm these pinouts
      digitalOUT  = 0; 
      analogIN1   = A1; 
      analogIN2   = A2;
      analogIN3   = A3;
      statusLED1  = 1;
      statusLED2  = 2;
      statusLED3  = 3;
      calibrateIN = 4;
      signalHigh  = 5;
      break;

    default:
      // default 
      digitalOUT  = 13; 
      analogIN1   = A0; 
      analogIN2   = A1;
      analogIN3   = A2;
      statusLED1  = 12;
      statusLED2  = 11;
      statusLED3  = 10;
      calibrateIN = 9;
      signalHigh  = 8;
  };  
};

void setup()
{
    // Load the board configurations
    boards();
    
    // Let's setup the output pin
    pinMode( digitalOUT, OUTPUT);
    digitalWrite( digitalOUT, signalOFF );

    // Let's setup the pin modes and default states    
    pinMode( analogIN, INPUT);
    pinMode( analogIN1, INPUT);
    pinMode( analogIN2, INPUT);
    pinMode( analogIN3, INPUT);
    
    digitalWrite( analogIN, LOW);
    digitalWrite( analogIN1, LOW);
    digitalWrite( analogIN2, LOW);
    digitalWrite( analogIN3, LOW);
    
    // We will settle the input once, at startup.
    performSettle();
   
  
};

void loop()
{
  processInput();
  processSettle();
  //blink(); // Using this to test whether the code was running...
};

