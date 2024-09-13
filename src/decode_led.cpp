#include <Arduino.h>
#include "wifi_functions.h"
#include "decode_led.h"

#define STATUS_OFF 0
#define STATUS_NORMAL 1
#define STATUS_ERROR 2

bool ledStatus = 0, ledChanged = 0;
bool codeStart, codePause, codeFail;
unsigned long ledOnStart = 0, ledOffStart = 0, ledOnDuration, ledOffDuration, lastChanged = 0;
int errorCode = 0, systemStatus = 0;

extern void sendStatus(void);

IRAM_ATTR void ledChange()
{
    if (ledChanged)
        return;

    lastChanged = millis();
    ledChanged = 1;

    if (!ledStatus && digitalRead(LED)) { // off to on
        ledStatus = 1;
        ledOnStart = lastChanged;
        ledOffDuration = lastChanged - ledOffStart; // record duration LED was off
    } else if (ledStatus && !digitalRead(LED)) { // on to off
        ledStatus = 0;
        ledOffStart = lastChanged;
        ledOnDuration = lastChanged - ledOnStart;
    }
}

void decodeLED()
{
    if ((millis() - lastChanged) > 5000) { // five seconds since last change
        if ((millis() - ledOnStart) > 15000 && ledStatus) {
            Serial.printf("System normal\n");
            if (systemStatus != STATUS_NORMAL)
                sendStatus();
            systemStatus = STATUS_NORMAL;
        } else if ((millis() - ledOffStart) > 15000 && !ledStatus) {
            Serial.printf("System off\n");
            if (systemStatus != STATUS_OFF)
                sendStatus();
            systemStatus = STATUS_OFF;
        }

        lastChanged = millis();
    }

    if (codeSent && millis() - codeSentLast > 30000) {
        codeSent = 0;
    }

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
}

void codeBegin()
{
    // print last completed code
    if (codeStart) {
        if (codePause && errorCode && !codeFail) { // code requires a start and a pause
            Serial.printf("\tCode: %d\n", errorCode);
            if (systemStatus != STATUS_ERROR)
                sendStatus();
            systemStatus = STATUS_ERROR;
        } else if (!codePause || codeFail)
            Serial.printf("\tCode: read fail\n");
        else
            Serial.printf("newline:\n");
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
