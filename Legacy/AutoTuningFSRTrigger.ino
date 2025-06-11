#include "Board.h"      // Board configuration
#include "FSRManager.h" // FSR data and logic management
#include <Arduino.h>    // For millis(), delay()

// Global objects
Board board;
FSRManager fsrManager(board); // Pass board reference to FSRManager

// Constants
const int CALIBRATION_SEED = 5; // Seed for initial calibration stability

// Global states not yet encapsulated
bool isDebugMode;
bool hasCalibrationRun = false;
unsigned long lastCalibrationTimestamp = 0;
unsigned long calibrationIntervalMs = 500;
bool systemTriggerState = false; // Overall system trigger state

/**
 * @brief Initializes the microcontroller, board configuration, pin modes, FSR manager, and performs initial sensor calibration.
 */
void setup() {
  board.initialize(BoardType::ADAFRUIT_TRINKET_X3); // Example board type
  isDebugMode = false;

  fsrManager.initializeSensors(); // Initialize FSR data arrays within FSRManager

  board.setupPins(); // Setup general pin modes via Board object

  lastCalibrationTimestamp = millis();
  do_calibration(); // Perform initial calibration
}

/**
 * @brief Main execution loop: reads sensors, updates triggers, and handles recalibration.
 */
void loop() {
  do_sensor();
  do_trigger();
  do_calibration();
}

/**
 * @brief Manages the FSR calibration process.
 */
void do_calibration() {
  if (!hasCalibrationRun) {
    for (int i = 0; i < 5; i++) {
      fsrManager.performCalibrationCycle(CALIBRATION_SEED);
      delay(5);
    }
    hasCalibrationRun = true;
    // Update timestamp after initial calibration priming is fully complete
    lastCalibrationTimestamp = millis();
  }

  if ((millis() - lastCalibrationTimestamp) > calibrationIntervalMs) {
    if (systemTriggerState == false) {
      fsrManager.performCalibrationCycle(CALIBRATION_SEED);
      lastCalibrationTimestamp = millis(); // Update timestamp after successful recalibration
    }
  }
}

/**
 * @brief Reads FSR sensor values via FSRManager and updates LED indicators.
 * @details LED update logic is temporarily kept here and will be moved to an LEDController.
 */
void do_sensor() {
  fsrManager.readAllSensors(); // FSRManager now handles reading and state determination

  // LED update logic (to be moved to LEDController)
  for (int i = 0; i < board.getSensorCount(); ++i) {
    pinMode(board.getLedPin(i), OUTPUT); // Ensure LED pin is output
    if (fsrManager.isSensorTriggered(i)) {
      digitalWrite(board.getLedPin(i), HIGH);
    } else {
      digitalWrite(board.getLedPin(i), LOW);
    }
  }
}

/**
 * @brief Updates the main system trigger output based on FSR states from FSRManager.
 */
void do_trigger() {
  if (fsrManager.isAnySensorTriggered()) {
    digitalWrite(board.getOutputPin(), LOW);
    systemTriggerState = true;
  } else {
    digitalWrite(board.getOutputPin(), HIGH);
    systemTriggerState = false;
  }
}
```
