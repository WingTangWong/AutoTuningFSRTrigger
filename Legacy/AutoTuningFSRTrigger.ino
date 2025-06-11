// Content for Legacy/AutoTuningFSRTrigger.ino after Phase 2

#include "Board.h" // Include the Board class header
#include <Arduino.h> // For abs(), millis(), delay()

// Global Board object
Board board;

// Constants
// CALIBRATION_SEED is used by FSRManager, which will be integrated next.
// For now, it's a sketch-specific constant if do_calibration directly uses it.
// The FSRManager will take it as an argument.
const int CALIBRATION_SEED = 5;

// Sensor-specific state arrays - these will be moved to FSRManager
// For now, define them here, sized by MAX_SENSORS_PER_BOARD from Board.h
bool fsrStates[MAX_SENSORS_PER_BOARD];
unsigned long fsrValues[MAX_SENSORS_PER_BOARD];
unsigned long fsrAverages[MAX_SENSORS_PER_BOARD];
unsigned long fsrNoiseLevels[MAX_SENSORS_PER_BOARD];
unsigned long fsrTriggerLevels[MAX_SENSORS_PER_BOARD];
unsigned long fsrTallies[MAX_SENSORS_PER_BOARD];
unsigned long fsrTotals[MAX_SENSORS_PER_BOARD];
unsigned long fsrNoiseLevelMax[MAX_SENSORS_PER_BOARD];

// Other global states
bool isDebugMode; // Should be initialized in setup()
bool hasCalibrationRun = false;
unsigned long lastCalibrationTimestamp = 0; // Initialize to 0
unsigned long calibrationIntervalMs = 500;
bool systemTriggerState = false; // Overall system trigger state

// Global loop iterator 'idx' is generally bad practice but used in original.
// Will be removed as logic moves into classes.

/**
 * @brief Initializes the microcontroller, board configuration, pin modes, and performs initial sensor calibration.
 * @details This function is called once at power-up or reset. It performs the following key actions:
 *          1. Initializes the `board` object with the desired `BoardType`.
 *          2. Sets `isDebugMode`.
 *          3. Initializes FSR data arrays.
 *          4. Calls `board.setupPins()` to configure pin modes.
 *          5. Calls `do_calibration()` to perform the initial FSR calibration sequence.
 */
void setup() {
  // Select your board type! This should be the only place to change it.
  board.initialize(BoardType::ADAFRUIT_TRINKET_X3);

  isDebugMode = false;

  // Initialize FSR state arrays based on the actual sensorCount for the selected board
  for (int i = 0; i < board.getSensorCount(); ++i) {
    fsrValues[i] = 0;
    fsrAverages[i] = 0;
    fsrNoiseLevels[i] = 0;
    fsrTriggerLevels[i] = 0;
    fsrTallies[i] = 0;
    fsrTotals[i] = 0;
    fsrNoiseLevelMax[i] = 0;
    fsrStates[i] = false;
  }

  board.setupPins();

  lastCalibrationTimestamp = millis(); // Initialize after potential delays in setup.
  do_calibration();
}

/**
 * @brief Main execution loop, called repeatedly after `setup()`.
 * @details This function continuously performs the following actions:
 *          1. Calls `do_sensor()` to read FSR values and update their states.
 *          2. Calls `do_trigger()` to update the main system trigger output.
 *          3. Calls `do_calibration()` for potential periodic recalibration.
 */
void loop() {
  do_sensor();
  do_trigger();
  do_calibration(); // Check for recalibration needs
}

/**
 * @brief Performs the actual FSR calibration process for all sensors.
 * @details This function is the core of the calibration logic. For each sensor:
 *          1. Sets the FSR input pin to `INPUT` mode if in X3 mode.
 *          2. Takes multiple initial readings (`CALIBRATION_SEED` times) to establish a baseline average.
 *          3. Updates the running average and calculates noise levels.
 *          4. Sets the trigger level based on the noise characteristics.
 *          5. Updates `lastCalibrationTimestamp`.
 *          6. Sets LED pins to `OUTPUT` mode and LOW (this LED part will move to LEDController).
 * @note Modifies global FSR arrays and `lastCalibrationTimestamp`. Changes pin modes.
 */
void do_real_calibration() {
  // This function's logic will largely move to FSRManager::performCalibrationCycle()
  // For now, it uses global FSR arrays and board object for pin info.

  for (int i = 0; i < board.getSensorCount(); ++i) {
    if (board.isX3ModeActive()) {
      pinMode(board.getFsrPin(i), INPUT);
      digitalWrite(board.getFsrPin(i), LOW);
    }
    // For non-X3, Board::setupPins should have already set FSR pins to INPUT.
  }

  for (int i = 0; i < board.getSensorCount(); ++i) {
    if (fsrTallies[i] < CALIBRATION_SEED) {
      for (int run = 0; run <= CALIBRATION_SEED; ++run) {
        fsrValues[i] = analogRead(board.getFsrPin(i));
        fsrTotals[i] = fsrTotals[i] + fsrValues[i];
        fsrTallies[i]++;
      }
      if (fsrTallies[i] > 0) { // Avoid division by zero
        fsrAverages[i] = (unsigned long)((fsrTotals[i] * 1.00) / (fsrTallies[i] * 1.00));
      } else {
        fsrAverages[i] = 0; // Should not happen if CALIBRATION_SEED >= 0
      }
    }

    fsrValues[i] = analogRead(board.getFsrPin(i));
    fsrAverages[i] = (unsigned long)(((fsrAverages[i]) + fsrValues[i]) / 2.00);

    unsigned long noiseDelta;
    if (fsrAverages[i] > fsrValues[i]) {
      noiseDelta = abs(fsrAverages[i] - fsrValues[i]);
    } else {
      noiseDelta = abs(fsrValues[i] - fsrAverages[i]);
    }
    fsrNoiseLevels[i] = (unsigned long)((fsrNoiseLevels[i] + noiseDelta) / 2.00);

    fsrNoiseLevels[i] = fsrNoiseLevels[i] + 8; // Empirical adjustment

    if (fsrNoiseLevels[i] > fsrNoiseLevelMax[i]) {
      fsrNoiseLevelMax[i] = fsrNoiseLevels[i];
    }
    fsrNoiseLevelMax[i] = (unsigned long)(((fsrNoiseLevelMax[i] * 1.00) + (fsrNoiseLevels[i] * 1.00)) / 2.00);
    fsrTriggerLevels[i] = fsrNoiseLevelMax[i];
  }
  lastCalibrationTimestamp = millis();

  // This part (setting LED pins) will be managed by an LEDController later.
  for (int i = 0; i < board.getSensorCount(); ++i) {
    pinMode(board.getLedPin(i), OUTPUT);
    digitalWrite(board.getLedPin(i), LOW);
  }
}

/**
 * @brief Manages the FSR calibration process, deciding when to call `do_real_calibration()`.
 * @details This function controls calibration timing:
 *          1. **Initial Priming:** If `!hasCalibrationRun`, calls `do_real_calibration()` multiple times.
 *          2. **Periodic Recalibration:** If `calibrationIntervalMs` has passed since `lastCalibrationTimestamp`
 *             and the system is not triggered (`!systemTriggerState`), calls `do_real_calibration()`.
 * @note Modifies `hasCalibrationRun` and calls `do_real_calibration()`.
 */
void do_calibration() {
  if (!hasCalibrationRun) {
    // Perform initial calibration multiple times to allow averages to stabilize.
    for (int i = 0; i < 5; i++) {
      do_real_calibration();
      delay(5);
    }
    hasCalibrationRun = true;
  }

  // Check if timed recalibration is due and if the system is not currently triggered.
  if ((millis() - lastCalibrationTimestamp) > calibrationIntervalMs) {
    if (systemTriggerState == false) { // Use the global systemTriggerState
      do_real_calibration();
    }
  }
}

/**
 * @brief Reads FSR sensor values, updates their states, and manages LED indicators.
 * @details This function is responsible for:
 *          1. **Pin Mode (X3):** If `isX3ModeActive()`, sets shared FSR/LED pins to `INPUT`.
 *          2. **Sensor Reading:** For each sensor, takes two analog readings (second is used).
 *          3. **State Update:** Sets `fsrStates[i]` based on thresholds.
 *          4. **LED Update:** Sets LED pins to `OUTPUT` and updates state. (This LED part will move).
 * @note Modifies global `fsrValues`, `fsrStates`. Changes pin modes and LED states.
 */
void do_sensor() {
  if (board.isX3ModeActive()) {
    for (int i = 0; i < board.getSensorCount(); ++i) {
      // In X3 mode, FSR pins are shared with LEDs; ensure they are set to INPUT before reading.
      pinMode(board.getFsrPin(i), INPUT);
      digitalWrite(board.getFsrPin(i), LOW);
    }
  }

  for (int i = 0; i < board.getSensorCount(); ++i) {
    fsrValues[i] = analogRead(board.getFsrPin(i));
    // Take two consecutive readings; the first may help settle the ADC.
    fsrValues[i] = analogRead(board.getFsrPin(i));

    if (fsrValues[i] > (fsrAverages[i] + fsrTriggerLevels[i])) {
      fsrStates[i] = true;
    } else if (fsrValues[i] < (fsrAverages[i] + fsrNoiseLevelMax[i])) {
      // Ensure a clear condition for turning off, providing hysteresis.
      fsrStates[i] = false;
    }
    // Note on trigger logic:
    // fsrTriggerLevels[i] is typically equal to fsrNoiseLevelMax[i] after calibration.
    // Trigger ON if: fsrValue > fsrAverage + fsrNoiseLevelMax
    // Trigger OFF if: fsrValue < fsrAverage + fsrNoiseLevelMax
    // Hysteresis is managed by the slow adaptation of fsrAverages/fsrNoiseLevelMax
    // or the difference between the trigger-on and trigger-off conditions.
  }

  // This LED update part will move to an LEDController.
  for (int i = 0; i < board.getSensorCount(); ++i) {
    // Ensure LED pin is set to output before writing.
    pinMode(board.getLedPin(i), OUTPUT);
    if (fsrStates[i] == true) {
      digitalWrite(board.getLedPin(i), HIGH);
    } else {
      digitalWrite(board.getLedPin(i), LOW);
    }
  }
}

/**
 * @brief Updates the main system trigger output based on the collective state of FSR sensors.
 * @details Iterates through sensors. If any sensor is triggered (`fsrStates[i] == true`),
 *          the main `outputPin` is set LOW (active for NC). Otherwise, it's set HIGH.
 *          It also updates `systemTriggerState`.
 * @note Modifies `outputPin` state and `systemTriggerState`. LED logic here is mostly redundant with do_sensor.
 */
void do_trigger() {
  bool anySensorActive = false;
  for (int i = 0; i < board.getSensorCount(); ++i) {
    if (fsrStates[i] == true) {
      anySensorActive = true;
      // Redundant LED control if also in do_sensor, but ensures correctness for this function's scope.
      // This will be cleaned up when LEDController is introduced.
      // pinMode(board.getLedPin(i), OUTPUT);
      // digitalWrite(board.getLedPin(i), HIGH);
    } else {
      // pinMode(board.getLedPin(i), OUTPUT);
      // digitalWrite(board.getLedPin(i), LOW);
    }
  }

  if (anySensorActive) {
    digitalWrite(board.getOutputPin(), LOW);
    systemTriggerState = true;
  } else {
    digitalWrite(board.getOutputPin(), HIGH);
    systemTriggerState = false;
  }
}
```
