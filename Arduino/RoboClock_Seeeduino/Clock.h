#ifndef Clock_h
#define Clock_h

#include "Arduino.h"
#include "Clock.h"
#include "Arm.h"
#include "Inventory.h"

class Clock {
  public:
    Clock();
    Arm arm;
    Inventory inventory;
    void setTime(int hour, int minute);
    void initClockState(int h0, int h1, int m0, int m1);
    void initStorageState();
    void run();
  private:
    int _clockState[4];
    int _desiredClockState[4];
    int _operationQueue[24]; 
    int _performingOperation;
    int _numberInTransit;
    void addOperationToQueue(int operation);
    int getPickOperation(int index);
    int getPlaceOperation(int index);
    bool isWorking();
    bool hasWorkToDo();
    void parseDesiredClockState(int hour, int minute);
    void compareStatesAndPlanOperationQueue();
    void describeOperation(int op);
    void printOperationQueue();
};

#endif
