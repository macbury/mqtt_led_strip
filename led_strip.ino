#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "credentials.c"

const int JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(10);
const char* CMD_ON = "ON";
const char* CMD_OFF = "OFF";

WiFiClient espClient;
PubSubClient client(espClient);

void setupWifi() {
  delay(10);
  Serial.println("Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void ensureWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }
}

void ensureMqttConnection() {
  //TODO send current state off and turn leds off if no connection.
  
  while (!client.connected()) {
    delay(1000);
    Serial.println("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "LedStrip-";
    clientId += String(random(0xffff), HEX);
    Serial.print("Client id: ");
    Serial.println(clientId);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe(MQTT_SET_TOPIC);
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

  processJson(message);
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

  boolean enabled = root["state"] == CMD_ON;
  if (enabled) {
    digitalWrite(BUILTIN_LED, LOW); 
  } else {
    digitalWrite(BUILTIN_LED, HIGH);
  }
  return true;
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);
  ensureWiFiConnection();
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(on_mqtt_message);
}

void loop() {
  ensureWiFiConnection();
  ensureMqttConnection();
  if (client.connected()) {
    client.loop();
  }
}
