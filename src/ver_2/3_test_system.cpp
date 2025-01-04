#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ThingSpeak settings
const char* thingSpeakApiKey = "YOUR_THINGSPEAK_API_KEY";
const char* serverName = "http://api.thingspeak.com/channels/YOUR_CHANNEL_ID/bulk_update.json";

// ML model coefficients (to be updated after training)
float modelWeights[3] = {0.0, 0.0, 0.0};
float modelBias = 0.0;

void setup() {
  Serial.begin(9600);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

// Simple logistic regression prediction
bool predictCollision(float x, float y, float z) {
  float result = x * modelWeights[0] + 
                 y * modelWeights[1] + 
                 z * modelWeights[2] + modelBias;
  return (1.0 / (1.0 + exp(-result))) > 0.5;
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    
    // Parse CSV data
    float x, y, z;
    sscanf(data.c_str(), "%f,%f,%f", &x, &y, &z);
    
    // Make prediction
    bool isCollision = predictCollision(x, y, z);
    
    // Prepare JSON for ThingSpeak
    StaticJsonDocument<200> doc;
    JsonObject field = doc.createNestedObject("field1");
    field["x"] = x;
    field["y"] = y;
    field["z"] = z;
    field["is_collision"] = isCollision;
    
    // Send to ThingSpeak
    if(WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");
      
      String jsonString;
      serializeJson(doc, jsonString);
      
      int httpResponseCode = http.POST(jsonString);
      
      if (httpResponseCode > 0) {
        Serial.println("Data uploaded successfully");
      } else {
        Serial.println("Error uploading data");
      }
      
      http.end();
    }
    
    delay(15000); // ThingSpeak rate limit (15 seconds)
  }
}