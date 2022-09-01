/*
 * Config settings - copy to new file called "config.h" and edit
 */
// Wifi settings
#define WIFI_SSID ""
#define WIFI_PASS ""

// broker IP/host
#define BROKER ""
// broker password or blank
#define BROKER_PASS ""
// base topic to publish on (will append "/<wifi MAC id>/temp")
// status (1 or 0) published as will to topic (TOPIC_ROOT/<wifi MAC id>/status)
#define TOPIC_ROOT "ChController"
// override default timeout from PubSubClient lib
#define MQTT_KEEPALIVE 60

// Relay pins on board
#define RELAY_1_PIN D3
#define RELAY_2_PIN D2

// LED pins on board
#define LED_1_PIN D6
#define LED_2_PIN D7

//Topics to pub/sub to
#define TOPIC_BASE "CH"
#define RELAY_1_TOPIC "Heat"
#define RELAY_2_TOPIC "Pump"
// A topic TOPIC_BASE/status will be report if the devices is connected

//Max time to keep relay on - will auto switch off after this time in case of a software bug never sending an off message
#define MAX_ON_MIN 15
