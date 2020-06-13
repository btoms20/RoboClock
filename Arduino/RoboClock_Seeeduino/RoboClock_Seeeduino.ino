#include <Time.h>
#include <TimeLib.h>

#include "Clock.h"

Clock myClock;
int counter = 0;

//*** Before Uploading ****
//1) Clock Podium should be empty except for a single zero in the last slot
//2) Inventory numbers should be in the following counter clockwise order 
//   (0,0,1,1,1,2,2,3,3,4,4,5,5,5,6,7,8,9) with the first zero placed at the active tray location 
//3) Software serial comms through pins D5 & D6 (this can be changed in Clock.cpp initializer)
//4) 28BYJ motor control via ULN2003 through default pins D8,D9,D10 & D11 (this can be changed in Inventory.cpp initializer) 

void setup() {
  // Manually set the current time (hr,min,sec,day,mnth,yr)
  setTime(13, 10, 20, 8, 4, 20);
  
  Serial.begin(9600);

  delay(100);
  
  Serial.println("Clock Test Ready...");

  //Let the clock know the initial podium state (-1 == empty slot)
  myClock.initClockState(-1, -1, -1, 0);

  delay(1000);
}

void loop() {
  counter++;

  if (counter % 5000 == 0) { //Only check the time once every 5 seconds...
    counter = 0;
    time_t t = now();
    if (minute(t) % 10 == 4) { //begin update
      int hour12 = hour(t) % 12;
      if (hour12 == 0) { hour12 = 12; }
      Serial.print("Setting the time to ");
      Serial.print(hour12);
      Serial.print(":");
      Serial.println(minute(t) + 1);
      myClock.setTime(hour12, minute(t)+1);
    } else if (minute(t) % 10 == 9) { //begin update
      int hour12 = hour(t);
      if (minute(t) == 59) {
        //Next Hour...
        hour12 += 1;
      }
      hour12 = hour12 % 12;
      if (hour12 == 0) { hour12 = 12; }
      int minutes = (minute(t) + 1) % 60;
      Serial.print("Setting the time to ");
      Serial.print(hour12);
      Serial.print(":");
      Serial.println(minutes);
      myClock.setTime(hour12, minutes);
    }
  }
  
  myClock.run();
}
