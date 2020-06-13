#include "Arduino.h"
#include "Inventory.h"

//28BYJ Stepper Motor Library
#include <CheapStepper.h>

// run the stepper at 12rpm (if using 5V power) - the default is ~16 rpm
// stepper.setRpm(12);
// stepper.getRpm()
// stepper.getDelay()

bool DEBUG = false;
bool usingSensors = true;

int STEPS_PER_TRAY = 688; //704

//Hall Effect Sensors
int MAG_1_SENSOR = 6;
int MAG_2_SENSOR = 7;
volatile bool _mag1Triggered = true;
volatile bool _mag2Triggered = true;
bool _trayAbove = true;

// Declare our stepper using default pins:
// arduino pin <--> pins on ULN2003 board:
//  2 <--> IN1
//  3 <--> IN2
//  4 <--> IN3
//  5 <--> IN4
Inventory::Inventory() : _stepper(2, 3, 4, 5) {
  _isMoving = false;
  _currentTray = 0;
  _desiredTray = -1;
  _movingDirection = false;

  _stepper.setRpm(16);
  stepperOff();

  //Set up our Hall Effect Sensors
  pinMode(MAG_1_SENSOR, INPUT);
  pinMode(MAG_2_SENSOR, INPUT);
}

//Doesn't Support Custom Initialization Orders Yet...
void Inventory::initializeState() {
  _currentTray = 0;
  _desiredTray = 0;
  
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

  if (DEBUG) { Serial.println("Inventory:: Initialized"); }
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
  if (!_isMoving) { return; }
  
  //Step our motor
  _stepper.run();

  if (usingSensors == true) { 
    //Check our tray sensors
    _mag1Triggered = (digitalRead(MAG_1_SENSOR) == LOW);
    _mag2Triggered = (digitalRead(MAG_2_SENSOR) == LOW);
    
    //Update our _trayAbove variable
    if (_trayAbove && (!_mag1Triggered && !_mag2Triggered)) {
      if (DEBUG) { Serial.println("Tray left presentation area"); }
      _trayAbove = false;
    } else if (!_trayAbove && (_mag1Triggered || _mag2Triggered)) {
      if (DEBUG) { Serial.println("Tray entered presentation area"); }
      _trayAbove = true;
      //Stop our current move
      _stepper.stop();
      _stepper.move(_movingDirection, 10);  //Move an additional few steps to center the tray...
      //Update current tray
      if (_movingDirection) {
        _currentTray += 1;
      } else {
        _currentTray -= 1;
      }
      if (_currentTray < 0) { _currentTray = 17; }
      else if (_currentTray > 17) { _currentTray = 0; }
      if (DEBUG) { 
        Serial.print("Current Tray: ");
        Serial.println(_currentTray);
        Serial.print("Desired Tray: ");
        Serial.println(_desiredTray);
      }
      //Continue moving towards our desired tray...
      if (_desiredTray != _currentTray) {
        _isMoving = false;
        _movingDirection ? next() : previous();
      } else {
        if (DEBUG) { 
          Serial.print("Inventory::Done Presenting Number '");
          Serial.print(_inventoryState[_desiredTray]);
          Serial.print("' on tray: ");
          Serial.println(_desiredTray);
        }
        _isMoving = false;
  
        //Turn off our stepper (to keep it from overheating)
        stepperOff();
      }
      return;
    } 
    
    //If steps left == 0 chain is probably binding...
    if (_stepper.getStepsLeft() == 0) {
      if (DEBUG) { Serial.println("We didn't reach our tray, our chain might be stuck"); }
      _stepper.stop();
      _stepper.move(!_movingDirection, 100);
      _stepper.move(_movingDirection, 100);
      _isMoving = false;
      if(_movingDirection) {
        next();
      } else {
        previous();
      }
    }
  } else { 
    if (_stepper.getStepsLeft() == 0) {
      if (DEBUG) { 
          Serial.print("Inventory::Done Presenting Number '");
          Serial.print(_inventoryState[_desiredTray]);
          Serial.print("' on tray: ");
          Serial.println(_desiredTray);
        }
        _isMoving = false;
        _currentTray = _desiredTray;

        //Turn off our stepper (to keep it from overheating)
        stepperOff();
    }
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
    if (DEBUG) { 
      Serial.println("Found Match For Requested Number");
      Serial.print("Clockwise Match: ");
      Serial.println(matchCW);
      Serial.print("CounterClockwise Match: ");
      Serial.println(matchCCW);
    }
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
    if (DEBUG) { 
      Serial.print("Failed to find a match for requested number '");
      Serial.print(number);
      Serial.println("' in our inventory");
      Serial.print("Clockwise Match: ");
      Serial.println(matchCW);
      Serial.print("CounterClockwise Match: ");
      Serial.println(matchCCW);
    }
   
    return false;
  } 
  
  //Generate newMove (cw, ccw) * number of tray / slots
  //return startNewMove(clockwise, traysAway);
  if (traysAway != 0) {
    if (usingSensors == true) { //Move one tray at a time, counting sensor triggers, until we reach our target tray
      return clockwise ? next() : previous();
    } else { //Move the required number of steps and hope we dont bind / lose steps in the process...
      return startNewMove(clockwise, traysAway);
    }
  } else { //We're already there, no move required
    return true;
  }
}

//We can only update our inventory at our presented tray index
// and we only accept updates when we're not moving...
bool Inventory::updateInventory(int number) {
  if (_isMoving) { return false; }
  _inventoryState[_currentTray] = number;
  return true;
}

bool Inventory::next() {
  startNewMove(true, 1);
}

bool Inventory::previous() {
  startNewMove(false, 1);
}

bool Inventory::startNewMove(bool dir, int trays) { 
  if (_isMoving == true) { return false; }

  //Debug Logging
  if (DEBUG) { 
    Serial.print("Moving ");
    if (dir == true) {
      Serial.print("clockwise");
    } else {
      Serial.print("counter clockwise");
    }
    Serial.print(" by ");
    Serial.print(trays);
    Serial.print(" trays (");
    Serial.print(trays * STEPS_PER_TRAY);
    Serial.print(" steps) to tray index: ");
    Serial.println(_desiredTray);
  }

  if (usingSensors == true) {
    _stepper.newMove(dir, (trays * STEPS_PER_TRAY) + 150); //add additional 150 steps if using sensors to make sure we get there...
  } else { 
    _stepper.newMove(dir, (trays * STEPS_PER_TRAY));
  }
  _isMoving = true;
  _movingDirection = dir;

  return true;
}

void Inventory::stepperOff() {
  for (int p=0; p<4; p++) {
    digitalWrite(_stepper.getPin(p), 0);
  }
}


//12 tooth sprocket values
// 4096 == 360 degrees, 8 trays
// 2048 == 180 degress, 4 trays
// 1024 ==  90 degrees, 2 trays
//  512 ==  45 degrees, 1 tray
