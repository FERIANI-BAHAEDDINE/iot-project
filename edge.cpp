#include "EmonLib.h"       // Energy monitor library
#include <WiFi.h>          // WiFi support for ESP32
#include <HTTPClient.h>     // HTTP client for sending data to the server

// WiFi Configuration
const char* ssid = "ESP32-AP";
const char* password = "123456789";

// HTTP server configuration
const char* serverUrl = "http://192.168.4.1/data";

WiFiClient wifiClient;
HTTPClient httpClient;

// Calibration constants
const float vCalibration = 41.5;
const float currCalibration = 0.15;

// EnergyMonitor instance
EnergyMonitor emon;

// Timer for regular updates
unsigned long lastMillis = millis();

// Variables for energy calculation
float kWh = 0.0;

void setup()
{
  Serial.begin(9600);

  emon.voltage(34, vCalibration, 1.7); // Voltage input pin, calibration, phase_shift
  emon.current(35, currCalibration);   // Current input pin, calibration

  connectToWiFi();

  delay(1000);
}

void loop()
{
  sendEnergyDataToServer();
  delay(5000); // Wait for 5 seconds
}

void connectToWiFi()
{
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendEnergyDataToServer()
{
  emon.calcVI(20, 2000); // Calculate voltage, current, and apparent power readings

  // Calculate energy consumed in kWh
  unsigned long currentMillis = millis();
  kWh += emon.apparentPower * (currentMillis - lastMillis) / 3600000000.0;
  lastMillis = currentMillis;

  // Prepare data in JSON format
  String jsonData = "{";
  jsonData += "\"voltage\":" + String(emon.Vrms, 2) + ",";
  jsonData += "\"current\":" + String(emon.Irms, 4) + ",";
  jsonData += "\"power\":" + String(emon.apparentPower, 4) + ",";
  jsonData += "\"energy_consumed\":" + String(kWh, 5);
  jsonData += "}";

  if (WiFi.status() == WL_CONNECTED) {
    httpClient.begin(wifiClient, serverUrl);
    httpClient.addHeader("Content-Type", "application/json");

    int httpResponseCode = httpClient.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.print("Data sent to server, HTTP Response Code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data, HTTP Response Code: ");
      Serial.println(httpResponseCode);
    }

    httpClient.end();
  } else {
    Serial.println("Error: Not connected to WiFi");
  }

  // Output energy data to Serial Monitor
  Serial.print("Vrms: ");
  Serial.print(emon.Vrms, 2);
  Serial.print(" V, Irms: ");
  Serial.print(emon.Irms, 4);
  Serial.print(" A, Power: ");
  Serial.print(emon.apparentPower, 4);
  Serial.print(" W, kWh: ");
  Serial.println(kWh, 5);
}
