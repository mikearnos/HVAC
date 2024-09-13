#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_functions.h"
#include "decode_led.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

void sendErrorCode(void);

void setup()
{
    pinMode(LED, INPUT);

    Serial.begin(115200);

    attachInterrupt(digitalPinToInterrupt(LED), ledChange, CHANGE);

    ledChange(); // sets things up to detect system normal on startup

    //errorCode = 34;
    //sendErrorCode();
}

void sendErrorCode()
{
    // connect to WiFi
    float connectionTimeSeconds = connectWifi();
    if (connectionTimeSeconds)
        Serial.printf("Connected in %.2f seconds\n", connectionTimeSeconds);
    else
        Serial.println("Could not connect\n");

    // connect to MQTT
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    while (!mqtt.connected()) {
        Serial.println("Connecting to MQTT...");
        if (mqtt.connect(CLIENT_ID)) {
            Serial.println("Connected");
        } else {
            Serial.print("Failed with state ");
            Serial.println(mqtt.state());
            delay(2000);
        }
    }

    // create JSON
    StaticJsonDocument<128> doc;
    if (errorCode) {
        doc["status"] = String("ERROR");
    } else {
        doc["status"] = String("OK");
    }
    doc["code"] = errorCode;
    doc["connect"] = connectionTimeSeconds; // already 2 decimal places

    char jsonBuf[128];
    size_t n = serializeJson(doc, jsonBuf);

    // send MQTT
    mqtt.publish(MQTT_TOPIC "/message", (uint8_t*)jsonBuf, (unsigned int)n);
    delay(100); //wait for data to be published (2ms works).

    Serial.println("Disconnecting from WiFi\n");
    disconnectWiFi();
}

void loop()
{
    decodeLED();
}
