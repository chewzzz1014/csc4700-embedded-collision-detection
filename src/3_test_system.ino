#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>

#define USE_SERIAL Serial

WiFiMulti wifiMulti;

// WiFi credentials
const char* ssid = "Xiaomi 14T";
const char* password = "51522zzwlwlbb";

// Server endpoint configuration
const char* endpoint = "http://192.168.255.45:8000/predict";

const char* apiKey = "Q2NJEC48ZJSUM5GO";

// ADXL345 setup
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Data collection settings
const int collectionTime = 3000;  // Timeframe for data collection in milliseconds (3 seconds)
const int totalEntries = 5;      // Collect exactly 5 entries in each timeframe
const int dataInterval = collectionTime / totalEntries; // Interval between each data point
unsigned long startTime;

// Batch ID and round ID
unsigned long batch_id;
int round_id = 1;

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

  // Test network connectivity
  Serial.println("Testing network connectivity...");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Try to ping the server
  Serial.print("Attempting to connect to server...");
  WiFiClient testClient;
  if (testClient.connect("192.168.255.45", 8000)) {
    Serial.println("Success!");
    testClient.stop();
  } else {
    Serial.println("Failed!");
  }
}

void loop() {
  Serial.print("\nData Collection Round: ");
  Serial.println(round_id);

  int countdownSeconds = 3; // Adjust countdown duration as needed
  Serial.println("Starting data collection in:");
  for (int i = countdownSeconds; i > 0; i--) {
    Serial.print(i);
    Serial.println("...");
    delay(1000); // Wait for 1 second
  }

  // Generate a new batch ID
  batch_id = random(100000, 999999);
  Serial.print("\nNew Batch ID: ");
  Serial.println(batch_id);

  // Collect data for the timeframe
  Serial.println("Starting data collection...");
  String xValues = "[";
  String yValues = "[";
  String zValues = "[";
  String dataBuffer[totalEntries];  // Buffer for collected data
  int dataCount = 0;
  startTime = millis();

  while (dataCount < totalEntries) {
    sensors_event_t event;
    accel.getEvent(&event);

    // Collect accelerometer data
    float x = event.acceleration.x;
    float y = event.acceleration.y;
    float z = event.acceleration.z;

    // Append data to buffers
    xValues += String(x) + (dataCount < totalEntries - 1 ? "," : "");
    yValues += String(y) + (dataCount < totalEntries - 1 ? "," : "");
    zValues += String(z) + (dataCount < totalEntries - 1 ? "," : "");

    // Store raw data
    dataBuffer[dataCount] = String(x) + "," + String(y) + "," + String(z) + "," + String(batch_id);

    Serial.print("Collected Entry ");
    Serial.print(dataCount + 1);
    Serial.print(": ");
    Serial.println(dataBuffer[dataCount]);

    dataCount++;
    delay(dataInterval);
  }

  xValues += "]";
  yValues += "]";
  zValues += "]";

  // Prepare JSON payload
  String jsonPayload = "{\"x\":" + xValues + ",\"y\":" + yValues + ",\"z\":" + zValues + "}";

  // Send data to FastAPI endpoint for prediction
  int is_collision = getPrediction(jsonPayload);

  // Update all entries with the same is_collision value
  for (int i = 0; i < totalEntries; i++) {
    dataBuffer[i] += "," + String(is_collision);
  }

  // Upload data to ThingSpeak
  uploadToThingSpeak(dataBuffer);

  round_id++;

  // Delay before starting the next round
  Serial.println("Waiting before starting the next batch...");
  delay(1000);
}

int getPrediction(String payload) {
    Serial.println("\n--- Starting Prediction Request ---");
    Serial.print("Current WiFi status: ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    
    HTTPClient http;
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    
    Serial.println("Sending payload:");
    Serial.println(payload);
    
    int httpResponseCode = http.POST(payload);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response from server:");
        Serial.println(response);
        return response.toInt();
    } else {
        Serial.print("HTTP request failed with error code: ");
        Serial.println(httpResponseCode);
    }
    
    http.end();
    
    
    return 0;
}

void uploadToThingSpeak(String dataBuffer[]) {
  Serial.println("\nUploading data to ThingSpeak...");
  for (int i = 0; i < totalEntries; i++) {
    // Print the dataBuffer value before uploading
    Serial.print("Data Buffer Entry ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(dataBuffer[i]);

    // Log progress
    Serial.print("Uploading entry ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.println(totalEntries);

    if (client.connect("api.thingspeak.com", 80)) {
      // Parse data entry
      String dataEntry = dataBuffer[i];
      int commaIndex1 = dataEntry.indexOf(",");
      int commaIndex2 = dataEntry.indexOf(",", commaIndex1 + 1);
      int commaIndex3 = dataEntry.indexOf(",", commaIndex2 + 1);
      int commaIndex4 = dataEntry.indexOf(",", commaIndex3 + 1);

      String x = dataEntry.substring(0, commaIndex1);
      String y = dataEntry.substring(commaIndex1 + 1, commaIndex2);
      String z = dataEntry.substring(commaIndex2 + 1, commaIndex3);
      String batchID = dataEntry.substring(commaIndex3 + 1, commaIndex4);
      String collision = dataEntry.substring(commaIndex4 + 1);

      // Format POST data as JSON for better clarity and correctness
      String postData = "{\"api_key\":\"" + String(apiKey) + "\","
                        "\"field1\":" + x + ","
                        "\"field2\":" + y + ","
                        "\"field3\":" + z + ","
                        "\"field4\":" + collision + ","
                        "\"field5\":" + batchID + "}";

      // Print the POST data
      Serial.println("\nPOST Data: ");
      Serial.println(postData);

      // Send HTTP POST request
      client.println("POST /update HTTP/1.1");
      client.println("Host: api.thingspeak.com");
      client.println("Content-Type: application/json");
      client.println("Content-Length: " + String(postData.length()));
      client.println();
      client.println(postData);

      // Log success
      Serial.println("Entry " + String(i + 1) + " uploaded successfully.");
      delay(15000);  // Respect ThingSpeak's rate limit
    } else {
      // Log error
      Serial.println("Error: Failed to connect to ThingSpeak server for entry " + String(i + 1) + ".");
    }
  }
}