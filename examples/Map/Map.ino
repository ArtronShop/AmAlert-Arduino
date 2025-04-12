#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <AmAlert.h>

#define WIFI_SSID "wifi name" // WiFi Name
#define WIFI_PASSWORD "wifi password" // WiFi Password

#define ROOM_TOKEN "your ROOM token" // Room Token

WiFiMulti wifiMulti;

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  // wait for WiFi connection
  Serial.print("Waiting for WiFi to connect...");
  while ((wifiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }
  Serial.println(" connected");

  Alert.begin(ROOM_TOKEN);

  Am_Alert_Massage_Option_t option;

  // Map
  option.map.service = LONGDO_MAP; // Longdo Map
  // option.map.service = GOOGLE_MAP; // Google Map
  option.map.lat = 13.910216086303118; // Latitude 
  option.map.lng = 100.51109150490085; // Longitude
  option.map.zoom = 20; // 1 - 20
  // option.map.api_key = "Google Map API Key"; // needs if use Google Map service

  if (Alert.send("Hello from ESP32 !", &option)) { // Send "Hello from ESP32 !" and asset to Am Alert
    Serial.println("Send notify successful");
  } else {
    Serial.printf("Send notify fail. check your token (code: %d)\n", Alert.status_code);
  }
}

void loop() {
  
}
