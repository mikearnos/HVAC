#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "../../../wifi.h"
#include "wifi_functions.h"

void disconnectWiFi()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(1);
}

float connectWifi()
{
    // turn on wifi radio
    WiFi.forceSleepWake();
    delay(1);
    WiFi.mode(WIFI_STA); //This line hides the viewing of ESP as wifi network

    // connect to wifi
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
    int16_t counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(10);
        if ((counter += 10) == 10000) { // 10 seconds
            return 0;
        }
    }
    return counter / 1000.0;
}