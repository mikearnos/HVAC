#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "wifi_functions.h"

// The ESP8266 RTC memory is arranged into blocks of 4 bytes.
// The access methods read and write 4 bytes at a time,
// so the RTC data structure should be padded to a 4-byte multiple.
struct {
    uint32_t crc32; // 4 bytes
    uint8_t ap_channel; // 1 byte,   5 in total
    uint8_t ap_mac[6]; // 6 bytes, 11 in total
    uint8_t padding; // 1 byte,  12 in total
} wifiSettings;

float connectWifi()
{
    // returns 0 if not connected, connectionTimeSeconds if connected
    float connectionTimeSeconds;

    // turn on wifi radio
    WiFi.forceSleepWake();
    delay(1);

    WiFi.persistent(false);
    //WiFi.setOutputPower(0.0); // 0 to 20.5 (dBm)

    WiFi.mode(WIFI_STA); //This line hides the viewing of ESP as wifi network

    // prepare static IP address (for faster initialisation)
    IPAddress ip(ESP_STATIC_IP);
    IPAddress gateway(ROUTER_GATEWAY);
    IPAddress subnet(ROUTER_SUBNET);
    WiFi.config(ip, gateway, subnet);

    if (loadRTCData()) {
        // The RTC data was good, make a quick connection
        Serial.printf("Quick connect to %s\n", STASSID);
        WiFi.begin(STASSID, STAPSK, wifiSettings.ap_channel, wifiSettings.ap_mac, true);
        connectionTimeSeconds = waitForConnection();
        if (connectionTimeSeconds) {
            // Quick connect success
            return connectionTimeSeconds;
        }

        // Quick connect is not working, reset WiFi and try regular connection
        WiFi.disconnect();
        delay(10);
        WiFi.forceSleepBegin();
        delay(10);
        WiFi.forceSleepWake();
        delay(10);
    }
    // connect to WiFi
    Serial.printf("RTC invalid, slow connect to %s\n", STASSID);
    WiFi.begin(STASSID, STAPSK);

    connectionTimeSeconds = waitForConnection();

    if (!connectionTimeSeconds)
        return 0;

    // Write current connection info back to RTC
    saveRTCData();

    return connectionTimeSeconds;
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
    delay(10);
    WiFi.mode(WIFI_OFF);
    delay(10);
    WiFi.forceSleepBegin();
    delay(10);
}

bool loadRTCData()
{
    // reads RTC data into rtcData struct
    // returns true if crc32 matches, false if not
    bool rtcValid = false;

    // Try to read WiFi settings from RTC memory
    if (ESP.rtcUserMemoryRead(0, (uint32_t*)&wifiSettings, sizeof(wifiSettings))) {
        // Calculate the CRC of what we just read from RTC memory,
        // but skip the first 4 bytes as that's the checksum itself.
        uint32_t crc = calculateCRC32(((uint8_t*)&wifiSettings) + 4, sizeof(wifiSettings) - 4);
        if (crc == wifiSettings.crc32) {
            rtcValid = true;
        }
    }
    return rtcValid;
}

void saveRTCData()
{
    // Write current connection info back to RTC
    wifiSettings.ap_channel = WiFi.channel();
    memcpy(wifiSettings.ap_mac, WiFi.BSSID(), 6); // Copy 6 bytes of BSSID (AP's MAC address)
    wifiSettings.crc32 = calculateCRC32(((uint8_t*)&wifiSettings) + 4, sizeof(wifiSettings) - 4);
    ESP.rtcUserMemoryWrite(0, (uint32_t*)&wifiSettings, sizeof(wifiSettings));
}

uint32_t calculateCRC32(const uint8_t* data, size_t length)
{
    uint32_t crc = 0xffffffff;
    while (length--) {
        uint8_t c = *data++;
        for (uint32_t i = 0x80; i > 0; i >>= 1) {
            bool bit = crc & 0x80000000;
            if (c & i) {
                bit = !bit;
            }

            crc <<= 1;
            if (bit) {
                crc ^= 0x04c11db7;
            }
        }
    }

    return crc;
}
