#ifndef BOARDS_H
  #define BOARDS_H
#endif

const int ATMEGA328P = 0;
const int ARDUINO_UNO = 0;
const int ARDUINO_MEGA = 1;
const int ATTINY85 = 2;
const int ADAFRUIT_TRINKET = 2;
const int ATTINY84 = 3;

const int MAX_SENSORS = 3;

  int ledPINS[MAX_SENSORS];
  int fsrPINS[MAX_SENSORS];
  int BOARD;
  bool DEBUG;

int outputPIN;
int triggerPIN;
int calibratePIN;
int sensors;

void boards_setup() {
  if ( ( BOARD == ATTINY85 ) || ( BOARD == ADAFRUIT_TRINKET ) )
  {
    outputPIN    =  0;
    triggerPIN   =  4;
    calibratePIN =  3;
    sensors      =  1;
    ledPINS[0]           = 1;
    fsrPINS[0]           = A1;
  };


  #if ( BOARD == ATTINY84) 
    #define outputPIN      0
    #define triggerPIN     5 
    #define calibratePIN   4
    #define sensors        3
    ledPINS[0]           = 1;
    ledPINS[1]           = 2;
    ledPINS[2]           = 3;
    fsrPINS[0]           = A1;
    fsrPINS[1]           = A2;
    fsrPINS[2]           = A3;
  #endif

  #if ( BOARD == ATMEGA328P  ) || ( BOARD == ARDUINO_UNO )
    #define outputPIN      13
    #define triggerPIN     8 
    #define calibratePIN   9
    #define sensors        3
    ledPINS[0]           = 10
    ledPINS[1]           = 11
    ledPINS[2]           = 12
    fsrPINS[0]           = A0
    fsrPINS[1]           = A1
    fsrPINS[2]           = A2
  #endif

  #if ( BOARD == ARDUINO_MEGA ) 
    #define outputPIN      13
    #define triggerPIN     8 
    #define calibratePIN   9
    #define sensors        3
    ledPINS[0]           = 10
    ledPINS[1]           = 11
    ledPINS[2]           = 12
    fsrPINS[0]           = A0
    fsrPINS[1]           = A1
    fsrPINS[2]           = A2
  #endif
};
