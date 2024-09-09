#include <Arduino.h>
//#include <ESP8266WiFi.h>
//#include "../../../wifi.h"

#define DATA D5
#define INVERTED 1

//const char* ssid = STASSID;
//const char* password = STAPSK;

bool ledStatus = 0;
unsigned long ledOnStart, ledOffStart = 0, ledOffDuration;

unsigned long last_print_time = millis();

void decodeLED(void);
void waitForLEDOff(void);

IRAM_ATTR void ledOn()
{
    if (ledStatus)
        return;

    ledStatus = 1;
    ledOnStart = millis();

    ledOffDuration = millis() - ledOffStart; // record duration LED was off
}

void setup()
{
    pinMode(DATA, INPUT);

    Serial.begin(115200);

    // LM393 IR detector ouputs high when light detected
    if (INVERTED)
        attachInterrupt(digitalPinToInterrupt(DATA), ledOn, FALLING);
    else
        attachInterrupt(digitalPinToInterrupt(DATA), ledOn, RISING);
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
    if (ledStatus) {
        waitForLEDOff();
        ledStatus = 0;
        ledOffStart = millis();

        ledOffDuration += 10; // small adjustment to make it more accurate
        if (ledOffDuration > 2450) {
            Serial.printf("Begin code 2500 (%lu)\n", ledOffDuration);
        } else if (ledOffDuration > 950) {
            Serial.printf("Next digit 1000 (%lu)\n", ledOffDuration);
        }

        unsigned long blink = millis() - ledOnStart + 250;
        if (blink > 1200) {
            Serial.printf("Long 1250 (%lu)\n", blink);
        } else if (blink > 450) {
            Serial.printf("Short 500 (%lu)\n", blink);
        }
    }
}

void waitForLEDOff()
{
    if (INVERTED) {
        while (!digitalRead(DATA)) { // wait for data line to go high (high = LED off)
            delay(1);
        }
    } else {
        while (digitalRead(DATA)) { // wait for data line to go low (low = LED off)
            delay(1);
        }
    }
}