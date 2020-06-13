#ifndef Arm_h
#define Arm_h

#include "Arduino.h"
#include "PololuMaestro.h"
#include "SoftwareSerial.h"

class Arm {
  public:
    Arm(int rx, int tx);
    void initialize();
    bool startSequence(int subroutine);
    void stopSequence();
    bool isWorking();
    void updateSelf();
  private:
    SoftwareSerial _maestroSerial;
    MicroMaestro _maestro;
    bool _isMoving;
    int _updateInterval;
    int _currentSubroutine;
};

#endif
