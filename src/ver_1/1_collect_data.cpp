#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFi.h>

// WiFi credentials
const char* ssid = "chewzzz";  
const char* password = "72700cc80790";  

// ThingSpeak settings
const char* server = "api.thingspeak.com";
const char* apiKey = "SSUKBQUP30C1SWX6"; // WRITE API KEY  

// ADXL345 setup
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Built-in LED pin
const int ledPin = 13;  // Built-in LED on pin 13
int ledState = HIGH; 

WiFiClient client;

// Thresholds for detecting collision
const float collisionThreshold = 3.0;  // Adjust based on sensitivity (e.g., 3g)
const float normalGravity = 9.8;       // Approximate value for Earth's gravity

void setup() {
  Serial.begin(9600);

  // Initialize ADXL345
  Serial.println("Initializing ADXL345...");
  if (!accel.begin()) {
    Serial.println("Error: No ADXL345 detected! Check your connections.");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G);
  Serial.println("ADXL345 initialized successfully.");

  // Set LED pin as OUTPUT
  pinMode(ledPin, OUTPUT);

  // Connect to Wi-Fi
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Still connecting...");
  }
  Serial.print("Connected to Wi-Fi. IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Get accelerometer event
  sensors_event_t event;
  accel.getEvent(&event);

  // Log accelerometer data
  float x = event.acceleration.x;
  float y = event.acceleration.y;
  float z = event.acceleration.z;
  Serial.print("Accelerometer Data - X: ");
  Serial.print(x);
  Serial.print(" Y: ");
  Serial.print(y);
  Serial.print(" Z: ");
  Serial.println(z);

  // Determine if a collision occurred based on the acceleration values
  int is_collision = 0;
  float totalAcceleration = sqrt(x * x + y * y + z * z);  // Calculate magnitude of acceleration
  if (abs(totalAcceleration - normalGravity) > collisionThreshold) {
    is_collision = 1;  // Collision detected
    ledState = HIGH;   // Turn on LED for visual feedback
  } else {
    is_collision = 0;  // No collision
    ledState = LOW;    // Turn off LED
  }
  Serial.print("Collision Status: ");
  Serial.println(is_collision);

  // Update LED state
  digitalWrite(ledPin, ledState);

  // Upload data to ThingSpeak
  Serial.println("Preparing data for ThingSpeak...");
  if (client.connect(server, 80)) {
    Serial.println("Connected to ThingSpeak server.");

    String postData = String("api_key=") + apiKey +
                      "&field1=" + String(x) +
                      "&field2=" + String(y) +
                      "&field3=" + String(z) +
                      "&field4=" + String(is_collision);

    // Log the POST data
    Serial.print("POST Data: ");
    Serial.println(postData);

    // Send HTTP POST request
    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(postData.length()));
    client.println();
    client.println(postData);

    // Log HTTP response
    Serial.println("Waiting for HTTP Response...");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {  // Wait for max 5 seconds
        Serial.println("Error: Timeout waiting for HTTP response.");
        client.stop();
        return;
      }
    }

    // Print HTTP response
    Serial.println("HTTP Response:");
    while (client.available()) {
      String response = client.readString();
      Serial.println(response);
    }

    client.stop();
    Serial.println("Data sent to ThingSpeak successfully.");
  } else {
    Serial.println("Error: Failed to connect to ThingSpeak server.");
  }

  // Wait for 1 seconds before next reading
  Serial.println("Waiting 1 seconds before next update...");
  delay(1000);  
}