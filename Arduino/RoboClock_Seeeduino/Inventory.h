#ifndef Inventory_h
#define Inventory_h

#include "Arduino.h"
//28BYJ Stepper Motor Library
#include "CheapStepper.h"

class Inventory {
  public:
    Inventory();
    void initializeState();
    bool requestNumber(int number);
    int presentedNumber();
    void next();
    void previous();
    void updateSelf();
    bool isWorking();
    bool updateInventory(int number);
  private:
    CheapStepper _stepper;
    int _currentTray;
    int _desiredTray;
    int _inventoryState[18];
    bool _isMoving;
};

#endif
