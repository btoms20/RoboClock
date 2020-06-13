#include "WifiTime.h"

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <RTCZero.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

RTCZero rtc;

WifiTime::WifiTime() {
  //Public Variables
  timezoneOffset = -8; //Default to PST

  //Private Variables
  _keyIndex = 0;
  _status = WL_IDLE_STATUS;

  //_ssid[] = SECRET_SSID;    // your network SSID (name)
  //_pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
} 

//Doesn't Support Custom Initialization Orders Yet...
void WifiTime::initialize() {
  // check if the WiFi module works
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( _status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    _status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the status:
  printWiFiStatus();

  rtc.begin();

  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!");
    while (1);
  }
  else {
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);

    Serial.println();
  }

  //Print the current Date / Time
  printDate();
  printTime();
}

void WifiTime::printTime()
{
  print2digits(rtc.getHours());
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.print(":");
  print2digits(rtc.getSeconds());
  Serial.println();
}

void WifiTime::printDate()
{
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear());

  Serial.print(" ");
}

int WifiTime::currentHour() {
  return getAdjustedHours(timezoneOffset);
  //return getPSTHours();
}

int WifiTime::currentMinute() {
  return rtc.getMinutes();
}

void WifiTime::printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void WifiTime::print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

//Accurate until 2025...
//Returns the hour adjusted for Timezone taking into account daylight savings...
int WifiTime::getAdjustedHours(int utcOffset) {
  int y = rtc.getYear();
  int h = rtc.getHours();
  int currentDaysToDate = daysToDate(rtc.getMonth(), rtc.getDay()); 
  int dstStart = DSTStartDay(y);
  int dstEnd = DSTEndDay(y); 
  //Daylight Savings time starts Sunday, March 8, 2020 @ 2am and ends Sunday, November 1, 2020 @ 2am
  if (currentDaysToDate >= dstStart && currentDaysToDate <= dstEnd) {
    //We're probably in DST...
    if (currentDaysToDate == dstStart) { //Check Hours
      if (rtc.getHours() >= 18) { 
        utcOffset += 1; //We're in daylights saving time...
      } 
    } else if (currentDaysToDate == dstEnd) { //Check Hours
      if (rtc.getHours() <= 18) { 
        utcOffset += 1; //We're in daylights saving time...
      }
    } else {
      utcOffset += 1; //We're in daylights saving time...
    }
  }
  h = h % 12;
  h += utcOffset;
  if (h <= 0) {
    return 12 + h;
  }
  return h;
}

int WifiTime::daysToDate(int m, int d) {
  int totalDays = 0;
  for (int i = 1; i < m; i++) {
    totalDays += daysInMonth(i);
  }
  totalDays += d;
  return totalDays;
}

int WifiTime::daysInMonth(int m) {
  switch (m) {
    case 1:
      return 31;
    case 2:
      if (rtc.getYear() % 4 == 0) {
        return 29;
      }
      return 28;
    case 3:
      return 31;
    case 4:
      return 30;
    case 5:
      return 31;
    case 6:
      return 30;
    case 7:
      return 31;
    case 8:
      return 31;
    case 9:
      return 30;
    case 10:
      return 31;
    case 11:
      return 30;
    case 12:
      return 31;
    default:
      return 0;
  }
}

//Returns the DST start date in total ytd days...
int WifiTime::DSTStartDay(int y) {
  switch (y) {
    case 20:
      return daysToDate(3, 8);  //March 8th
    case 21:
      return daysToDate(3, 14); //March 14th
    case 22:
      return daysToDate(3, 13); //March 13th
    case 23:
      return daysToDate(3, 12); //March 12th
    case 24:
      return daysToDate(3, 10); //March 10th
    case 25:
      return daysToDate(3, 9);  //March 9th
    default:
      return 0;
  }
}

//Returns the DST end date in total ytd days...
int WifiTime::DSTEndDay(int y) {
  switch (y) {
    case 20:
      return daysToDate(11, 8); //November 8th 
    case 21:
      return daysToDate(11, 7); //November 7th
    case 22:
      return daysToDate(11, 6); //November 6th
    case 23:
      return daysToDate(11, 5); //November 5th
    case 24:
      return daysToDate(11, 3); //November 3rd
    case 25:
      return daysToDate(11, 2); //November 2nd
    default:
      return 0;
  }
}
