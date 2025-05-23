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
  - Create straightforward branch for ATTiny85/Trinket

  Disclaimer: As the author of this code, I make no guarantees of suitability of the code I'm publishing and take no responsibility for any
              damage that may result from the use of this code. For example, but not limited to,  if your expensive 3d printer implodes on 
              itself and my code is operating the FSR and hooked up to the z axis end-stop and it fails to detect contact and your print head
              ploughs into your print bed, causing structural failure of your printer.... you've been warned.  Seriously.

*/

//
// Function definitions
//
void do_sensor();
void do_pin_setup();
void do_calibration();
void do_board_setup();
void do_trigger();

//
// Global Variables
//
enum class BoardType : int {
  ATMEGA328P = 0,
  ARDUINO_UNO = 0, // Note: Same value as ATMEGA328P
  ARDUINO_MEGA = 1,
  ATTINY85 = 2,
  ADAFRUIT_TRINKET = 2, // Note: Same value as ATTINY85
  ATTINY84 = 3,
  ADAFRUIT_TRINKET_X3 = 4,
  ATTINY85_X3 = 5
};

// Flag to indicate if FSR input pins are shared with LED output pins (X3 mode)
bool isX3Mode = false;
const int MAX_SENSORS = 3;
const int CALIBRATION_SEED = 5;
int ledPins[MAX_SENSORS];
int fsrPins[MAX_SENSORS];
bool fsrStates[MAX_SENSORS];

// The FSR read values, running average, Noise Level, and Trigger Levels
unsigned long fsrValues[MAX_SENSORS];
unsigned long fsrAverages[MAX_SENSORS];
unsigned long fsrNoiseLevels[MAX_SENSORS];
unsigned long fsrTriggerLevels[MAX_SENSORS];
unsigned long fsrTallies[MAX_SENSORS];
unsigned long fsrTotals[MAX_SENSORS];
unsigned long fsrNoiseLevelMax[MAX_SENSORS];

BoardType currentBoard;
bool isDebugMode;
bool hasCalibrationRun = false;
unsigned long lastCalibrationTimestamp = millis();
unsigned long calibrationIntervalMs = 500; // milliseconds between recalibration intervals?
int outputPin;
int triggerPin;
int calibratePin;
int sensorCount;
bool systemTriggerState = false;

int idx;

//
// Main Setup
//
void setup() {
  // Select your board type!
  currentBoard = BoardType::ADAFRUIT_TRINKET_X3; // Choose your board! If you chose a TRINKET type board, make sure TRINKET is defined.
  isDebugMode = false; // Are we doing debug?

  do_board_setup();
  do_pin_setup(); // Let's setup the pins. Using function, want to keep this file clean.
  do_calibration(); // Let's do the initial calibration
}

//
// Main program loop
//
void loop() {
  do_sensor(); // Get readings. If readings sufficient, act on them.
  do_trigger(); // Update trigger and LED statuses
}

//
// Function implementations
//
void do_board_setup() {
  int idx;
  // Trinket's IDE handling code. Since they do not define A1, A2, A3....

  // Initialize the various fsr values
  for (idx = 0; idx < MAX_SENSORS; idx++) {
    fsrValues[idx] = 0;
    fsrAverages[idx] = 0;
    fsrNoiseLevels[idx] = 0;
    fsrTriggerLevels[idx] = 0;
    fsrTallies[idx] = 0;
    fsrTotals[idx] = 0;
    fsrNoiseLevelMax[idx] = 0;
    fsrStates[idx] = false;
  }

  // http://learn.adafruit.com/downloads/pdf/introducing-trinket.pdf
  if ((currentBoard == BoardType::ATTINY85) || (currentBoard == BoardType::ADAFRUIT_TRINKET)) {
    outputPin = 0;
    triggerPin = 4;
    calibratePin = 2;
    sensorCount = 1;
    isX3Mode = false;

    ledPins[0] = 1;
    ledPins[1] = 1;
    ledPins[2] = 1;

    fsrPins[0] = A1;
    fsrPins[1] = A1;
    fsrPins[2] = A1;
  }

  // Note, in X3 mode, the wiring is different, since we are leveraging the ADC pins. No longer taking advantage of the
  // onboard LED module on Trinket boards
  if ((currentBoard == BoardType::ATTINY85_X3) || (currentBoard == BoardType::ADAFRUIT_TRINKET_X3)) {
    outputPin = 0;
    triggerPin = 4;
    calibratePin = 2;
    sensorCount = 3;

    ledPins[0] = 2;
    ledPins[1] = 4;
    ledPins[2] = 3;

    // These are direct ADC channel numbers (ADC1, ADC2, ADC3) for ATtiny85.
    fsrPins[0] = 1;
    fsrPins[1] = 2;
    fsrPins[2] = 3;

    isX3Mode = true;
  }

  if (currentBoard == BoardType::ATTINY84) {
    outputPin = 0;
    triggerPin = 5;
    calibratePin = 4;
    sensorCount = 3;
    isX3Mode = false;

    ledPins[0] = 1;
    ledPins[1] = 2;
    ledPins[2] = 3;

    fsrPins[0] = A1;
    fsrPins[1] = A2;
    fsrPins[2] = A3;
  }

  if ((currentBoard == BoardType::ATMEGA328P) || (currentBoard == BoardType::ARDUINO_UNO)) {
    outputPin = 13;
    triggerPin = 8;
    calibratePin = 9;
    sensorCount = 3;
    isX3Mode = false;

    ledPins[0] = 10;
    ledPins[1] = 11;
    ledPins[2] = 12;

    fsrPins[0] = A0;
    fsrPins[1] = A1;
    fsrPins[2] = A2;
  }

  if (currentBoard == BoardType::ARDUINO_MEGA) {
    outputPin = 13;
    triggerPin = 8;
    calibratePin = 9;
    sensorCount = 3;
    isX3Mode = false;

    ledPins[0] = 10;
    ledPins[1] = 11;
    ledPins[2] = 12;

    fsrPins[0] = A0;
    fsrPins[1] = A1;
    fsrPins[2] = A2;
  }
}

void do_real_calibration() {
  int idx;
  int run;
  // We want all input pins to be in input mode.
  for (idx = 0; idx < sensorCount; idx++) {
    pinMode(fsrPins[idx], INPUT);
    digitalWrite(fsrPins[idx], LOW);
  }

  for (idx = 0; idx < sensorCount; idx++) {
    // Let's make sure we have some kind of averaging data to begin with.
    // This is only done at the start
    if (fsrTallies[idx] < CALIBRATION_SEED) {
      for (run = 0; run <= CALIBRATION_SEED; run++) {
        fsrValues[idx] = analogRead(fsrPins[idx]);
        fsrTotals[idx] = fsrTotals[idx] + fsrValues[idx];
        fsrTallies[idx]++;
      }
      fsrAverages[idx] = (unsigned long) ((fsrTotals[idx] * 1.00) / (fsrTallies[idx] * 1.00));
    }

    // Now, let's take a reading and add to the average
    fsrValues[idx] = analogRead(fsrPins[idx]);
    fsrAverages[idx] = (unsigned long) (((fsrAverages[idx]) + fsrValues[idx]) / 2.00);

    if (fsrAverages[idx] > fsrValues[idx]) {
      fsrNoiseLevels[idx] = (unsigned long) ((fsrNoiseLevels[idx] + abs(fsrAverages[idx] - fsrValues[idx])) / 2.00);
    } else {
      fsrNoiseLevels[idx] = (unsigned long) ((fsrNoiseLevels[idx] + abs(fsrValues[idx] - fsrAverages[idx])) / 2.00);
    }

    // Add a small fixed value to the noise floor; an empirical adjustment.
    fsrNoiseLevels[idx] = fsrNoiseLevels[idx] + 8;

    if (fsrNoiseLevels[idx] > fsrNoiseLevelMax[idx]) {
      fsrNoiseLevelMax[idx] = fsrNoiseLevels[idx];
    }

    fsrNoiseLevelMax[idx] = (unsigned long) (((fsrNoiseLevelMax[idx] * 1.00) + (fsrNoiseLevels[idx] * 1.00)) / 2.00);
    // The trigger threshold is set relative to the running average, using the max observed noise level.
    fsrTriggerLevels[idx] = fsrNoiseLevelMax[idx];
  }
  lastCalibrationTimestamp = millis();

  // Okay we done. Do we need to flip the LED pins back to output mode?
  for (idx = 0; idx < sensorCount; idx++) {
    pinMode(ledPins[idx], OUTPUT);
    digitalWrite(ledPins[idx], LOW);
  }
}

void do_calibration() {
  int idx;
  int run;

  // The first time, we prime the calibration system.
  // Perform initial calibration multiple times to allow averages to stabilize.
  if (!hasCalibrationRun) {
    for (idx = 0; idx < 5; idx++) {
      do_real_calibration();
      delay(5);
    }
    hasCalibrationRun = true;
  }

  // Check if timed recalibration is due and if the system is not currently triggered.
  if ((millis() - lastCalibrationTimestamp) > calibrationIntervalMs) {
    if (systemTriggerState == false) {
      do_real_calibration();
    }
  }
}

void do_sensor() {
  int idx;
  int pin;

  for (idx = 0; idx < sensorCount; idx++) {
    if (isX3Mode == true) {
      // We are using the same pins for input as we are
      // for output.
      // Prep the pins for input again
      // In X3 mode, FSR pins are shared with LEDs; ensure they are set to INPUT before reading.
      pinMode(fsrPins[idx], INPUT);
      digitalWrite(fsrPins[idx], LOW);
    }
  }

  for (idx = 0; idx < sensorCount; idx++) {
    // Let's take a reading. (we'll take two)
    fsrValues[idx] = analogRead(fsrPins[idx]);
    // Take two consecutive readings; the first may help settle the ADC after multiplexer changes.
    fsrValues[idx] = analogRead(fsrPins[idx]);

    if (fsrValues[idx] > (fsrAverages[idx] + fsrTriggerLevels[idx])) {
      fsrStates[idx] = true;
    }

    if (fsrValues[idx] < (fsrAverages[idx] + fsrNoiseLevelMax[idx])) {
      fsrStates[idx] = false;
    }
    // Note on trigger logic:
    // fsrTriggerLevels[idx] is typically equal to fsrNoiseLevelMax[idx] after calibration.
    // Trigger ON if: fsrValue > fsrAverage + fsrNoiseLevelMax
    // Trigger OFF if: fsrValue < fsrAverage + fsrNoiseLevelMax
    // This means hysteresis is primarily managed by the responsiveness of fsrAverages
    // and fsrNoiseLevelMax to sustained changes, rather than a fixed numerical gap here.
  }

  // If we are using the FSR pins for both LED and FSR input, let's
  // flip them back to output mode and set their light correctly.
  for (idx = 0; idx < sensorCount; idx++) {
    pinMode(ledPins[idx], OUTPUT);
    if (fsrStates[idx] == true) {
      digitalWrite(ledPins[idx], HIGH);
    } else {
      digitalWrite(ledPins[idx], LOW);
    }
  }
}

void do_trigger() {
  int finalState = 0;
  int idx;
  for (idx = 0; idx < sensorCount; idx++) {
    if (fsrStates[idx] == true) {
      // Ensure LED pin is set to output before writing.
      pinMode(ledPins[idx], OUTPUT);
      digitalWrite(ledPins[idx], HIGH);
      digitalWrite(outputPin, LOW);
      finalState++;
    } else {
      // Ensure LED pin is set to output before writing.
      pinMode(ledPins[idx], OUTPUT);
      digitalWrite(ledPins[idx], LOW);
      finalState--;
    }
  }

  if (finalState < 1) {
    digitalWrite(outputPin, HIGH);
  }
}

void do_pin_setup() {
  int idx;
  // Output trigger pin. Default is NC / Normall Connected mode. So high!
  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, HIGH);

  // The trigger pin. Pull up. To trigger, pull down to ground.
  pinMode(triggerPin, INPUT);
  digitalWrite(triggerPin, HIGH);

  // Calibration pin. Pull up. Pull down to ground to trigger calibration.
  // If left shorted to ground, will re-calibrate when calibration timeout occurs.
  pinMode(calibratePin, INPUT);
  digitalWrite(calibratePin, HIGH);

  for (idx = 0; idx < sensorCount; idx++) {
    // Make sure the LED pins are set to output and are low.
    pinMode(ledPins[idx], OUTPUT);
    digitalWrite(ledPins[idx], LOW);

    // If the LED and FSR pins differ, then change them to pinput.
    // Otherwise, leave them as output. This supports FSR/LED on the same pins.
    // For non-X3 boards, FSR pins are dedicated inputs.
    if (isX3Mode != true) {
      // Make sure the FSR pins are set to input and are pulled low.
      pinMode(fsrPins[idx], INPUT);
      digitalWrite(fsrPins[idx], LOW);
    }
  }
}

