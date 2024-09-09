#include <Arduino.h>
//#include <ESP8266WiFi.h>
//#include "../../../wifi.h"

#define DATA D5

//const char* ssid = STASSID;
//const char* password = STAPSK;

bool ledStatus = 0, ledChanged = 0;
bool codeStart, codePause, codeFail;
unsigned long ledOnStart = 0, ledOffStart = 0, ledOnDuration, ledOffDuration, lastChanged = 0;
int errorCode = 0;

void decodeLED(void);
void pulseLong(void);
void pulseShort(void);
void codeBegin(void);
void nextDigit(void);

IRAM_ATTR void ledChange()
{
    if (ledChanged)
        return;

    lastChanged = millis();
    ledChanged = 1;

    if (!ledStatus && digitalRead(DATA)) { // off to on
        ledStatus = 1;
        ledOnStart = lastChanged;
        ledOffDuration = lastChanged - ledOffStart; // record duration LED was off
        //ledOffDuration = (ledOffDuration + 50) / 250 * 250;
    } else if (ledStatus && !digitalRead(DATA)) { // on to off
        ledStatus = 0;
        ledOffStart = lastChanged;
        ledOnDuration = lastChanged - ledOnStart;
        //ledOnDuration = (ledOnDuration + 50) / 250 * 250;
    }
}

void setup()
{
    pinMode(DATA, INPUT);

    Serial.begin(115200);

    attachInterrupt(digitalPinToInterrupt(DATA), ledChange, CHANGE);
}

void loop()
{
    if (((millis() - lastChanged) > 5000) && digitalRead(DATA) && ledStatus) {
        lastChanged = millis();
        Serial.printf("\nSystem normal");
    }

    decodeLED();
}

void decodeLED()
{
    if (ledChanged) {
        if (ledStatus) { // measure the duration of low pulses
            if (ledOffDuration > 2200) { // begin code is low for 2250ms (repeat adds 250ms)
                codeBegin();
            } else if (ledOffDuration > 950) { // pause between digits is low for 1000ms
                nextDigit();
            }
        } else { // measure duration of high pulses
            if (ledOnDuration > 950) { // long pulse is high 1000ms
                pulseLong();
            } else if (ledOnDuration > 200) { // short pulse is high 250ms
                pulseShort();
            }
        }
        ledChanged = 0;
    }
}

void codeBegin()
{
    // print last completed code
    if (codeStart) {
        if (codePause && errorCode && !codeFail) // code requires a start and a pause
            Serial.printf("\tCode: %d", errorCode);

        if (!codePause || codeFail)
            Serial.printf("\tCode: read fail");
        codeStart = 0;
    }
    // start reading new code
    if (!codeStart) {
        Serial.printf("\n↓%lu Reading code... ", ledOffDuration);
        codeStart = 1;
        codePause = 0;
        codeFail = 0;
    }
    errorCode = 0;
}

void nextDigit()
{
    if (codeStart && !codePause) { // check for only 1 pause
        Serial.printf("↓%lu ", ledOffDuration);
        codePause = 1;
    } else { // code hasn't started yet, or pause already happened
        codeFail = 1;
        Serial.printf("pause_err ");
    }
}

void pulseLong()
{
    if (codeStart && codePause) {
        Serial.printf("↑%lu ", ledOnDuration);
        errorCode++;
    } else {
        codeFail = 1;
        Serial.printf("long_err ");
    }
}

void pulseShort()
{
    if (codeStart && !codePause) {
        Serial.printf("↑%lu ", ledOnDuration);
        errorCode += 10;
    } else {
        codeFail = 1;
        Serial.printf("short_err ");
    }
}
