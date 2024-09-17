#include <Arduino.h>
#include "wifi_functions.h"
#include "decode_led.h"

bool ledStatus, ledChanged = 0;
bool codeStart, codePause, codeFail;
unsigned long ledOnStart = 0, ledOffStart = 0, ledOnDuration, ledOffDuration, lastChanged;
int errorCode = 0, systemStatus = -1;

extern void sendStatus(int);

IRAM_ATTR void ledChange()
{
    if (ledChanged)
        return;

    ledChanged = 1;
    lastChanged = millis();

    if (digitalRead(LED)) { // off to on
        ledStatus = 1;
        ledOnStart = lastChanged;
        ledOffDuration = millis() - ledOffStart; // record duration LED was off
    } else { // on to off
        ledStatus = 0;
        ledOffStart = lastChanged;
        ledOnDuration = millis() - ledOnStart;
    }
}

void decodeLED()
{
    if (ledChanged) {
        if (ledStatus) { // measure the duration of low pulses
            if (ledOffDuration > 2200) { // code begin is low for 2250ms (repeat adds 250ms)
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

    if ((millis() - lastChanged) > 10000) { // if LED has not changed in 10 seconds
        static unsigned long lastPrint;
        if (millis() - lastPrint > 5000) { // print status every 5 seconds
            if (ledStatus) {
                Serial.printf("System normal\n");
                sendStatus(STATUS_NORMAL);
            } else {
                Serial.printf("System off\n");
                sendStatus(STATUS_OFF);
            }
            lastPrint = millis();
        }
        ledOnStart = millis();
        ledOffStart = millis();
        ledOnDuration = 0;
        ledOffDuration = 0;
        codeStart = 0;
        codePause = 0;
        codeFail = 1;
    }
}

void codeBegin()
{
    // print last completed code
    if (codeStart) {
        if (codePause && errorCode && !codeFail) { // code requires a start and a pause
            Serial.printf("\tCode: %d\n", errorCode);
            sendStatus(STATUS_ERROR);
        } else if (!codePause || codeFail)
            Serial.printf("\tCode: read fail\n");
        codeStart = 0;
    }
    // start reading new code
    if (!codeStart) {
        Serial.printf("↓%lu Reading code... ", ledOffDuration);
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
