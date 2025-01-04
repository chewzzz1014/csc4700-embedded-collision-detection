#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";

// ThingSpeak settings
const char* server = "http://api.thingspeak.com/update";
const char* apiKey = "YOUR_API_WRITE_KEY";

// ADXL345 setup
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

WiFiClient client;

void setup() {
  Serial.begin(115200);
  
  // Initialize ADXL345
  if (!accel.begin()) {
    Serial.println("No ADXL345 detected!");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G); // Set range to ±16G
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);

  // Collect accelerometer data
  float x = event.acceleration.x;
  float y = event.acceleration.y;
  float z = event.acceleration.z;

  // Send data to server (which runs the model for prediction)
  String url = "http://your_server_address/predict?x=" + String(x) + "&y=" + String(y) + "&z=" + String(z);
  HTTPClient http;
  
  // Make the HTTP request to the server
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Received prediction: " + payload);
    
    // Send accelerometer data and prediction to ThingSpeak
    String postData = String("api_key=") + apiKey +
                      "&field1=" + String(x) +
                      "&field2=" + String(y) +
                      "&field3=" + String(z) +
                      "&field4=" + payload;  // The server response (prediction)
    http.begin("http://api.thingspeak.com/update");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.POST(postData);
  } else {
    Serial.println("Error in HTTP request");
  }

  http.end();
  delay(1000);  // Collect and send data every second
}