#ifndef Arm_h
#define Arm_h

#include "Arduino.h"
#include "PololuMaestro.h"

class Arm {
  public:
    Arm();
    void initialize();
    bool startSequence(int subroutine);
    void stopSequence();
    bool isWorking();
    void updateSelf();
  private:
    MicroMaestro _maestro;
    bool _isMoving;
    int _updateInterval;
    int _currentSubroutine;
};

#endif
