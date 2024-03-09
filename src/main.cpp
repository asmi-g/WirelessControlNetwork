#include <Arduino.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>

#define WIFI_SSID "wifi_network_name"
#define WIFI_PASSWORD "wifi_password"

#define WS_HOST ""
#define WS_PORT 443
#define WS_URL ""

WiFiMulti wifiMulti;
WebSocketsClient wsClient;

void onWSEvent(WStype_t type, uint8_t * payload, size_t length){
  //handles events w/ switch case statement
  switch(type){
    case WStype_CONNECTED:
      Serial.println("WS Connected");
      break;
    case WStype_DISCONNECTED:
      Serial.println("WS Disconnected");
      break;
    case WStype_TEXT:
      Serial.printf("WS Message: %s\n", payload);
      break;
  }
}

void setup() {
  //connecting to Wifi
  Serial.begin(921600);
  pinMode(LED_BUILTIN, OUTPUT);

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  Serial.println("Connected");

  //beginning WSS protocol for websocket client
  wsClient.beginSSL(WS_HOST, WS_PORT, WS_URL, "","wss");
  wsClient.onEvent(onWSEvent); //watches for and handles events w/ event handler function
}

void loop() {
  digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED);
  wsClient.loop();
}