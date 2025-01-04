#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFi.h>

// WiFi credentials
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";

// ThingSpeak settings
const char* server = "api.thingspeak.com";
const char* apiKey = "YOUR_API_WRITE_KEY";

// ADXL345 setup
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Built-in LED pin
const int ledPin = 13;  // Built-in LED on pin 13
int ledState = HIGH; // LOW; 

WiFiClient client;

void setup() {
  Serial.begin(9600);

  // Initialize ADXL345
  if (!accel.begin()) {
    Serial.println("No ADXL345 detected!");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G);

  // Set LED pin as OUTPUT
  pinMode(ledPin, OUTPUT);

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

  digitalWrite(ledPin, ledState);

  // Check if the built-in LED is ON or OFF to simulate button press
  int is_collision = (digitalRead(ledPin) == HIGH) ? 1 : 0;  // LED ON = collision (is_collision = 1)

  // Collect accelerometer data
  float x = event.acceleration.x;
  float y = event.acceleration.y;
  float z = event.acceleration.z;

  // Upload data to ThingSpeak
  if (client.connect(server, 80)) {
    String postData = String("api_key=") + apiKey +
                      "&field1=" + String(x) +
                      "&field2=" + String(y) +
                      "&field3=" + String(z) +
                      "&field4=" + String(is_collision);

    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(postData.length()));
    client.println();
    client.println(postData);
    client.stop();
  }

  delay(1000);  // Collect data every second
}