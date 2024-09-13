#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_functions.h"
#include "decode_led.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

void sendStatus(int);

void setup()
{
    pinMode(LED, INPUT);

    Serial.begin(115200);

    attachInterrupt(digitalPinToInterrupt(LED), ledChange, CHANGE);

    ledChange(); // sets things up to detect system normal on startup

    //errorCode = 34;
    //sendStatus();
}

void loop()
{
    decodeLED();
}

void sendStatus(int newStatus)
{
    if (newStatus == systemStatus) // only transmit when status changes
        return;

    systemStatus = newStatus;

    //int connectBegin = millis();

    // connect to WiFi
    float connectionTimeSeconds = connectWifi();
    if (connectionTimeSeconds)
        Serial.printf("Connected in %.2f seconds\n", connectionTimeSeconds);
    else
        Serial.println("Could not connect\n");

    // connect to MQTT
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    while (!mqtt.connected()) {
        Serial.print("Connecting to MQTT... ");
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

    Serial.println("Disconnecting from WiFi\n");
    disconnectWiFi();

    codeSent = 1;
    codeSentLast = millis();

    //lastChanged = codeSentLast;
    ledOffStart = codeSentLast;
    ledOnStart = codeSentLast;
}
