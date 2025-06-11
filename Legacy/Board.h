#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h> // For pin definitions like A0, A1, etc., and pinMode, digitalWrite

const int MAX_SENSORS_PER_BOARD = 3; // Define a constant for max sensors, can be used in BoardConfig

// Moved from AutoTuningFSRTrigger.ino
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

struct BoardConfig {
  BoardType type;
  int outputPin;
  int triggerPin;
  int calibratePin;
  int sensorCount;
  int ledPins[MAX_SENSORS_PER_BOARD];
  int fsrPins[MAX_SENSORS_PER_BOARD];
  bool isX3Mode;
};

class Board {
public:
  Board(); // Constructor
  void initialize(BoardType type);
  void setupPins(); // Handles initial pin modes

  // Getter methods
  int getOutputPin() const;
  int getTriggerPin() const;
  int getCalibratePin() const;
  int getSensorCount() const;
  int getLedPin(int sensorIndex) const;
  int getFsrPin(int sensorIndex) const;
  bool isX3ModeActive() const;
  BoardType getCurrentBoardType() const;

private:
  // Store the selected board configuration
  BoardConfig currentConfig;
  BoardType selectedBoardType; // To store the enum type itself

  // Static array of all known board configurations
  // This will be defined and populated in Board.cpp
  static const BoardConfig boardConfigurations[];
  static const int numBoardConfigurations;
};

#endif // BOARD_H
