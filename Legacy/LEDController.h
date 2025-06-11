#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "Board.h"
#include "FSRManager.h"

class LEDController {
public:
  LEDController(Board& boardRef, FSRManager& fsrManagerRef);
  // void setupLEDs(); // Pins are already set up by Board::setupPins()
  void update();    // Updates all LED states based on FSRManager

private:
  Board& board;
  FSRManager& fsrManager;
};

#endif // LED_CONTROLLER_H
