#include "Arduino.h"
#include "Arm.h"

//Micro Maestro Library
#include "PololuMaestro.h"

#define _maestroSerial SERIAL_PORT_HARDWARE_OPEN
Arm::Arm() : _maestro(_maestroSerial) {
  _isMoving = false;
  _currentSubroutine = -1;
}

void Arm::initialize() {
  //Start comms with the maestro
  _maestroSerial.begin(9600);

  delay(100);

  //Configure Servo Acceleration
  _maestro.setAcceleration(0, 4);
  _maestro.setAcceleration(2, 8);
  _maestro.setAcceleration(3, 8);
  _maestro.setAcceleration(5, 10);

  Serial.println("ARM::Initialized");
}

//Called every loop cycle (should be cheap)
//getScriptStatus == 1 when the script has finished
//getScriptStatus == 0 when the script is running
void Arm::updateSelf() {
  if (_isMoving && (_maestro.getScriptStatus() == 1)) {
    //We just finished a subroutine
    Serial.print("Arm::Finished Subroutine ");
    Serial.println(_currentSubroutine);
    _currentSubroutine = -1;
    _isMoving = false;
    _updateInterval = 0;
  }
}

//Called every loop cycle (should be cheap)
bool Arm::isWorking() {
  return _isMoving;
}

bool Arm::startSequence(int subroutine) {
  if (_maestro.getScriptStatus() == 1) {
    //We're ready to start a new sequence
    Serial.print("Arm::Running SubRoutine ");
    Serial.println(subroutine);
    
    _maestro.restartScript(subroutine);
    _currentSubroutine = subroutine;
    _isMoving = true;
    return true;
  } else {
    return false;
  }
}

void Arm::stopSequence() {
  if (_maestro.getScriptStatus() == 0) {
    Serial.print("Arm::Stopping SubRoutine ");
    Serial.println(_currentSubroutine);
    _maestro.stopScript();
    _currentSubroutine = -1;
    _isMoving = false;
  }
}
