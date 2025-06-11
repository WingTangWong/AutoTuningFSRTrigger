#include "Board.h"      // Board configuration
#include "FSRManager.h" // FSR data and logic management
#include "LEDController.h" // Manages LED outputs
#include <Arduino.h>    // For millis(), delay()

// Global objects
Board board;
FSRManager fsrManager(board);
LEDController ledController(board, fsrManager); // Instantiate LEDController

// Constants
const int CALIBRATION_SEED = 5;

// Global states not yet encapsulated
bool isDebugMode;
bool hasCalibrationRun = false;
unsigned long lastCalibrationTimestamp = 0;
unsigned long calibrationIntervalMs = 500;
bool systemTriggerState = false;

/**
 * @brief Initializes the microcontroller, board, FSR manager, LEDs, and performs initial calibration.
 */
void setup() {
  board.initialize(BoardType::ADAFRUIT_TRINKET_X3);
  isDebugMode = false;

  fsrManager.initializeSensors();
  board.setupPins(); // Sets up general pins, including initial LED pin modes
  // ledController.setupLEDs(); // Not strictly needed if Board::setupPins covers it.

  lastCalibrationTimestamp = millis();
  do_calibration();
}

/**
 * @brief Main execution loop.
 */
void loop() {
  fsrManager.readAllSensors(); // Read sensors first
  ledController.update();      // Update LEDs based on new sensor states
  do_trigger();                // Update main trigger output
  do_calibration();            // Check for recalibration
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
    lastCalibrationTimestamp = millis();
  }

  if ((millis() - lastCalibrationTimestamp) > calibrationIntervalMs) {
    if (systemTriggerState == false) {
      fsrManager.performCalibrationCycle(CALIBRATION_SEED);
      lastCalibrationTimestamp = millis();
    }
  }
}

// void do_sensor() // This function is now effectively replaced by calls to
                  // fsrManager.readAllSensors() and ledController.update() in loop()

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
