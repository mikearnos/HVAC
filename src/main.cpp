#include <Arduino.h>
//#include <ESP8266WiFi.h>
//#include "../../../wifi.h"

#define DATA D5
#define INVERTED 0 // LM393 IR detector ouputs low when light detected

//const char* ssid = STASSID;
//const char* password = STAPSK;

bool ledStatus = 0, ledChanged = 0;
bool codeStart, codePause, codeFail, readShortPulses, readLongPulses;
unsigned long ledOnStart = 0, ledOffStart = 0, ledOnDuration, ledOffDuration;
int errorCode;

unsigned long last_print_time = millis();

void decodeLED(void);
void waitForLEDOff(void);
int readLED(void);
void pulseLong(void);
void pulseShort(void);
void codeBegin(void);
void nextDigit(void);

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
    errorCode = 0;

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
        if (ledStatus) {
            if (ledOffDuration > 2450) {
                codeBegin();
            } else if (ledOffDuration > 950) {
                nextDigit();
            }
        } else {
            ledOnDuration += 250; // adjustment to account for pause
            if (ledOnDuration > 1200) {
                pulseLong();
            } else if (ledOnDuration > 450) {
                pulseShort();
            }
        }

        ledChanged = 0;
    }
}

void codeBegin()
{
    //Serial.printf("Begin code 2500 (%lu)\n", ledOffDuration);

    // print last completed code
    if (codeStart && codePause && errorCode && !codeFail) { // code requires a start and a pause
        Serial.printf("\tCode: %d", errorCode);
        codeStart = 0;
    }
    if (codeStart && (!codePause || codeFail)) {
        Serial.printf("\tCode: read fail");
        codeStart = 0;
    }
    if (!codeStart) {
        Serial.printf("\nReading code... ");
        codeStart = 1;
        codePause = 0;
        codeFail = 0;
        readShortPulses = 1;
        readLongPulses = 0;
    }
    errorCode = 0;
}

void nextDigit()
{
    //Serial.printf("Next digit 1000 (%lu)\n", ledOffDuration);
    if (codeStart && !codePause) {
        Serial.printf("pause ");
        codePause = 1;
        readShortPulses = 0;
        readLongPulses = 1;
    } else {
        codeFail = 1;
        Serial.printf("pause_err ");
    }
}

void pulseLong()
{
    //Serial.printf("Long 1250 (%lu)\n", ledOnDuration);
    if (codeStart && codePause && readLongPulses && !readShortPulses) {
        Serial.printf("long ");
        errorCode++;
    } else {
        codeFail = 1;
        Serial.printf("long_err ");
    }
}

void pulseShort()
{
    //Serial.printf("Short 500 (%lu)\n", ledOnDuration);
    if (codeStart && !codePause && readShortPulses && !readLongPulses) {
        Serial.printf("short ");
        errorCode += 10;
    } else {
        codeFail = 1;
        Serial.printf("short_err ");
    }
}

int readLED()
{
    if (INVERTED)
        return digitalRead(DATA) ^ 1;
    else
        return digitalRead(DATA);
}