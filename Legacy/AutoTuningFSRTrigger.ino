#include "Board.h"                 // Board configuration
#include "FSRManager.h"            // FSR data and logic management
#include "LEDController.h"         // Manages LED outputs
#include "TriggerOutputController.h" // Manages main trigger output and system state
#include <Arduino.h>               // For millis(), delay()

// Global objects
Board board;
FSRManager fsrManager(board);
LEDController ledController(board, fsrManager);
TriggerOutputController triggerController(board, fsrManager); // Instantiate TriggerOutputController

// Constants
const int CALIBRATION_SEED = 5;

// Global states not yet encapsulated / sketch-wide settings
bool isDebugMode;
bool hasCalibrationRun = false;
unsigned long lastCalibrationTimestamp = 0;
unsigned long calibrationIntervalMs = 500;
// bool systemTriggerState = false; // Now managed by TriggerOutputController

/**
 * @brief Initializes the microcontroller, board, FSR manager, LEDs, trigger output and performs initial calibration.
 */
void setup() {
  board.initialize(BoardType::ADAFRUIT_TRINKET_X3);
  isDebugMode = false;
  
  fsrManager.initializeSensors();
  board.setupPins();
  // triggerController.update(); // Initial state set by constructor, update in loop first.
  
  lastCalibrationTimestamp = millis();
  do_calibration();
}

/**
 * @brief Main execution loop.
 */
void loop() {
  fsrManager.readAllSensors();
  ledController.update();
  triggerController.update(); // Updates main output pin and internal systemTriggerState
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
    lastCalibrationTimestamp = millis();
  }

  // Check if timed recalibration is due and if the system is not currently triggered.
  // Uses triggerController.isSystemTriggered() now.
  if ((millis() - lastCalibrationTimestamp) > calibrationIntervalMs) {
    if (triggerController.isSystemTriggered() == false) {
      fsrManager.performCalibrationCycle(CALIBRATION_SEED);
      lastCalibrationTimestamp = millis();
    }
  }
}

// do_sensor() is fully replaced by fsrManager.readAllSensors() and ledController.update()
// do_trigger() is fully replaced by triggerController.update()

```
