#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "credentials.c"
#include "SingleColor.h"
#include "SinColor.h"
#include "RainbowColor.h"
#include "DualColor.h"
#include "FireEffect.h"

const int JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(10);
const char* CMD_ON = "ON";
const char* CMD_OFF = "OFF";

WiFiClient espClient;
PubSubClient client(espClient);

#include "iot.h"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIN_LED_STRIP, NEO_GRB + NEO_KHZ800);

LedState currentState;
Effect * effect;

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

  if (String(MQTT_RESET_TOPIC) == topic) {
    if (String(message) == CMD_ON) {
      Serial.println("Enable OTA");
      setupOTA();
    } else {
      Serial.println("Reseting ESP!");
      ESP.restart();
    }
    return;
  }

  if (String(MQTT_DEEP_SLEEP_TOPIC) == topic) {
    Serial.println("Starting deep sleep");
    return;
  }

  
  Serial.println(message);

  if (processJson(message)) {
    sendCurrentState();
    return;
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
    if (root["effect"] == "FireEffect") {
      effect = new FireEffect();
    } else if (root["effect"] == "DualColor") {
      effect = new DualColor();
    } else if (root["effect"] == "RainbowColor") {
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
  strip.setBrightness(0);
  strip.show();

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(on_mqtt_message);
  effect = new SingleColor();
}

void onConnect() {
  Serial.print("Subscribing: ");
  Serial.println(MQTT_SET_TOPIC);
  client.subscribe(MQTT_SET_TOPIC);
  Serial.println(MQTT_RESET_TOPIC);
  client.subscribe(MQTT_RESET_TOPIC);
  Serial.println(MQTT_DEEP_SLEEP_TOPIC);
  client.subscribe(MQTT_DEEP_SLEEP_TOPIC);
  sendCurrentState();
}

void loop() {
  if (client.connected()) {
    client.loop();
    ArduinoOTA.handle();
  } else {
    if (ensureMqttConnection()) {
      onConnect();
    }
  }
}
