#include "FSRManager.h"
#include <Arduino.h>

FSRManager::FSRManager(Board& boardRef) : board(boardRef) {
  // Constructor
}

void FSRManager::initializeSensors() {
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
}

// Takes calibrationSeed as an argument now
void FSRManager::performCalibrationCycle(int calibrationSeed) {
  int sensorCount = board.getSensorCount();
  for (int idx = 0; idx < sensorCount; ++idx) {
    if (board.isX3ModeActive()) {
        pinMode(board.getFsrPin(idx), INPUT);
        digitalWrite(board.getFsrPin(idx), LOW);
    }

    if (fsrTallies[idx] < calibrationSeed) {
      for (int run = 0; run <= calibrationSeed; ++run) {
        fsrValues[idx] = analogRead(board.getFsrPin(idx));
        fsrTotals[idx] = fsrTotals[idx] + fsrValues[idx];
        fsrTallies[idx]++;
      }
      if (fsrTallies[idx] > 0) {
        fsrAverages[idx] = (unsigned long)((fsrTotals[idx] * 1.00) / (fsrTallies[idx] * 1.00));
      } else {
        fsrAverages[idx] = 0;
      }
    }

    fsrValues[idx] = analogRead(board.getFsrPin(idx));
    fsrAverages[idx] = (unsigned long)(((fsrAverages[idx]) + fsrValues[idx]) / 2.00);

    unsigned long noiseDelta;
    if (fsrAverages[idx] > fsrValues[idx]) {
      noiseDelta = abs(fsrAverages[idx] - fsrValues[idx]);
    } else {
      noiseDelta = abs(fsrValues[idx] - fsrAverages[idx]);
    }
    fsrNoiseLevels[idx] = (unsigned long)((fsrNoiseLevels[idx] + noiseDelta) / 2.00);

    fsrNoiseLevels[idx] = fsrNoiseLevels[idx] + 8;

    if (fsrNoiseLevels[idx] > fsrNoiseLevelMax[idx]) {
      fsrNoiseLevelMax[idx] = fsrNoiseLevels[idx];
    }
    fsrNoiseLevelMax[idx] = (unsigned long)(((fsrNoiseLevelMax[idx] * 1.00) + (fsrNoiseLevels[idx] * 1.00)) / 2.00);

    fsrTriggerLevels[idx] = fsrNoiseLevelMax[idx];
  }
}

void FSRManager::readAllSensors() {
  int sensorCount = board.getSensorCount();

  if (board.isX3ModeActive()) {
    for (int idx = 0; idx < sensorCount; ++idx) {
      pinMode(board.getFsrPin(idx), INPUT);
      digitalWrite(board.getFsrPin(idx), LOW);
    }
  }

  for (int idx = 0; idx < sensorCount; ++idx) {
    fsrValues[idx] = analogRead(board.getFsrPin(idx));
    fsrValues[idx] = analogRead(board.getFsrPin(idx));

    if (fsrValues[idx] > (fsrAverages[idx] + fsrTriggerLevels[idx])) {
      fsrStates[idx] = true;
    } else if (fsrValues[idx] < (fsrAverages[idx] + fsrNoiseLevelMax[idx])) { // Added 'else if' for clarity
      fsrStates[idx] = false;
    }
    // If in between, state remains unchanged.
  }
}

bool FSRManager::isSensorTriggered(int sensorIndex) const {
  if (sensorIndex >= 0 && sensorIndex < board.getSensorCount()) {
    return fsrStates[sensorIndex];
  }
  return false;
}

bool FSRManager::isAnySensorTriggered() const {
  for (int i = 0; i < board.getSensorCount(); ++i) {
    if (fsrStates[i]) {
      return true;
    }
  }
  return false;
}

unsigned long FSRManager::getFsrValue(int sensorIndex) const {
  if (sensorIndex >= 0 && sensorIndex < board.getSensorCount()) {
    return fsrValues[sensorIndex];
  }
  return 0; // Return 0 or some error indicator for invalid index
}

int FSRManager::getActiveSensorCount() const {
    return board.getSensorCount();
}
```
