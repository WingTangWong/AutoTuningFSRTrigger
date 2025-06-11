#include "LEDController.h"
#include <Arduino.h> // For digitalWrite, pinMode (though pinMode is mainly in Board)

LEDController::LEDController(Board& boardRef, FSRManager& fsrManagerRef)
  : board(boardRef), fsrManager(fsrManagerRef) {
  // Constructor
}

void LEDController::update() {
  for (int i = 0; i < board.getSensorCount(); ++i) {
    // Ensure LED pin is output; Board::setupPins should have done this,
    // but for X3 mode, pins might have been INPUT for FSR reading.
    // However, FSRManager::readAllSensors() should have left FSR pins as INPUT
    // and LED pins (if shared) would need to be explicitly set to OUTPUT here.
    // If not X3, ledPins are distinct and already OUTPUT.
    // Board::setupPins initializes LED pins to OUTPUT.
    // If X3 mode, FSRManager has just used the shared pin as INPUT.
    // So, explicitly setting to OUTPUT here is crucial for X3 mode.
    pinMode(board.getLedPin(i), OUTPUT);

    if (fsrManager.isSensorTriggered(i)) {
      digitalWrite(board.getLedPin(i), HIGH);
    } else {
      digitalWrite(board.getLedPin(i), LOW);
    }
  }
}
