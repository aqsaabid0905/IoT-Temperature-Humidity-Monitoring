#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

#define DHT_PIN 15
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

// Wokwi Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ThingSpeak
const char* server = "api.thingspeak.com";
const char* writeAPIKey = "F2WWJUI8A0D3OWRH";

WiFiClientSecure client;

void setup() {
  Serial.begin(115200);

  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // For Wokwi simulation
  client.setInsecure();

  Serial.println("System Ready!");
}

void loop() {

  // Read DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT22!");
    delay(2000);
    return;
  }

  Serial.println();
  Serial.println("===== SENSOR DATA =====");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Connect to ThingSpeak using HTTPS
  Serial.println("Connecting to ThingSpeak...");

  if (client.connect(server, 443)) {

    String url = "/update?api_key=";
    url += writeAPIKey;
    url += "&field1=";
    url += String(temperature);
    url += "&field2=";
    url += String(humidity);

    Serial.println("Sending data...");

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("ThingSpeak response:");

    unsigned long timeout = millis();

    while (client.available() == 0) {

      if (millis() - timeout > 5000) {
        Serial.println("Response timeout!");
        client.stop();
        return;
      }
    }

    while (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }

    client.stop();

    Serial.println("Upload completed!");

  } else {

    Serial.println("Connection to ThingSpeak failed!");

  }

  Serial.println("=======================");
  Serial.println("Waiting 15 seconds...");

  delay(15000);
}
