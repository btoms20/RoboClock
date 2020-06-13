#ifndef WifiTime_h
#define WifiTime_h

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <RTCZero.h>

class WifiTime {
  public:
    WifiTime();
    void initialize();
    RTCZero rtc;
    int timezoneOffset;

    void printTime();
    void printDate();
    int currentHour();
    int currentMinute();
    
  private:
    int _keyIndex;
    int _status;
    //char _ssid[];
    //char _pass[];
    int DSTStartDay(int y);
    int DSTEndDay(int y);
    int daysInMonth(int m);
    int daysToDate(int m, int d);
    int getAdjustedHours(int utcOffset);
    //int getPSTHours();
    void print2digits(int number);
    void printWiFiStatus();
};

#endif
