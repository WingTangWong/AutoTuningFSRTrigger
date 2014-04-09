/*

  Auto Tuning FSR Trigger
  Author: Wing Tang Wong
  GitHub: https://github.com/WingTangWong/AutoTuning-FSR-Trigger

  Takes readings from one or more FSR(Force Sensing Resistor) sensors and produces a signal event
  when contact is made. The code will calibrate itself in either one-shot mode or in continous 
  mode.

  Features:
  - [DONE] auto calibration of 1(attiny85) or more(attiny84/atmega328) FSR units
  - [DONE] one-shot calibration at powerup. Hit reset on the board to re-calibrate.
  - [DONE] LED status indicators
  - [DONE] Output signal defaults to emulate a Normally Closed switch (NC)

  Todo:
  - daisy chaining modules with cascading input
  - one-shot, push-button, or continous re-calibration modes
  - triple FSR input on ATTINY85/TRINKET (no LED status)


  Disclaimer: As the author of this code, I make no guarantees of suitability of the code I'm publishing and take no responsibility for any
              damage that may result from the use of this code. For example, but not limited to,  if your expensive 3d printer implodes on 
              itself and my code is operating the FSR and hooked up to the z axis end-stop and it fails to detect contact and your print head
              ploughs into your print bed, causing structural failure of your printer.... you've been warned.  Seriously.

*/

// Attempt fix for a compile bug for when I have multiple calls to functions



// 
// Function definitions
// 
void do_sensor();
void do_pin_setup();
void do_calibration();
// void do_show_state();
void do_board_setup();
void do_trigger();
void do_led();
//
// Global Variables 
//

int ATMEGA328P = 0;
int ARDUINO_UNO = 0;
int ARDUINO_MEGA = 1;
int ATTINY85 = 2;
int ADAFRUIT_TRINKET = 2;
int ATTINY84 = 3;
int ADAFRUIT_TRINKETX3 = 4;
int ATTINY85X3 = 5;
bool X3 = false;
const int MAX_SENSORS = 3;
int seed = 5;
int ledPINS[MAX_SENSORS];
int fsrPINS[MAX_SENSORS];
bool fsrSTATE[MAX_SENSORS];




// The FSR read values, running average, Noise Level, and Trigger Levels
unsigned long fsrVAL[MAX_SENSORS];
unsigned long fsrAVG[MAX_SENSORS];
unsigned long fsrNL[MAX_SENSORS];
unsigned long fsrTL[MAX_SENSORS];
unsigned long fsrTALLY[MAX_SENSORS];
unsigned long fsrTOTAL[MAX_SENSORS];
unsigned long fsrNLmax[MAX_SENSORS];

int BOARD;
bool DEBUG;
bool calibration_HAS_RUN = false;
unsigned long calibration_LAST_RUN=millis();
unsigned long calibration_INTERVAL=500; // milliseconds between recalibration intervals?
int outputPIN;
int triggerPIN;
int calibratePIN;
int sensors;
bool triggerSTATE = false;

int idx;

// Uncomment the following if you are using the Trinket Board!
#define TRINKET 1


// 
// Main Setup 
//
void setup() {  
  // Select your board type!
  BOARD = ADAFRUIT_TRINKET;     // Choose your board! If you chose a TRINKET type board, make sure TRINKET is defined.
  DEBUG = false;                 // Are we doing debug?
  
  do_board_setup();
  do_pin_setup();            // Let's setup the pins. Using function, want to keep this file clean.
  do_calibration();         // Let's do the initial calibration

  // Note, debug only supported for UNO and MEGA boards. Sorry. 
  /*  Uncomment for UNO and MEGA boards
  if ( ( DEBUG == true )  && ( ( BOARD == ARDUINO_UNO ) || ( BOARD == ARDUINO_MEGA ) ) ) {
      Serial.begin(38400);  
      Serial.println(" ");
      Serial.println(" ");
      Serial.println(" ");
      Serial.print( "# " );  // Non-metric lines are prefixed with a "#"
      Serial.print( millis() );
      Serial.print( " : " );
      Serial.println(">> Board Startup");

      do_show_state();        // Display the current values/etc. to the serial connection.
  };
  */
   
};


//
// Main program loop
//
void loop() {
  do_sensor();       // Get readings. If readings sufficient, act on them.
  do_trigger();      // Update trigger and LED statuses
  // Uncomment this to support continous calibration. Disabled for now. Doesn't work in several cases.
  //do_calibration();  // If calibration required, enabled, or triggered, do it. 
  // do_show_state();        // Display the current values/etc. to the serial connection.

};


// 
// Function implementations
// 

void do_board_setup() {
  int idx;
// Trinket's IDE handling code. Since they do not define A1, A2, A3....
#ifdef  TRINKET 
  #define A0  0
  #define A1  2
  #define A2  4
  #define A3  3
#endif


  // Initialize the various fsr values
  for( idx=0; idx< MAX_SENSORS; idx++) {
    fsrVAL[idx]=0;
    fsrAVG[idx]=0;
    fsrNL[idx]=0;
    fsrTL[idx]=0;
    fsrTALLY[idx]=0;
    fsrTOTAL[idx]=0;
    fsrNLmax[idx]=0;
    fsrSTATE[idx]=false;
  };
 
  // http://learn.adafruit.com/downloads/pdf/introducing-trinket.pdf 
  if ( ( BOARD == ATTINY85 ) || ( BOARD == ADAFRUIT_TRINKET ) )
  {
    outputPIN    = 0;
    triggerPIN   =  4;
    calibratePIN =  2;
    sensors      =  1;

    ledPINS[0]           = 1;
    ledPINS[1]           = 1;
    ledPINS[2]           = 1;
    
    fsrPINS[0]           = A1;
    fsrPINS[1]           = A1;
    fsrPINS[2]           = A1;
  };
  
   if ( ( BOARD == ATTINY85X3 ) || ( BOARD == ADAFRUIT_TRINKETX3 ) )
  {
    outputPIN    = 0;
    triggerPIN   =  4;
    calibratePIN =  2;
    sensors      =  1;

    ledPINS[0]           = 2;
    ledPINS[1]           = 4;
    ledPINS[2]           = 3;
    
    fsrPINS[0]           = A1;
    fsrPINS[1]           = A2;
    fsrPINS[2]           = A3;
    
    X3 = true;
  };


  if ( BOARD == ATTINY84)  {
    outputPIN    =  0;
    triggerPIN   =  5;
    calibratePIN =  4;
    sensors      =  3;

    ledPINS[0]           = 1;
    ledPINS[1]           = 2;
    ledPINS[2]           = 3;

    fsrPINS[0]           = A1;
    fsrPINS[1]           = A2;
    fsrPINS[2]           = A3;

  };

 if ( ( BOARD == ATMEGA328P  ) || ( BOARD == ARDUINO_UNO ) ) {
     outputPIN     =  13;
     triggerPIN    =  8 ;
     calibratePIN  =  9;
     sensors       =  3;

    ledPINS[0]           = 10;
    ledPINS[1]           = 11;
    ledPINS[2]           = 12;

    fsrPINS[0]           = A0;
    fsrPINS[1]           = A1;
    fsrPINS[2]           = A2;
  };

  if ( BOARD == ARDUINO_MEGA )  {
     outputPIN     = 13;
     triggerPIN    = 8;
     calibratePIN  = 9;
     sensors       = 3;
    ledPINS[0]           = 10;
    ledPINS[1]           = 11;
    ledPINS[2]           = 12;
    fsrPINS[0]           = A0;
    fsrPINS[1]           = A1;
    fsrPINS[2]           = A2;
  };
};

void do_real_calibration() {
  int idx;
  int run;
      // For FSR input, we want a running average of   
      for ( idx=0; idx < sensors ; idx++ ) {

        // Let's make sure we have some kind of averaging data to begin with.
        // This is only done at the start
        if ( fsrTALLY[idx] < seed ) {
          for( run=0; run <=seed ; run++ ) {
            fsrVAL[idx] = analogRead( fsrPINS[idx] );
            fsrTOTAL[idx] = fsrTOTAL[idx] + fsrVAL[idx];
            fsrTALLY[idx]++;
          };
        fsrAVG[idx] = (unsigned long)( ( fsrTOTAL[idx] * 1.00 )  / ( fsrTALLY[idx] * 1.00 ) );
        };

        // Now, let's take a reading and add to the average
        fsrVAL[idx] = analogRead( fsrPINS[idx] );
        fsrAVG[idx] = (unsigned long)(((fsrAVG[idx]) + fsrVAL[idx]) / 2.00);

        if ( fsrAVG[idx] > fsrVAL[idx] ) {
          fsrNL[idx] = (unsigned long)((fsrNL[idx] + abs(fsrAVG[idx] - fsrVAL[idx])  ) / 2.00 );
        } else {
          fsrNL[idx] = (unsigned long)((fsrNL[idx] + abs(fsrVAL[idx] - fsrAVG[idx])  ) / 2.00 );
        };
        
        fsrNL[idx] = fsrNL[idx] + 8;
        
        if ( fsrNL[idx] > fsrNLmax[idx] ) {
          fsrNLmax[idx] = fsrNL[idx];
        };
        
        fsrNLmax[idx] = (unsigned long)((( fsrNLmax[idx] * 1.00 ) + ( fsrNL[idx] * 1.00 )  ) / 2.00 );
        fsrTL[idx] = fsrNLmax[idx];

      };
   calibration_LAST_RUN=millis();

  
};

void do_calibration() { 
  int idx;
  int run;

  // The first time, we prime the calibration system.
  if ( ! calibration_HAS_RUN ) {
    for( idx=0 ; idx < 5; idx++ ) {
      do_real_calibration();
      delay(5);
    };
    calibration_HAS_RUN = true;
  };
  
  if ( ( millis() - calibration_LAST_RUN ) > calibration_INTERVAL ) {
    if ( triggerSTATE == false ) {
      do_real_calibration();
    };
  };
};

void do_sensor() { 
  int idx;
  int pin;

  
  for( idx=0; idx<sensors ; idx++ ) {

    if ( fsrPINS[idx] == ledPINS[idx] ) {
      // We are using the same pins for input as we are
      // for output.
      // Prep the pins for input again
      pinMode(fsrPINS[idx], INPUT );
      digitalWrite( fsrPINS[idx], LOW );
      delay(5); // Let's wait 5ms to give time for the pull down resistor to bring
                // the value down.
    };

    // Let's take a reading. (we'll take two)
    fsrVAL[idx] = analogRead( fsrPINS[idx] );
    fsrVAL[idx] = analogRead( fsrPINS[idx] );

    if ( fsrVAL[idx] > ( fsrAVG[idx] + fsrTL[idx]  )  ) {
      fsrSTATE[idx]=true;
    };

    if ( fsrVAL[idx] < ( fsrAVG[idx] + fsrNLmax[idx] )  ) {
      fsrSTATE[idx]=false;
    };

    // If we are using the FSR pins for both LED and FSR input, let's
    // flip them back to output mode and set their light correctly.
    if (X3 == true ) {
      pinMode( ledPINS[idx] , OUTPUT );
      if ( fsrSTATE[idx] == true ) {
        digitalWrite( ledPINS[idx], HIGH );
      } else {
        digitalWrite( ledPINS[idx], LOW );
      };
    };
    
  }; 
};

void do_trigger() {
  int finalSTATE=0;
  int idx;
  for( idx=0 ; idx < sensors ; idx++ )
  {
      if ( fsrSTATE[idx] == true ) {
        pinMode( ledPINS[idx] , OUTPUT );
        digitalWrite( ledPINS[idx], HIGH);
        digitalWrite( outputPIN, LOW );
        finalSTATE++;
      } else {
        pinMode( ledPINS[idx] , OUTPUT );
        digitalWrite( ledPINS[idx], LOW);
        finalSTATE--;
      };
  };

  if ( finalSTATE < 1 ) {
    digitalWrite( outputPIN, HIGH );
  };
 
};

void do_pin_setup() { 
  int idx;
  // Output trigger pin. Default is NC / Normall Connected mode. So high!
  pinMode( outputPIN , OUTPUT );
  digitalWrite( outputPIN, HIGH );

  // The trigger pin. Pull up. To trigger, pull down to ground.
  pinMode( triggerPIN , INPUT ); 
  digitalWrite( triggerPIN, HIGH );

  // Calibration pin. Pull up. Pull down to ground to trigger calibration.
  // If left shorted to ground, will re-calibrate when calibration timeout occurs.
  pinMode( calibratePIN, INPUT );
  digitalWrite( calibratePIN, HIGH );

  for(idx=0; idx < sensors; idx++) {
    // Make sure the LED pins are set to output and are low.
    pinMode( ledPINS[idx] , OUTPUT );
    digitalWrite( ledPINS[idx], LOW );

    // If the LED and FSR pins differ, then change them to pinput.
    // Otherwise, leave them as output. This supports FSR/LED on the same pins. 
    if ( X3 != true ) {
      // Make sure the FSR pins are set to input and are pulled low.
      pinMode( fsrPINS[idx], INPUT );
      digitalWrite( fsrPINS[idx], LOW );
    }
  };

};

//
// This will display an internal status update to the serial output.
// 
/*void do_show_state() { 
  int pinIDX;
  // First, let's print the FSR status(s)
  for( pinIDX=0 ; pinIDX < sensors ; pinIDX++ ) {  
    Serial.print(millis());
    Serial.print(":");
    Serial.print("F");
    Serial.print( pinIDX);
    Serial.print(":");
    Serial.print( fsrVAL[pinIDX] );
    Serial.print(":");
    Serial.print( fsrAVG[pinIDX] );
    Serial.print(":");
    Serial.print( fsrNL[pinIDX] );    
    Serial.print(":");
    Serial.print( fsrNLmax[pinIDX] );
    Serial.print(":");
    Serial.print( fsrTL[pinIDX] );
    Serial.print(":");
    Serial.print( "triggered:" );
    if ( triggerSTATE == true ) {
      Serial.print( "ON" );
    } else {
      Serial.print("OFF");
    };
    Serial.print(" ");

  };
  Serial.println(" ");
};
*/

