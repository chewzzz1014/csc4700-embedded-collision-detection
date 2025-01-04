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

WiFiClient client;

void setup() {
  Serial.begin(9600);

  // Initialize ADXL345
  if (!accel.begin()) {
    Serial.println("No ADXL345 detected!");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G);

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

  // Manually label data
  int is_collision = 0;  // Set this manually (1 for collision, 0 for no collision)

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