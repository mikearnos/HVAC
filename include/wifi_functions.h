#include "../../../wifi.h" // local WiFi credentials

#define MQTT_PORT 1883
#define CLIENT_ID "ESP_HVAC"
#define MQTT_TOPIC "furnace"

float connectWifi(void);
float waitForConnection(void);
void disconnectWiFi(void);
