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

#define MQTT_THROTTLE_TIME (1000 * 3600 * 12) // 12 hour delay

unsigned long codeSentLast[3];

void sendStatus(int);

void setup()
{
    Serial.begin(115200);

    myDisp.begin();
    myDisp.setBrightness(2);

    myDisp.swirly(40);
    myDisp.clear();

    pinMode(LED, INPUT_PULLUP);
    ledStatus = digitalRead(LED) ^ 1;
    ledChange(); // sets things up to detect system normal on startup

    systemStatus = -1;

    attachInterrupt(digitalPinToInterrupt(LED), ledChange, CHANGE);

    codeSentLast[STATUS_OFF] = -MQTT_THROTTLE_TIME;
    codeSentLast[STATUS_NORMAL] = -MQTT_THROTTLE_TIME;
    codeSentLast[STATUS_ERROR] = -MQTT_THROTTLE_TIME;
}

void loop()
{
    decodeLED();

    delay(1); // let the ESP do its thing, otherwise it can interfere with the timings
}

void printConnectWaitStatus()
{
    const char* statusMessage[] = { "Off", "Normal", "Error" };
    Serial.printf("MQTT throttle code: \"%s\", waiting ", statusMessage[systemStatus]);

    unsigned long seconds = (MQTT_THROTTLE_TIME - (millis() - codeSentLast[systemStatus])) / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    minutes -= hours * 60;
    seconds -= hours * 3600 + minutes * 60;
    Serial.printf("%lu hours, %lu minutes, %lu seconds", hours, minutes, seconds);

    Serial.printf(" to resend\n");
}

void sendStatus(int newStatus)
{
    if (newStatus == systemStatus) // only transmit when status changes
        return;

    systemStatus = newStatus;

    if (millis() - codeSentLast[newStatus] < MQTT_THROTTLE_TIME) {
        // we only want to send MQTT messages every so often
        printConnectWaitStatus();
        return;
    }

    // connect to WiFi
    float connectionTimeSeconds = connectWifi();
    if (!connectionTimeSeconds) {
        Serial.println("Could not connect\n");
        myDisp.blink(0);
        for (int i = 10; i; i--) {
            myDisp.scrollText((char*)" NO ", 200);
            delay(1000);
            myDisp.scrollText((char*)"WIFI", 200);
            delay(1000);
        }
        return;
    }

    // connect to MQTT
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    while (!mqtt.connected()) {
        Serial.print("Connecting to MQTT... ");
        if (mqtt.connect(CLIENT_ID)) {
            Serial.println("Connected");
        } else {
            Serial.print("MQTT connect failed with state ");
            Serial.println(mqtt.state());
            Serial.println("Disconnecting from WiFi\n");
            disconnectWiFi();
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
}
