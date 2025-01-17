#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>  

const char *ap_ssid = "ESP32-AP";          
const char *ap_password = "123456789";     
const char *wifi_ssid = "iPhone";          
const char *wifi_password = "12345678";   

WebServer server(80);                      

void handlePostRequest() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");  
    
    Serial.println("Data received: ");
    Serial.println(body);

    HTTPClient httpClient;
    const String externalServerUrl = "http://34.68.246.227:8000/api/energy_data/";

    String postData = body;

    if (WiFi.status() == WL_CONNECTED) {
      httpClient.begin(externalServerUrl);  
      httpClient.addHeader("Content-Type", "application/json");
      httpClient.addHeader("accept", "application/json");

      int httpResponseCode = httpClient.POST(postData);  

      if (httpResponseCode > 0) {
        Serial.print("Response from external server: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Error sending data to external server: ");
        Serial.println(httpResponseCode);
      }

      httpClient.end();
    } else {
      Serial.println("Error: Not connected to WiFi");
      server.send(400, "text/plain", "Not connected to WiFi");
      return;
    }

    server.send(200, "text/plain", "Data received and forwarded successfully");
  } else {
    Serial.println("Error: No payload received");
    server.send(400, "text/plain", "No payload received");
  }
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(wifi_ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Connected to external network");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  

  WiFi.softAP(ap_ssid, ap_password);
  IPAddress AP_IP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(AP_IP);  
  
  server.on("/data", HTTP_POST, handlePostRequest);

  server.begin();
}

void loop() {
  server.handleClient();
}
