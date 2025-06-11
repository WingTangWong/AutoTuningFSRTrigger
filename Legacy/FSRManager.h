#ifndef FSR_MANAGER_H
#define FSR_MANAGER_H

#include "Board.h" // Needs Board class for board-specific info
                   // MAX_SENSORS_PER_BOARD is defined in Board.h

class FSRManager {
public:
  FSRManager(Board& boardRef);
  void initializeSensors();
  void performCalibrationCycle(int calibrationSeed); // Takes seed as argument
  void readAllSensors();
  bool isSensorTriggered(int sensorIndex) const;
  bool isAnySensorTriggered() const;
  unsigned long getFsrValue(int sensorIndex) const;
  int getActiveSensorCount() const;

private:
  Board& board;

  bool fsrStates[MAX_SENSORS_PER_BOARD];
  unsigned long fsrValues[MAX_SENSORS_PER_BOARD];
  unsigned long fsrAverages[MAX_SENSORS_PER_BOARD];
  unsigned long fsrNoiseLevels[MAX_SENSORS_PER_BOARD];
  unsigned long fsrTriggerLevels[MAX_SENSORS_PER_BOARD];
  unsigned long fsrTallies[MAX_SENSORS_PER_BOARD];
  unsigned long fsrTotals[MAX_SENSORS_PER_BOARD];
  unsigned long fsrNoiseLevelMax[MAX_SENSORS_PER_BOARD];
};

#endif // FSR_MANAGER_H
