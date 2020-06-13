#include "Arduino.h"
#include "Clock.h"
#include "Inventory.h"
#include "Arm.h"

//Operations Key
//Inventory (30 + (requested item))
// 30 - 1 == 29 -> Request an empty tray
// 30 + 0 == 30 -> Request a 'Zero'
// ...
// 30 + 9 == 39 -> Request a 'Nine'

//Subroutine Key
//  0 -> Active Home
//  1 -> Pick 0
//  2 -> Pick 1
//  3 -> Pick 2
//  4 -> Pick 3
//  5 -> Pick Tray
//  6 -> Place 0
//  7 -> Place 1
//  8 -> Place 2
//  9 -> Place 3
// 10 -> Place Tray

int PICK_TRAY  =  5;
int PLACE_TRAY = 10;
int INVENTORY_OPERATION_OFFSET = 30;

Clock::Clock() : arm(5,6), inventory() { }

void Clock::initClockState(int h0, int h1, int m0, int m1) {
  _clockState[0] = h0;
  _clockState[1] = h1;
  _clockState[2] = m0;
  _clockState[3] = m1;

  _desiredClockState[0] = -1;
  _desiredClockState[1] = -1;
  _desiredClockState[2] = -1;
  _desiredClockState[3] = -1;

  _performingOperation = -1;
  _numberInTransit = -1;
  Serial.println("Clock::Initialized Clock State");

  //Initialize Storage State / Inventory & Operation Queue...
  initStorageState();
  
  //Initialize Arm
  arm.initialize();
}

void Clock::initStorageState() {
  //Initialize our Ineventory State (the numbers that sit on each tray)
  inventory.initializeState();

  //Clear our Operation Queue
  //   -1 == no operation
  // < 20 == Subroutine
  // > 20 == Chain Rotation 
  for (int i = 0; i < 24; i++) {
    _operationQueue[i] = -1;
  }
  Serial.println("Clock::Initialized Storage State");
}

//Main Run Loop For Clock Updates
//Clock uses this chance to...
// - Pass updates along to our Inventory/Chain Drive
// - Manage the Arms subroutine scripts 
void Clock::run() {
  //Update the Chain Stepper Motor if necessary...
  inventory.updateSelf();
  arm.updateSelf();

  //If we're working, lets wait for our current task to finish...
  if (isWorking()) {
    return;
  }

  //If there's work to do (and we're not working) lets do it...
  if(hasWorkToDo() && _performingOperation == -1) { 
    int currentOperation = _operationQueue[0];
    if (currentOperation > 20) { //We need to present a number...
      if (inventory.requestNumber(currentOperation - INVENTORY_OPERATION_OFFSET)) {
        _performingOperation = currentOperation;
        describeOperation(currentOperation); //Prints the current operation out to the console...
      } else {
        Serial.print("Clock::Failed to execute task: Inventory Rejected Request for Number: ");
        Serial.println(currentOperation - INVENTORY_OPERATION_OFFSET);
      }
    } else if (currentOperation >= 0 && currentOperation <= 10) { //We need to run a sub routine... 
      if (arm.startSequence(currentOperation)) {
        _performingOperation = currentOperation;
        describeOperation(currentOperation); //Prints the current operation out to the console...
      } else {
        Serial.print("Clock::Failed to execute task: Arm Rejected Request for subroutine: ");
        Serial.println(currentOperation);
      }
    } else {
      Serial.print("Clock::Unknown Operation: ");
      Serial.println(currentOperation);
    }
  }
}

void Clock::setTime(int hour, int minute) {
  //Make sure we're not already working
  if (hasWorkToDo() == true) {
    //Serial.println("Clock::Failed to setTime due to a previous task that's still executing");
    return;
  }

  //Parse the desired clock state
  parseDesiredClockState(hour, minute);
  
  //Compare current clockState with desiredClockState
  //and determine which numbers need to be replaced / changed
  compareStatesAndPlanOperationQueue();
  printOperationQueue();
}

//isWorking - Called every loop via run() method...
bool Clock::isWorking() {
  if (arm.isWorking() || inventory.isWorking()) {
    return true;
  }
  if (_performingOperation >= 0) { //We just finished an operation, update our opQueue...
    //Remove Operation from operation queue...
    int removedOp = -1;
    for (int i = 0; i < 24; i++) {
      if (_operationQueue[i] == _performingOperation) {
        _operationQueue[i] = -1;
        removedOp = i; //Should always be 0 unless we start handling async task execution...
        break;
      }
    }
    //Shift operation queue
    for (int i = removedOp; i < 23; i++) {
      _operationQueue[i] = _operationQueue[i+1];
    }
    _operationQueue[23] = -1;

    //Log the operation we just completed...
    Serial.print("Clock::Finished Operation: ");
    Serial.println(_performingOperation);

    if (_performingOperation == 5) { //Picked Tray
      //Log number picked in `numberInTransit`
      _numberInTransit = inventory.presentedNumber();
      //Update our Inventory to reflect the now empty tray
      inventory.updateInventory(-1);
    } else if (_performingOperation == 10) { //Placeed Tray
      //Update our Inventory to reflect the newly placed number
      inventory.updateInventory(_numberInTransit);
      //Clear our `numberInTransit` variable
      _numberInTransit = -1;
    } else if (_performingOperation >= 1 && _performingOperation < 5) { //Picked Number
      //Update `numberInTransit` based on our clock's state
      _numberInTransit = _clockState[_performingOperation - 1];
      //Update our clocks state to reflect the now empty clock index
      _clockState[_performingOperation - 1] = -1;
    } else if (_performingOperation >= 6 && _performingOperation < 10) { //Placed Number
      //Update our clocks state with the numberInTransit that was just placed
      _clockState[_performingOperation - 6] = _numberInTransit;
      //Clear our `numberInTransit` variable
      _numberInTransit = -1;
    }
    
    //Reset our _performingOperation variable
    _performingOperation = -1;

    //Log the updated operationQueue
    printOperationQueue();
  }
  return false;
}

//hasWorkToDo
bool Clock::hasWorkToDo() {
  for (int i = 0; i < 24; i++) {
    if (_operationQueue[i] != -1) {
      return true;
    }
  }
  return false;
}

//Given an hour and a minute this method parses them into 2 seperate hour digits and 2 seperate minute digits and updates our `_desiredClockState`
void Clock::parseDesiredClockState(int hour, int minute) {
  //Reset Desired Clock State
  _desiredClockState[0] = -1;
  _desiredClockState[1] = -1;
  _desiredClockState[2] = -1;
  _desiredClockState[3] = -1;
  
  //Parse Desired Time
  int largeHour = hour / 10;
  if (largeHour == 1) {
    _desiredClockState[0] = 1; //Place a 1 in index 0
  } else {
    _desiredClockState[0] = -1; //Make sure index 0 is empty
  }
  _desiredClockState[1] = hour % 10;
  _desiredClockState[2] = minute / 10;
  int smallMinute = minute % 10;
  if (smallMinute >=3 && smallMinute <= 7) {
    _desiredClockState[3] = 5;
  } else {
    _desiredClockState[3] = 0;
  }

  //Print / Debug State
  Serial.print("Clock::Desired Clock State: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(_desiredClockState[i]);
    Serial.print("|"); 
  }
  Serial.println("");
  Serial.print("Clock::Current Clock State: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(_clockState[i]);
    Serial.print("|"); 
  }
  Serial.println("");
}

// Compare current clockState with desiredClockState
// and determine which numbers need to be replaced / changed
void Clock::compareStatesAndPlanOperationQueue() {
  for (int j = 0; j < 4; j++) {
    if (_clockState[j] != _desiredClockState[j]) {
      if (_clockState[j] != -1) {
        //We need to remove the current number from the podium before placing the desired number...
        Serial.print("Clock::We need to remove the '");
        Serial.print(_clockState[j]);
        Serial.print("' at index: ");
        Serial.print(j);
        Serial.println(" before proceeding");
        addOperationToQueue(getPickOperation(j));
        addOperationToQueue(INVENTORY_OPERATION_OFFSET - 1); //Rotate to Empty...
        addOperationToQueue(PLACE_TRAY);
      }
      if (_desiredClockState[j] != -1) {
        //Add an operation to the queue...
        Serial.print("Clock::We need to place a '");
        Serial.print(_desiredClockState[j]);
        Serial.print("' at index: ");
        Serial.println(j);
        addOperationToQueue(INVENTORY_OPERATION_OFFSET + _desiredClockState[j]); //Rotate to desired number...
        addOperationToQueue(PICK_TRAY);
        addOperationToQueue(getPlaceOperation(j));
      }
    }
    Serial.println("");
  }
}

//Appends an operation to the end of our operation queue.
//This queue will be worked through based on a first in first out order.
//Each operation can take an indeterminate amount of time and will be removed from the queue once executed.
void Clock::addOperationToQueue(int operation) {
  int index = 0;
  while (_operationQueue[index] != -1 || index > 24) {
    index++;
  }
  Serial.print("Clock::Adding op ");
  Serial.print(operation);
  Serial.print(" at index ");
  Serial.println(index);
  _operationQueue[index] = operation;
}

//Given an index to PICK, this method returns the appropriate subroutine id
int Clock::getPickOperation(int index) {
  if (index == 0) {
    return 1;
  } else if (index == 1) {
    return 2;
  } else if (index == 2) {
    return 3;
  } else if (index == 3) {
    return 4;
  } else { 
    return -2;
  }
}

//Given an index to PLACE, this method returns the appropriate subroutine id
int Clock::getPlaceOperation(int index) {
  if (index == 0) {
    return 6;
  } else if (index == 1) {
    return 7;
  } else if (index == 2) {
    return 8;
  } else if (index == 3) {
    return 9;
  } else { 
    return -2;
  }
}

void Clock::printOperationQueue() {
  Serial.print("Clock::Operation Queue: |");
  for(int i = 0; i < 24; i++) {
    Serial.print(_operationQueue[i]);
    Serial.print("|");
  }
  Serial.println("");
}

void Clock::describeOperation(int op) {
  if (op == -1) { //Empty / No Operation
    Serial.println("Clock::No Operation");
  } else if (op > 10) { //Inventory Operation
    Serial.print("Clock::Requesting a '");
    if ((op - INVENTORY_OPERATION_OFFSET) == -1) {
      Serial.print("empty tray");
    } else {
      Serial.print(op - INVENTORY_OPERATION_OFFSET);
    }
    Serial.println("' from our Inventory");
  } else { //Arm Subroutine Operation
    Serial.print("Clock::Requesting Arm ");
    switch (op) {
      case 0:
        Serial.println("Return to Home position");
        break;
      case 1:
        Serial.println("Pick number from index 0");
        break;
      case 2:
        Serial.println("Pick number from index 1");
        break;
      case 3:
        Serial.println("Pick number from index 2");
        break;
      case 4:
        Serial.println("Pick number from index 3");
        break;
      case 5:
        Serial.println("Pick number from presented Tray");
        break;
      case 6:
        Serial.println("Place number at index 0");
        break;
      case 7:
        Serial.println("Place number at index 1");
        break;
      case 8:
        Serial.println("Place number at index 2");
        break;
      case 9:
        Serial.println("Place number at index 3");
        break;
      case 10:
        Serial.println("Place number on presented Tray");
        break;
      default:
        Serial.println("Unknown Operation");
        break;
    }
  }
}
