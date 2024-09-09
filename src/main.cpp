#include <Arduino.h>
//#include <ESP8266WiFi.h>
//#include "../../../wifi.h"

#define DATA D5
#define INVERTED 0 // LM393 IR detector ouputs low when light detected

//const char* ssid = STASSID;
//const char* password = STAPSK;

bool ledStatus = 0, ledChanged = 0;
unsigned long ledOnStart = 0, ledOffStart = 0, ledOnDuration, ledOffDuration;

unsigned long last_print_time = millis();

void decodeLED(void);
void waitForLEDOff(void);
int readLED(void);

IRAM_ATTR void ledChange()
{
    if (ledChanged)
        return;

    ledChanged = 1;

    if (!ledStatus && readLED()) { // off to on
        ledStatus = 1;
        ledOnStart = millis();
        ledOffDuration = millis() - ledOffStart; // record duration LED was off
    } else if (ledStatus && !readLED()) { // on to off
        ledStatus = 0;
        ledOffStart = millis();
        ledOnDuration = millis() - ledOnStart;
    }
}

void setup()
{
    pinMode(DATA, INPUT);
    ledStatus = readLED();

    Serial.begin(115200);

    attachInterrupt(digitalPinToInterrupt(DATA), ledChange, CHANGE);
}

void loop()
{
    // Print every 2 seconds (non-blocking)
    if ((unsigned long)(millis() - last_print_time) > 500) {
        last_print_time = millis();
    }

    decodeLED();

    unsigned long watchDog;
}

void decodeLED()
{
    if (ledChanged) {
        //ledOffDuration += 10; // small adjustment to make it more accurate
        if (ledStatus) {
            if (ledOffDuration > 2450) {
                Serial.printf("Begin code 2500 (%lu)\n", ledOffDuration);
            } else if (ledOffDuration > 950) {
                Serial.printf("Next digit 1000 (%lu)\n", ledOffDuration);
            }
        } else {
            ledOnDuration += 250; // adjustment to make it more accurate
            if (ledOnDuration > 1200) {
                Serial.printf("Long 1250 (%lu)\n", ledOnDuration);
            } else if (ledOnDuration > 450) {
                Serial.printf("Short 500 (%lu)\n", ledOnDuration);
            }
        }

        ledChanged = 0;
    }
}

int readLED()
{
    if (INVERTED)
        return digitalRead(DATA) ^ 1;
    else
        return digitalRead(DATA);
}