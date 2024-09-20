#include "../../../wifi.h" // local WiFi credentials

#define ESP_STATIC_IP 192, 168, 1, 61
#define MQTT_PORT 1883
#define CLIENT_ID "ESP_HVAC"
#define MQTT_TOPIC "furnace"

float connectWifi(void);
float waitForConnection(void);
void disconnectWiFi(void);

uint32_t calculateCRC32(const uint8_t*, size_t);
bool loadRTCData(void);
void saveRTCData(void);

extern bool codeSent[];
extern unsigned long codeSentLast[];
