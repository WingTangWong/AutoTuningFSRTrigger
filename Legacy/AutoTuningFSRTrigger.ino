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
/**
 * @brief Initializes the microcontroller, board configuration, pin modes, and performs initial sensor calibration.
 * @details
 * This function is called once at power-up or reset. It performs the following key actions:
 * 1. Sets the `currentBoard` type (e.g., `BoardType::ADAFRUIT_TRINKET_X3`).
 * 2. Sets `isDebugMode` (currently to `false`).
 * 3. Calls `do_board_setup()` to configure board-specific pin assignments and initialize sensor arrays based on `currentBoard`.
 * 4. Calls `do_pin_setup()` to set the pinMode for output, trigger, calibration, LED, and FSR pins.
 * 5. Calls `do_calibration()` to perform the initial FSR calibration sequence.
 * @note Modifies global variables: `currentBoard`, `isDebugMode`. Initializes many other global sensor state variables via called functions.
 * @see do_board_setup()
 * @see do_pin_setup()
 * @see do_calibration()
 */
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
/**
 * @brief Main execution loop, called repeatedly after `setup()`.
 * @details
 * This function continuously performs the following actions:
 * 1. Calls `do_sensor()` to read FSR values, update their states (triggered/not triggered), and manage LED indicators.
 * 2. Calls `do_trigger()` to update the main system trigger output based on individual FSR states.
 * 3. Periodically, `do_calibration()` (called from within `do_sensor()` or explicitly if uncommented) may run to recalibrate sensors if conditions are met.
 * @note Relies heavily on global variables for state management.
 * @see do_sensor()
 * @see do_trigger()
 */
void loop() {
  do_sensor(); // Get readings. If readings sufficient, act on them.
  do_trigger(); // Update trigger and LED statuses
}

//
// Function implementations
//
/**
 * @brief Configures board-specific parameters like pin assignments and sensor count.
 * @details
 * This function initializes hardware-specific settings based on the globally set `currentBoard` type.
 * Key actions include:
 * 1. Initializes all FSR-related global arrays (e.g., `fsrValues`, `fsrAverages`, `fsrStates`) to zero or default states.
 * 2. Based on `currentBoard`, sets:
 *    - `outputPin`, `triggerPin`, `calibratePin`.
 *    - `sensorCount` (number of FSRs to use).
 *    - `ledPins[]` array with pin numbers for LED indicators.
 *    - `fsrPins[]` array with analog pin numbers for FSR inputs.
 *    - `isX3Mode` flag (true if pins are shared for FSR input and LED output, e.g., on ATtiny85 X3 configurations).
 * @note Modifies global variables: `fsrValues`, `fsrAverages`, `fsrNoiseLevels`, `fsrTriggerLevels`, `fsrTallies`, `fsrTotals`, `fsrNoiseLevelMax`, `fsrStates`, `outputPin`, `triggerPin`, `calibratePin`, `sensorCount`, `ledPins`, `fsrPins`, `isX3Mode`.
 * @globalvar currentBoard Determines which board configuration is applied.
 */
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

/**
 * @brief Performs the actual FSR calibration process for all sensors.
 * @details
 * This function is the core of the calibration logic. For each sensor:
 * 1. Sets the FSR input pin to `INPUT` mode.
 * 2. **Initial Seeding:** If the sensor has not been calibrated sufficiently ( `fsrTallies[idx] < CALIBRATION_SEED`), it takes `CALIBRATION_SEED + 1` readings, sums them, and calculates an initial `fsrAverages[idx]`.
 * 3. **Running Average Update:** Takes a new analog reading (`fsrValues[idx]`) and updates `fsrAverages[idx]` using a simple running average: `(previous_average + current_value) / 2`.
 * 4. **Noise Level Calculation:** Calculates the absolute difference between the current reading and the updated average. This difference is then averaged into `fsrNoiseLevels[idx]`.
 * 5. **Noise Floor Adjustment:** Adds a fixed value (8) to `fsrNoiseLevels[idx]`. This is an empirical adjustment to ensure the noise floor isn't set too low.
 * 6. **Maximum Noise Level Tracking:** Updates `fsrNoiseLevelMax[idx]` if the current `fsrNoiseLevels[idx]` is greater. `fsrNoiseLevelMax` itself is also averaged over time.
 * 7. **Trigger Threshold Setting:** Sets `fsrTriggerLevels[idx]` to the current `fsrNoiseLevelMax[idx]`. The trigger condition is typically `current_reading > average_reading + trigger_level`.
 * 8. Updates `lastCalibrationTimestamp`.
 * 9. After calibrating all sensors, sets the LED pins back to `OUTPUT` mode and turns them off.
 * @note Modifies global variables: `fsrValues`, `fsrTotals`, `fsrTallies`, `fsrAverages`, `fsrNoiseLevels`, `fsrNoiseLevelMax`, `fsrTriggerLevels`, `lastCalibrationTimestamp`.
 * @note Changes pin modes for FSR pins (to INPUT) and LED pins (to OUTPUT).
 */
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

/**
 * @brief Manages the FSR calibration process, deciding when to call `do_real_calibration()`.
 * @details
 * This function controls the calibration timing and conditions:
 * 1. **Initial Priming:** If calibration has not run before (`!hasCalibrationRun`), it calls `do_real_calibration()` multiple times (currently 5 times with a short delay) to allow the sensor averages and noise levels to stabilize. Sets `hasCalibrationRun = true` afterwards.
 * 2. **Periodic Recalibration:** If the time elapsed since `lastCalibrationTimestamp` exceeds `calibrationIntervalMs` AND the system is not currently in a triggered state (`systemTriggerState == false`), it calls `do_real_calibration()` to update calibration values.
 * @note Modifies global variables: `hasCalibrationRun` (indirectly via `do_real_calibration()`: `lastCalibrationTimestamp`).
 * @globalvar hasCalibrationRun Tracks if the initial calibration sequence has completed.
 * @globalvar lastCalibrationTimestamp Timestamp of the last successful `do_real_calibration()` run.
 * @globalvar calibrationIntervalMs Interval at which recalibration is considered.
 * @globalvar systemTriggerState Prevents recalibration if the system is actively triggered.
 * @see do_real_calibration()
 */
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

/**
 * @brief Reads FSR sensor values, updates their states, and manages LED indicators.
 * @details
 * This function is responsible for the primary sensing loop:
 * 1. **Pin Mode Setup (X3 Mode):** If `isX3Mode` is true, it iterates through FSR pins and sets them to `INPUT` mode with `digitalWrite(pin, LOW)` (disabling pull-ups, ensuring high impedance). This is necessary because pins are shared for input (FSR) and output (LED).
 * 2. **Sensor Reading:** For each sensor:
 *    - Takes two consecutive analog readings from the `fsrPins[idx]`. The second reading is used (the first may help stabilize the ADC).
 *    - Updates `fsrValues[idx]` with the reading.
 * 3. **State Update:**
 *    - If `fsrValues[idx]` is greater than `(fsrAverages[idx] + fsrTriggerLevels[idx])`, sets `fsrStates[idx] = true` (triggered).
 *    - If `fsrValues[idx]` is less than `(fsrAverages[idx] + fsrNoiseLevelMax[idx])`, sets `fsrStates[idx] = false` (not triggered).
 *      (Note: `fsrTriggerLevels` and `fsrNoiseLevelMax` are often the same after calibration, providing a basic hysteresis based on how `fsrAverages` and `fsrNoiseLevelMax` adapt).
 * 4. **LED Update:** For each sensor:
 *    - Sets the corresponding `ledPins[idx]` to `OUTPUT` mode.
 *    - If `fsrStates[idx]` is true, turns the LED HIGH; otherwise, turns it LOW.
 * @note Modifies global variables: `fsrValues`, `fsrStates`.
 * @note Changes pin modes for FSR pins (if `isX3Mode`) and LED pins.
 * @globalvar isX3Mode Affects pin mode handling for FSR inputs.
 * @globalvar fsrPins Array of FSR input pins.
 * @globalvar fsrValues Array to store current FSR readings.
 * @globalvar fsrAverages Array of calibrated average FSR readings.
 * @globalvar fsrTriggerLevels Array of trigger thresholds for FSRs.
 * @globalvar fsrNoiseLevelMax Array of maximum noise levels for FSRs.
 * @globalvar fsrStates Array indicating the trigger state of each FSR.
 * @globalvar ledPins Array of LED output pins.
 */
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

/**
 * @brief Updates the main system trigger output based on the collective state of FSR sensors.
 * @details
 * This function iterates through all configured sensors:
 * 1. For each sensor:
 *    - If `fsrStates[idx]` is true (sensor is triggered):
 *      - Sets the corresponding `ledPins[idx]` to `OUTPUT` and HIGH (redundant if `do_sensor()` just did this, but ensures LED state).
 *      - Sets the main `outputPin` to LOW (active state for a Normally Closed setup).
 *      - Increments `finalState`.
 *    - If `fsrStates[idx]` is false:
 *      - Sets the corresponding `ledPins[idx]` to `OUTPUT` and LOW.
 *      - Decrements `finalState`.
 * 2. After checking all sensors, if `finalState` is less than 1 (meaning no sensors are actively triggered, or more sensors are explicitly 'off' than 'on' if logic were more complex), it sets the `outputPin` to HIGH (inactive state for Normally Closed).
 * @note Modifies the state of `outputPin` and `ledPins[]`.
 * @note The `finalState` logic is a simple way to OR the trigger conditions: if any sensor is on, the output is active. If all are off, it's inactive.
 * @globalvar fsrStates Array indicating the trigger state of each FSR.
 * @globalvar ledPins Array of LED output pins.
 * @globalvar outputPin The main system trigger output pin.
 * @globalvar sensorCount Number of sensors to check.
 */
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

/**
 * @brief Sets up the initial pin modes for various control and sensor pins.
 * @details
 * This function configures the `pinMode` for several types of pins:
 * 1. **`outputPin`:** Set to `OUTPUT` and initialized to `HIGH`. This is the main trigger signal output, typically emulating a Normally Closed (NC) switch (HIGH = not triggered, LOW = triggered).
 * 2. **`triggerPin`:** Set to `INPUT_PULLUP` (by setting to `INPUT` then `digitalWrite(pin, HIGH)`). This pin can be used for an external manual trigger or other control.
 * 3. **`calibratePin`:** Set to `INPUT_PULLUP`. This pin can be used to manually initiate a recalibration sequence.
 * 4. **LED Pins (`ledPins[]`):** For each sensor, sets the corresponding LED pin to `OUTPUT` and initializes it to `LOW` (off).
 * 5. **FSR Pins (`fsrPins[]`):**
 *    - If `isX3Mode` is `false` (dedicated FSR pins): Sets each FSR pin to `INPUT` and writes `LOW` to it (disables internal pull-up, ensuring high impedance for analog reading).
 *    - If `isX3Mode` is `true` (shared pins): Pin modes are handled dynamically in `do_sensor()`.
 * @note Modifies pin modes and initial states of `outputPin`, `triggerPin`, `calibratePin`, `ledPins[]`, and `fsrPins[]` (if not in X3 mode).
 * @globalvar outputPin The main system trigger output pin.
 * @globalvar triggerPin External trigger input pin.
 * @globalvar calibratePin Manual calibration input pin.
 * @globalvar ledPins Array of LED output pins.
 * @globalvar fsrPins Array of FSR input pins.
 * @globalvar sensorCount Number of sensors/pins to configure.
 * @globalvar isX3Mode Affects whether FSR pin modes are set here or dynamically.
 */
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

