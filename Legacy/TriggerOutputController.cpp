#include "TriggerOutputController.h"
#include <Arduino.h> // For digitalWrite

TriggerOutputController::TriggerOutputController(Board& boardRef, FSRManager& fsrManagerRef)
  : board(boardRef), fsrManager(fsrManagerRef), currentSystemTriggerState(false) {
  // Initialize with system not triggered
}

void TriggerOutputController::update() {
  if (fsrManager.isAnySensorTriggered()) {
    digitalWrite(board.getOutputPin(), LOW); // Active state (LOW for NC)
    currentSystemTriggerState = true;
  } else {
    digitalWrite(board.getOutputPin(), HIGH); // Inactive state (HIGH for NC)
    currentSystemTriggerState = false;
  }
}

bool TriggerOutputController::isSystemTriggered() const {
  return currentSystemTriggerState;
}
