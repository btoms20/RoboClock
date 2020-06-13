
#include "Clock.h"
#include "WifiTime.h"

Clock myClock;
WifiTime wt;
int counter = 0;
bool alarmSet = false;

void setup() {  
  Serial.begin(9600);

  delay(500);

  Serial.println("Initializing Wifi and fetching time...");

  //Init Wifi and Sync RTC Time
  wt.initialize();
  wt.timezoneOffset = -8; //UTC Timezone Offset (-8 = PST, -5 = EST, +2 = Paris, etc) 

  //Init clock state (podium numbers)
  myClock.inventory.usingSensors = true; //Set this to 'true' if you're using the hall effect chain sensors
  myClock.initClockState(-1, -1, -1, 0); //-1 == empty slot

  delay(1000);

  Serial.println("Clock Ready...");

  //Update the clock to the nearest incement of 5
  int first[] = {-1, -1};
  nextIncrementOf(5, first);
  adjustForTimezone(first);
  myClock.setTime(toTwelve(first[0]), first[1]);
  
  Serial.print("Current Time ");
  Serial.print(wt.currentHour());
  Serial.print(":");
  Serial.print(wt.currentMinute());
  Serial.println(" Timezone");

  Serial.print("Clock Set for ");
  Serial.print(first[0]);
  Serial.print(":");
  Serial.print(first[1]);
  Serial.println(" Timezone");
}


void loop() {
  myClock.run();

  if (alarmSet == false) { 
    setNextAlarm();
  }
}

//Grabs the current time / date and sets an alarm to trigger the next clock update (next 5 minute interval)
void setNextAlarm() {
  int next[] = {-1, -1};
  nextIncrementOf(5, next);
  wt.rtc.setAlarmTime(next[0], next[1], 0);
  wt.rtc.enableAlarm(wt.rtc.MATCH_HHMMSS);
  wt.rtc.attachInterrupt(updateClockTime);

  alarmSet = true;
  Serial.print("Alarm Set for ");
  Serial.print(next[0]);
  Serial.print(":");
  Serial.print(next[1]);
  Serial.println(" UTC");
}

//Returns the next increment of UTC time...
//Works in 24 Hour Format
void nextIncrementOf(int n, int *arr) {
  //Get the current time
  int minutes = wt.rtc.getMinutes() + 1;
  int hours = wt.rtc.getHours();
  if (minutes >= (59 - n)) {
    hours++;
    minutes = 0;
  }
  minutes = (minutes + (n-1)) / n * n; //Round up to increment of n
  if (hours >= 24) {
    hours = 0;
  }
  arr[0] = hours;
  arr[1] = minutes;
}

//Works in 24 Hour Format
void adjustForTimezone(int *arr) {
  if (arr[0] == -1 || arr[1] == -1) { return; }
  arr[0] = wt.currentHour();
  if (arr[1] == 0) {
    arr[0] += 1;
    if (arr[0] >= 24) {
      arr[0] = 0;
    }
  }
}

//Converts a 24 Hour Format digit into 12 Hour Format...
int toTwelve(int hour) {
  int h = hour % 12;
  if (h == 0) { return 12; }
  return h;
}

void updateClockTime() {
  //Set Next Alarm...
  alarmSet = false;
  
  //Update Clock...
  Serial.println("Updating Clock...");
  wt.printDate();
  Serial.print(wt.currentHour());
  Serial.print(":");
  Serial.print(wt.currentMinute());
  Serial.println(" Timezone");

  myClock.setTime(wt.currentHour(), wt.currentMinute());
}
