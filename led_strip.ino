#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "credentials.c"

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
      client.subscribe(MQTT_TOPIC);
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
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
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
