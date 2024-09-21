#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_functions.h"
#include "decode_led.h"
#include <wire.h>

#include <WarmCat_6x14Backpack.h>
WarmCat6x14 myDisp(1);

WiFiClient espClient;
PubSubClient mqtt(espClient);

#define WIFI_THROTTLE_TIME 1000 * 60 * 4

bool codeFirstSend[3] = { 0, 0, 0 };
unsigned long codeSentLast[3];

void sendStatus(int);

void setup()
{
    Serial.begin(115200);

    myDisp.begin();
    myDisp.setBrightness(2);

    myDisp.swirly(40);
    myDisp.clear();

    pinMode(LED, INPUT);
    ledStatus = digitalRead(LED) ^ 1;
    ledChange(); // sets things up to detect system normal on startup

    systemStatus = -1;

    attachInterrupt(digitalPinToInterrupt(LED), ledChange, CHANGE);

    codeSentLast[STATUS_OFF] = -WIFI_THROTTLE_TIME;
    codeSentLast[STATUS_NORMAL] = -WIFI_THROTTLE_TIME;
    codeSentLast[STATUS_ERROR] = -WIFI_THROTTLE_TIME;

    /*unsigned long value = -1;
    Serial.printf("value = %lu\n", value);
    value = value - 1;
    Serial.printf("value - 1 = %lu\n", value);*/
}

void loop()
{
    decodeLED();

    delay(1); // let the ESP do its thing, otherwise it can interfere with the timings
}

void sendStatus(int newStatus)
{
    if (newStatus == systemStatus) // only transmit when status changes
        return;

    systemStatus = newStatus;

    if (millis() - codeSentLast[newStatus] > WIFI_THROTTLE_TIME) { // send each status once per 10 minutes
        //Serial.printf("Status: %d, now: %lu, SentLast %lu\n", newStatus, millis(), codeSentLast[newStatus]);
    } else {
        Serial.printf("Status: %d waiting %lu seconds to resend\n", newStatus,
            (WIFI_THROTTLE_TIME / 1000) - (millis() - codeSentLast[newStatus]) / 1000);
        return;
    }

    // connect to WiFi
    float connectionTimeSeconds = connectWifi();
    if (connectionTimeSeconds) {
        //Serial.printf("Connected in %.2f seconds\n", connectionTimeSeconds);
    } else
        Serial.println("Could not connect\n");

    // connect to MQTT
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    while (!mqtt.connected()) {
        Serial.print("Connecting to MQTT... ");
        if (mqtt.connect(CLIENT_ID)) {
            Serial.println("Connected");
        } else {
            Serial.print("MQTT connect failed with state ");
            Serial.println(mqtt.state());
            return;
        }
    }

    // create JSON
    StaticJsonDocument<128> doc;
    if (systemStatus == STATUS_NORMAL)
        doc["status"] = String("OK");
    else if (systemStatus == STATUS_OFF)
        doc["status"] = String("OFF");
    else if (errorCode) {
        doc["status"] = String("ERROR");
        doc["code"] = errorCode;
    }
    doc["connect"] = connectionTimeSeconds; // already 2 decimal places

    char jsonBuf[128];
    size_t n = serializeJson(doc, jsonBuf);

    // send MQTT
    mqtt.publish(MQTT_TOPIC "/message", (uint8_t*)jsonBuf, (unsigned int)n);
    delay(100); //wait for data to be published (2ms works).

    Serial.printf("MQTT sent: %s\n", (uint8_t*)jsonBuf);
    mqtt.disconnect(); // required before disconnecting WiFi

    Serial.println("Disconnecting from WiFi\n");
    disconnectWiFi();

    codeSentLast[newStatus] = millis();
    //codeFirstSend[newStatus] = 1;

    // reset flags in case WiFi interrupted a code read
    //ledOnDuration = 0;
    //ledOffDuration = 0;
}
