#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "credentials.c"
#include "SingleColor.h"
#include "SinColor.h"
#include "RainbowColor.h"

#define PIN_LED_STRIP D4
#define PIXEL_COUNT 30
const int JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(10);
const char* CMD_ON = "ON";
const char* CMD_OFF = "OFF";

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIN_LED_STRIP, NEO_GRB + NEO_KHZ800);

LedState currentState;
Effect * effect;

void setupWifi() {
  delay(1000);
  Serial.println("----------");
  Serial.println("Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  delay(1000);
  randomSeed(micros());
  printWifiInfo();
}

void printWifiInfo() {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void ensureWifiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    Serial.print("WIFI Disconnected. Attempting reconnection.");
    setupWifi();
  }
}

void ensureMqttConnection() {
  while (!client.connected()) {
    delay(1000);
    Serial.println("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "LedStrip-";
    clientId += String(random(0xffff), HEX);
    Serial.print("Client id: ");
    Serial.println(clientId);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      printWifiInfo();
      Serial.print("Subscribing: ");
      Serial.println(MQTT_SET_TOPIC);
      client.subscribe(MQTT_SET_TOPIC);
      sendCurrentState();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void on_mqtt_message(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  /**
   * Transform this to more normal stuff like array of char not a bytefuck
   */
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (processJson(message)) {
    sendCurrentState();
  }
}

/**
 * Parse out everything about action
 */
boolean processJson(char * rawJson) {
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rawJson);

  if (!root.success()) {
    Serial.println(rawJson);
    Serial.println("json is fucked up!");
    return false;
  }

  currentState.enabled = root["state"] == CMD_ON;

  if (root.containsKey("effect")) {
    LedState transitionState = effect->getCurrentState();
    delete effect;
    Serial.print("Changing effect to");
    if (root["effect"] == "RainbowColor") {
      effect = new RainbowColor();
    } else if (root["effect"] == "SinColor") {
      effect = new SinColor();
    } else {
      Serial.println("Unsuported effect, fallback to default");
      effect = new SingleColor();
    }
    effect->resume(transitionState);
  }

  if (root.containsKey("color")) {
    currentState.red = root["color"]["r"];
    currentState.green = root["color"]["g"];
    currentState.blue = root["color"]["b"];
  }

  if (root.containsKey("brightness")) {
    currentState.brightness = root["brightness"];
  }

  if (currentState.enabled) {
    effect->begin(currentState);
  } else {
    effect->end();
  }
  
  return true;
}

/**
 * Inform mqtt component in home assistant about light state
 */
void sendCurrentState() {
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["state"] = currentState.enabled ? CMD_ON : CMD_OFF;
  JsonObject& color = root.createNestedObject("color");
  color["r"] = currentState.red;
  color["g"] = currentState.green;
  color["b"] = currentState.blue;

  root["brightness"] = currentState.brightness;
  root["effect"] = effect->name();

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));
  Serial.println("Sending current state:");
  Serial.println(buffer);
  client.publish(MQTT_STATE_TOPIC, buffer, true);
}

void updateLeds() {
  if (effect->update(strip)) {
    strip.show();
    delay(33);
  } else {
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  currentState = { 255, 255, 255, 100, false };
  pinMode(BUILTIN_LED, INPUT);
  strip.begin();
  strip.show();
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(on_mqtt_message);
  effect = new SingleColor();
}

void loop() {
  if (client.connected()) {
    client.loop();
    updateLeds();
  } else {
    ensureWifiConnection();
    ensureMqttConnection();
    delay(5000);
  }
}
