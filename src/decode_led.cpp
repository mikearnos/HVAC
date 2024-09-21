#include <Arduino.h>
#include "wifi_functions.h"
#include "decode_led.h"
#include <WarmCat_6x14Backpack.h>

bool ledStatus, ledChanged = 0;
bool codeStart = 0, codePause, codeFail;
unsigned long ledOnStart = 0, ledOffStart = 0, ledOnDuration, ledOffDuration, ledLastChanged;
int errorCode = 0, systemStatus = -1;

extern void sendStatus(int);
extern WarmCat6x14 myDisp;

IRAM_ATTR void ledChange()
{
    if (ledChanged)
        return;

    ledChanged = 1;
    ledLastChanged = millis();

    if (digitalRead(LED)) { // off to on
        ledStatus = 1;
        ledOnStart = ledLastChanged;
        ledOffDuration = millis() - ledOffStart; // record duration LED was off
    } else { // on to off
        ledStatus = 0;
        ledOffStart = ledLastChanged;
        ledOnDuration = millis() - ledOnStart;
    }
}

bool around(unsigned long duration, int timing)
{
    unsigned int range = 50; // 50ms plus or minus
    if (duration >= (timing - range) && duration <= (timing + range))
        return 1;
    return 0;
}

void printCode()
{
    char message[4 + 1];
    Serial.printf("\tCode: %d\n", errorCode);
    if (errorCode >= 0 && errorCode <= 99) {
        sprintf(message, " %d ", errorCode);
        myDisp.blink(0);
        myDisp.disp4Char(message, 0);
        myDisp.blink(3);
        sendStatus(STATUS_ERROR);
    }
}

void printNormal()
{
    Serial.printf("System normal\n");
    myDisp.blink(0);
    myDisp.scrollText((char*)" OK ", 200);
    sendStatus(STATUS_NORMAL);
}

void printOff()
{
    Serial.printf("System off\n");
    myDisp.blink(0);
    myDisp.scrollText((char*)" OFF", 200);
    sendStatus(STATUS_OFF);
}

void decodeLED()
{
    if (ledChanged) {
        if (ledStatus) { // measure the duration of low pulses
            if (ledOffDuration > 2200) { // code begin is low for 2250ms (repeat adds 250ms)
                codeBegin();
            } else if (around(ledOffDuration, 1000)) { // pause between digits is low for 1000ms
                nextDigit();
            }
        } else { // measure duration of high pulses
            if (around(ledOnDuration, 1000)) { // long pulse is high 1000ms
                pulseLong();
            } else if (around(ledOnDuration, 250)) { // short pulse is high 250ms
                pulseShort();
            }
        }
        ledChanged = 0;
    }

    if ((millis() - ledLastChanged) > 3500) { // if LED has not changed in 3.5 seconds
        static unsigned long lastPrint;
        if (millis() - lastPrint > 5000) { // print status every 5 seconds
            if (ledStatus) {
                printNormal();
            } else {
                printOff();
            }
            lastPrint = millis();
        }
        ledLastChanged = millis();
        //ledOnStart = ledLastChanged;
        //ledOffStart = ledLastChanged;
        codeStart = 0;
        codePause = 0;
    }

    // check for when a code is complete
    if ((millis() - ledLastChanged) > 300 && !ledStatus) {
        if (codeStart && codePause && !codeFail) {
            if (around(ledOnDuration, 1000) || around(ledOnDuration, 250)) {
                printCode();
                codeStart = 0;
                codePause = 0;
            }
        }
    }
}

void codeBegin()
{
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
