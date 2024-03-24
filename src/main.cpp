#include <Arduino.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "wifi_network_name"
#define WIFI_PASSWORD "wifi_password"

#define WS_HOST ""
#define WS_PORT 443
#define WS_URL ""

#define MSG_SIZE 256

WiFiMulti wifiMulti;
WebSocketsClient wsClient;

void sendErrorMessage(const char *error){ 
  //writes error message to client
  char msg[MSG_SIZE];

  sprintf(msg, "{\"action\":\"msg\",\"type\":\"error\",\"body\":\"%s\"}", error);

  wsClient.sendTXT(msg);
}

void validInputMessage(){
  //does the opposite of sendErrorMessage, it validates that user's input was accepted
    wsClient.sendTXT("{\"action\":\"msg\",\"type\":\"status\",\"body\":\"success\"}");
}

uint8_t toMode(const char *val){
  //compares value to three of the given pin modes, return value if any are true
  if(strcmp(val, "output") == 0) {
    return OUTPUT;
  }

  if (strcmp(val, "input_pullup") == 0){
    return INPUT_PULLUP;
  }

  return INPUT;
}

void handleMessage(uint8_t * payload)
{
  JsonDocument doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, payload);

  // Test if parsing succeeds
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    sendErrorMessage(error.c_str());
    return;
  }

  // Perform validation on JSON document
  if(!doc["type"].is<const char *>()){
    sendErrorMessage("Invalid message type format");
    return;
  }

  // Perform validation on JSON document
  if(strcmp(doc["type"], "cmd") == 0){ //use strcmp, or string compare
    // Test if body is valid
    if(!doc["body"].is<JsonObject>()){
      sendErrorMessage("Invalid command body");
      return;
    }

    if(strcmp(doc["body"]["type"], "pinMode") == 0){ //string compare
      // Here, the toMode function just performs validation for the pinMode function
      pinMode(doc["body"]["pin"], toMode(doc["body"]["mode"]));
      validInputMessage();
      return;
    }

    if(strcmp(doc["body"]["type"], "digitalWrite") == 0){ //string compare
      // Handles digitalWrite command
      digitalWrite(doc["body"]["pin"], doc["body"]["value"]);
      validInputMessage();
      return;
    }

    if(strcmp(doc["body"]["type"], "digitalRead") == 0){ //string compare
      // Handles digitalRead command, sends value that was read to client
      auto value = digitalRead(doc["body"]["pin"]);
      char msg[MSG_SIZE];
      sprintf(msg, "{\"action\":\"msg\",\"type\":\"output\",\"body\":%d}", value);
      wsClient.sendTXT(msg);
      return;
    }

    // Handles errors associated with nested if statement for command type
    sendErrorMessage("Unsupported command type");
    return;
  }

  // Handles errors associated with handleMessage function (i.e, no action was done to the LED pin due to the command not being in the codebase)
  sendErrorMessage("Unsupported message type");
  return;
}

void onWSEvent(WStype_t type, uint8_t * payload, size_t length){
  //handles websockets events w/ switch case statement, depending on status of socket
  switch(type){
    case WStype_CONNECTED:
      Serial.println("WS Connected");
      break;
    case WStype_DISCONNECTED:
      Serial.println("WS Disconnected");
      break;
    case WStype_TEXT:
      Serial.printf("WS Message: %s\n", payload);
      handleMessage(payload);
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

  //pulls led pin to HIGH to turn it on if connected to wifi
  digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED);
  wsClient.loop();
}