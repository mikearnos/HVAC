#include <Arduino.h>
#include "wifi_functions.h"
#include "decode_led.h"
#include <WarmCat_6x14Backpack.h>

bool ledStatus, ledChanged = 0;
bool codeStart = 0, codePause, codeFail, stumble = 0;
unsigned long ledOnStart = 0, ledOffStart = 0, ledOnDuration, ledOffDuration, ledLastChanged;
int errorCode = 0, systemStatus = -1;

extern void sendStatus(int);
extern WarmCat6x14 myDisp;

IRAM_ATTR void ledChange()
{
    ledLastChanged = millis();

    if (ledChanged)
        stumble = 1;

    ledChanged = 1;

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
    unsigned int range = 50; // 50ms plus or minus the input timing value
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
    if (stumble) {
        Serial.printf("took a stumble!\n");
        stumble = 0;
        ledOffDuration = 0;
        ledOnDuration = 0;
        codeFail = 1; // we haven't procesed the last change yet
    }

    if (ledChanged) {
        if (ledStatus) { // measure the duration of low pulses
            if (around(ledOffDuration, 2250) || around(ledOffDuration, 2500)) {
                // code begin is low for 2250ms (repeat adds 250ms)
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

    if ((millis() - ledLastChanged) > 300 && !ledStatus) {
        // last low transition means end of code
        if (around(ledOnDuration, 1000)) { // last pulse was long
            if (codeStart && codePause && !codeFail) { // code was complete
                printCode();
                codeStart = 0;
            }
        }
    }

    if ((millis() - ledLastChanged) > 5000) { // if LED has not changed in 5 seconds
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
        //codeStart = 0;
    }
}

void codeBegin()
{
    // start reading new code
    Serial.printf("↓%lu Reading code... ", ledOffDuration);
    codeStart = 1;
    codePause = 0;
    codeFail = 0;
    errorCode = 0;
}

void nextDigit()
{
    if (codeStart && !codePause && !codeFail) { // check for only 1 pause
        Serial.printf("↓%lu ", ledOffDuration);
    } else { // code hasn't started yet, or pause already happened
        codeFail = 1;
        Serial.printf("pause_err ");
    }
    codePause = 1;
}

void pulseLong()
{
    if (codeStart && codePause & !codeFail) {
        Serial.printf("↑%lu ", ledOnDuration);
        errorCode++;
    } else {
        codeFail = 1;
        Serial.printf("long_err ");
    }
}

void pulseShort()
{
    if (codeStart && !codePause && !codeFail) {
        Serial.printf("↑%lu ", ledOnDuration);
        errorCode += 10;
    } else {
        codeFail = 1;
        Serial.printf("short_err ");
    }
}
