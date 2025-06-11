#ifndef TRIGGER_OUTPUT_CONTROLLER_H
#define TRIGGER_OUTPUT_CONTROLLER_H

#include "Board.h"
#include "FSRManager.h"

class TriggerOutputController {
public:
  TriggerOutputController(Board& boardRef, FSRManager& fsrManagerRef);
  void update();
  bool isSystemTriggered() const;

private:
  Board& board;
  FSRManager& fsrManager;
  bool currentSystemTriggerState; // Internal state
};

#endif // TRIGGER_OUTPUT_CONTROLLER_H
