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

// Data collection settings
const int collectionTime = 5000;  // Timeframe for data collection in milliseconds (5 seconds)
const int totalEntries = 5;      // Collect exactly 5 entries in each timeframe
const int dataInterval = collectionTime / totalEntries; // Interval between each data point
unsigned long startTime;

// Collision and batch ID
int is_collision = 0;             // Initialize as no collision
unsigned long batch_id;           // Unique batch ID for each 5-second timeframe

WiFiClient client;

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

  // Connect to Wi-Fi
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Still connecting...");
  }
  Serial.print("Connected to Wi-Fi. IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize random seed for batch_id generation
  randomSeed(analogRead(0));
}

void loop() {
  // Start a 3-second countdown before starting data collection
  Serial.println("\nStarting countdown:");
  for (int i = 3; i > 0; i--) {
    Serial.print(i);
    Serial.println("...");
    delay(1000);  // Wait for 1 second before printing the next number
  }

  // Generate a new batch ID for the current 5-second timeframe
  batch_id = random(100000, 999999);
  Serial.print("\nNew Batch ID: ");
  Serial.println(batch_id);

  // Start data collection for the current batch
  Serial.println("Starting data collection...");
  String dataBuffer[totalEntries];  // Array to store exactly 5 data entries
  int dataCount = 0;                // Count of collected data points
  startTime = millis();             // Mark the start time of the collection

  // Collect exactly 5 data points in 5 seconds
  while (dataCount < totalEntries) {
    // Get accelerometer event
    sensors_event_t event;
    accel.getEvent(&event);

    // Get accelerometer data
    float x = event.acceleration.x;
    float y = event.acceleration.y;
    float z = event.acceleration.z;

    // Calculate total acceleration (magnitude)
    float totalAcceleration = sqrt(x * x + y * y + z * z);

    // Check for collision (if total acceleration exceeds threshold)
    is_collision = (totalAcceleration > 15.0) ? 1 : 0; // Adjust threshold as needed

    // Add data to the buffer
    dataBuffer[dataCount] = String(x) + "," + String(y) + "," + String(z) + "," +
                            String(is_collision) + "," + String(batch_id);
    dataCount++;

    // Print collected data entry to serial monitor
    Serial.print("Collected Entry ");
    Serial.print(dataCount);
    Serial.print(": ");
    Serial.println(dataBuffer[dataCount - 1]);

    // Wait for the next data point collection
    delay(dataInterval);
  }

  // Upload all collected data points from the buffer to ThingSpeak
  Serial.println("\nUploading data to ThingSpeak...");
  for (int i = 0; i < totalEntries; i++) {
    // Progress indicator
    Serial.print("Uploading ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.println(totalEntries);

    if (client.connect(server, 80)) {
      // Parse the buffered data
      String dataEntry = dataBuffer[i];
      int commaIndex1 = dataEntry.indexOf(",");
      int commaIndex2 = dataEntry.indexOf(",", commaIndex1 + 1);
      int commaIndex3 = dataEntry.indexOf(",", commaIndex2 + 1);
      int commaIndex4 = dataEntry.indexOf(",", commaIndex3 + 1);

      String x = dataEntry.substring(0, commaIndex1);
      String y = dataEntry.substring(commaIndex1 + 1, commaIndex2);
      String z = dataEntry.substring(commaIndex2 + 1, commaIndex3);
      String collision = dataEntry.substring(commaIndex3 + 1, commaIndex4);
      String batchID = dataEntry.substring(commaIndex4 + 1);

      // Format the POST data
      String postData = String("api_key=") + apiKey +
                        "&field1=" + x +
                        "&field2=" + y +
                        "&field3=" + z +
                        "&field4=" + collision +
                        "&field5=" + batchID;

      // Send HTTP POST request
      client.println("POST /update HTTP/1.1");
      client.println("Host: api.thingspeak.com");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("Content-Length: " + String(postData.length()));
      client.println();
      client.println(postData);

      // Wait for HTTP response (no detailed logging for each response)
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {  // Timeout after 5 seconds
          Serial.println("Error: Timeout waiting for HTTP response.");
          client.stop();
          break;
        }
      }

      // Read and ignore the response
      while (client.available()) {
        client.readString();  // Discard the response to reduce logging
      }

      client.stop();
      Serial.println("Upload successful.");
    } else {
      Serial.println("Error: Failed to connect to ThingSpeak server.");
    }

    // Respect ThingSpeak's rate limit (1 update every 15 seconds per channel)
    delay(15000);
  }

  // Delay before collecting the next batch
  Serial.println("Waiting before starting the next batch...");
  delay(1000);
}