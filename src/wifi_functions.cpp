#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "wifi_functions.h"

bool codeSent = 0;
unsigned long codeSentLast;

float connectWifi()
{
    // turn on wifi radio
    WiFi.forceSleepWake();
    delay(1);
    WiFi.mode(WIFI_STA); //This line hides the viewing of ESP as wifi network

    // connect to WiFi
    Serial.printf("Connecting to %s\n", STASSID);
    WiFi.begin(STASSID, STAPSK);

    float connectionTimeSeconds = waitForConnection();

    if (connectionTimeSeconds)
        return connectionTimeSeconds;
    else
        return 0;
}

float waitForConnection()
{
    // returns 0 if not connected, connection time in seconds if connected
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(10);
        if ((counter += 10) >= 10000) { // 10 seconds
            return 0;
        }
    }
    return counter / 1000.0;
}

void disconnectWiFi()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(1);
}
