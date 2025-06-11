#include "Board.h"
#include <Arduino.h> // Required for HIGH, LOW, INPUT, OUTPUT, INPUT_PULLUP constants

// Define and populate the static array of BoardConfig structures
// This matches the logic previously in do_board_setup()
const BoardConfig Board::boardConfigurations[] = {
  {
    BoardType::ATMEGA328P, // Also ARDUINO_UNO
    13, // outputPin
    8,  // triggerPin
    9,  // calibratePin
    3,  // sensorCount
    {10, 11, 12}, // ledPins
    {A0, A1, A2}, // fsrPins
    false // isX3Mode
  },
  {
    BoardType::ARDUINO_MEGA,
    13, // outputPin
    8,  // triggerPin
    9,  // calibratePin
    3,  // sensorCount
    {10, 11, 12}, // ledPins
    {A0, A1, A2}, // fsrPins
    false // isX3Mode
  },
  {
    BoardType::ATTINY85, // Also ADAFRUIT_TRINKET
    0,  // outputPin
    4,  // triggerPin
    2,  // calibratePin
    1,  // sensorCount
    {1, 1, 1}, // ledPins (only first one is typically used)
    {A1, A1, A1},// fsrPins (only first one is typically used, maps to PB2 on Trinket which is A1)
    false // isX3Mode
  },
  {
    BoardType::ATTINY84,
    0,  // outputPin
    5,  // triggerPin
    4,  // calibratePin
    3,  // sensorCount
    {1, 2, 3},    // ledPins
    {A1, A2, A3}, // fsrPins (actual analog pin numbers)
    false // isX3Mode
  },
  {
    BoardType::ADAFRUIT_TRINKET_X3, // Also ATTINY85_X3
    0,  // outputPin
    4,  // triggerPin
    2,  // calibratePin  (Note: Pin 2 is also an FSR pin in this mode)
    3,  // sensorCount
    {2, 4, 3},    // ledPins (PB2, PB4, PB3 - these are also FSR pins)
    {1, 2, 3},    // fsrPins (ADC channels: ADC1/PB2, ADC2/PB4, ADC3/PB3)
    true  // isX3Mode
  }
  // Note: ARDUINO_UNO, ADAFRUIT_TRINKET, ATTINY85_X3 are covered by their base types or X3 type.
};

const int Board::numBoardConfigurations = sizeof(Board::boardConfigurations) / sizeof(Board::BoardConfig);

Board::Board() {
  // Constructor can be empty or initialize default values if necessary
  // For now, currentConfig will be populated by initialize()
}

void Board::initialize(BoardType type) {
  selectedBoardType = type;
  bool found = false;
  for (int i = 0; i < numBoardConfigurations; ++i) {
    if (boardConfigurations[i].type == type) {
      currentConfig = boardConfigurations[i];
      // Special handling for types that share definitions
      if (type == BoardType::ARDUINO_UNO) currentConfig.type = BoardType::ARDUINO_UNO;
      if (type == BoardType::ADAFRUIT_TRINKET) currentConfig.type = BoardType::ADAFRUIT_TRINKET;
      if (type == BoardType::ATTINY85_X3) currentConfig.type = BoardType::ATTINY85_X3;

      found = true;
      break;
    }
  }
  if (!found) {
    // Fallback or error handling: If no specific config found, default to a known one (e.g., ARDUINO_UNO)
    // Or handle error appropriately, e.g. by setting a default config or a specific error state
    // For now, let's default to ARDUINO_UNO config if type is not explicitly found (e.g. ATTINY85_X3 if ADAFRUIT_TRINKET_X3 was passed)
    // This simplistic fallback might need refinement based on how BoardType enum aliases are handled.
    // A more robust approach would ensure all enum values map to an explicit BoardConfig or handle error.
    for (int i = 0; i < numBoardConfigurations; ++i) {
        if (boardConfigurations[i].type == BoardType::ARDUINO_UNO) { // Default fallback
            currentConfig = boardConfigurations[i];
            selectedBoardType = BoardType::ARDUINO_UNO; // Update selected type to reflect fallback
            break;
        }
    }
  }
}

void Board::setupPins() {
  // Setup for outputPin, triggerPin, calibratePin
  pinMode(currentConfig.outputPin, OUTPUT);
  digitalWrite(currentConfig.outputPin, HIGH); // Default to NC state

  pinMode(currentConfig.triggerPin, INPUT_PULLUP);
  // digitalWrite(currentConfig.triggerPin, HIGH); // INPUT_PULLUP does this

  pinMode(currentConfig.calibratePin, INPUT_PULLUP);
  // digitalWrite(currentConfig.calibratePin, HIGH); // INPUT_PULLUP does this

  // Setup for LED pins
  for (int i = 0; i < currentConfig.sensorCount; ++i) {
    if (currentConfig.ledPins[i] >= 0) { // Basic check for valid pin
        pinMode(currentConfig.ledPins[i], OUTPUT);
        digitalWrite(currentConfig.ledPins[i], LOW); // Initialize LEDs to OFF
    }
  }

  // FSR pins are typically set to INPUT dynamically in do_sensor or do_real_calibration
  // especially for X3 mode. However, if not X3 mode, we can set them here.
  if (!currentConfig.isX3Mode) {
    for (int i = 0; i < currentConfig.sensorCount; ++i) {
      if (currentConfig.fsrPins[i] >= 0) { // Basic check
        pinMode(currentConfig.fsrPins[i], INPUT);
        // digitalWrite(currentConfig.fsrPins[i], LOW); // Ensure no pull-up, let external pull-down do its job
      }
    }
  }
}

// Getter implementations
int Board::getOutputPin() const {
  return currentConfig.outputPin;
}

int Board::getTriggerPin() const {
  return currentConfig.triggerPin;
}

int Board::getCalibratePin() const {
  return currentConfig.calibratePin;
}

int Board::getSensorCount() const {
  return currentConfig.sensorCount;
}

int Board::getLedPin(int sensorIndex) const {
  if (sensorIndex >= 0 && sensorIndex < currentConfig.sensorCount && sensorIndex < MAX_SENSORS_PER_BOARD) {
    return currentConfig.ledPins[sensorIndex];
  }
  return -1; // Invalid index or sensor not configured
}

int Board::getFsrPin(int sensorIndex) const {
  if (sensorIndex >= 0 && sensorIndex < currentConfig.sensorCount && sensorIndex < MAX_SENSORS_PER_BOARD) {
    return currentConfig.fsrPins[sensorIndex];
  }
  return -1; // Invalid index or sensor not configured
}

bool Board::isX3ModeActive() const {
  return currentConfig.isX3Mode;
}

BoardType Board::getCurrentBoardType() const {
  return selectedBoardType; // Return the type passed to initialize()
}
