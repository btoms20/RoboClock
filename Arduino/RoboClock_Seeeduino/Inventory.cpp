#include "Arduino.h"
#include "Inventory.h"

//28BYJ Stepper Motor Library
#include <CheapStepper.h>

// run the stepper at 12rpm (if using 5V power) - the default is ~16 rpm
// stepper.setRpm(12);
// stepper.getRpm()
// stepper.getDelay()

int STEPS_PER_TRAY = 688; //704

// Declare our stepper using default pins:
// arduino pin <--> pins on ULN2003 board:
//  8 <--> IN1
//  9 <--> IN2
// 10 <--> IN3
// 11 <--> IN4
Inventory::Inventory() : _stepper(8, 9, 10, 11) {
  _isMoving = false;
  _currentTray = 0;
  _desiredTray = -1;

  _stepper.setRpm(12);
}

//Doesn't Support Custom Initialization Orders Yet...
void Inventory::initializeState() {
  _currentTray = 0;
  
  _inventoryState[0]  = 0;
  _inventoryState[1]  = 0;
  _inventoryState[2]  = 1;
  _inventoryState[3]  = 1;
  _inventoryState[4]  = 1;
  _inventoryState[5]  = 2;
  _inventoryState[6]  = 2;
  _inventoryState[7]  = 3;
  _inventoryState[8]  = 3;
  _inventoryState[9]  = 4;
  _inventoryState[10] = 4;
  _inventoryState[11] = 5;
  _inventoryState[12] = 5;
  _inventoryState[13] = 5; 
  _inventoryState[14] = 6;
  _inventoryState[15] = 7;
  _inventoryState[16] = 8;
  _inventoryState[17] = 9;

  Serial.println("Inventory:: Initialized");
}

// Returns the current presented number
// 0-9  ->  Number 0-9
//  -1  ->  Empty Tray
//  -2  ->  Moving (no number currently presented)
int Inventory::presentedNumber() {
  if (_isMoving == true) {
    return -2;
  }
  return _inventoryState[_currentTray];
}

bool Inventory::isWorking() {
  return _isMoving;
}

void Inventory::updateSelf() {
  if (_isMoving == true) {
    _stepper.run();
  }
  
  if (_isMoving == true && _stepper.getStepsLeft() == 0) {
    //We're done!
    Serial.print("Inventory::Done Presenting Number '");
    Serial.print(_inventoryState[_desiredTray]);
    Serial.print("' on tray: ");
    Serial.println(_desiredTray);
    _currentTray = _desiredTray;
    _desiredTray = -1;
    _isMoving = false;
  }
}

//Finds the closest `number` in our inventory to our current tray index
//Returns False if we can't satisfy the request (no empty trays, etc) or if we're already busy...
bool Inventory::requestNumber(int number) {
  //We reject the request if we're already working
  if (_isMoving == true) {
    return false;
  }

  bool clockwise = true;
  int traysAway = -1;
  int matchCW = -1;
  int matchCCW = -1;
  //Search Clockwise
  for(int i = 0; i < 18; i++) {
    int idx = (_currentTray + i) % 18;
    if (_inventoryState[idx] == number) {
      matchCW = i;
      break;
    }
  }
  //Search CounterClockwise
  for(int i = 0; i < 18; i++) {
    int idx = (_currentTray - i);
    if (idx < 0) { 
      idx = 18 + idx;
    }
    if (_inventoryState[idx] == number) {
      matchCCW = i;
      break;
    }
  }
  //Find closest requested number to current tray
  if (matchCW != -1 && matchCCW != -1) {
    Serial.println("Found Match For Requested Number");
    Serial.print("Clockwise Match: ");
    Serial.println(matchCW);
    Serial.print("CounterClockwise Match: ");
    Serial.println(matchCCW);
    //Pick the lowest of the two
    if (matchCW <= matchCCW) {
      //Choosing Clockwise Route...
      clockwise = true;
      traysAway = matchCW;
      _desiredTray = (_currentTray + traysAway) % 18;
    } else {
      //Choosing Counter Clockwise Route...
      clockwise = false;
      traysAway = matchCCW;
      _desiredTray = (_currentTray - traysAway);
      if (_desiredTray < 0) {
        _desiredTray = 18 + _desiredTray;
      }
    }
  } else {
    //No Match...
    Serial.print("Failed to find a match for requested number '");
    Serial.print(number);
    Serial.println("' in our inventory");
    Serial.print("Clockwise Match: ");
    Serial.println(matchCW);
    Serial.print("CounterClockwise Match: ");
    Serial.println(matchCCW);
    return false;
  } 
  
  //Generate newMove (cw, ccw) * number of tray / slots
  Serial.print("Moving ");
  if (clockwise == true) {
    Serial.print("clockwise");
  } else {
    Serial.print("counter clockwise");
  }
  Serial.print("by ");
  Serial.print(traysAway);
  Serial.print(" trays (");
  Serial.print(traysAway * STEPS_PER_TRAY);
  Serial.print(" steps) to tray index: ");
  Serial.println(_desiredTray);
  
  _stepper.newMove(clockwise, traysAway * STEPS_PER_TRAY); //512
  _isMoving = true;

  return true;
}

//We can only update our inventory at our presented tray index
// and we only accept updates when we're not moving...
bool Inventory::updateInventory(int number) {
  if (_isMoving) { return false; }
  _inventoryState[_currentTray] = number;
  return true;
}

void Inventory::next() {
  if (_isMoving == false) {
    _stepper.newMove(true, STEPS_PER_TRAY);
    _isMoving = true;
  }
}

void Inventory::previous() {
  if (_isMoving == false) {
    _stepper.newMove(false, STEPS_PER_TRAY);
    _isMoving = true;
  }
}

//12 tooth sprocket values
// 4096 == 360 degrees, 8 trays
// 2048 == 180 degress, 4 trays
// 1024 ==  90 degrees, 2 trays
//  512 ==  45 degrees, 1 tray
