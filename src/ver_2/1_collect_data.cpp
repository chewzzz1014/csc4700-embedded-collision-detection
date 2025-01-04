#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <SoftwareSerial.h>

// Initialize ADXL345
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// ESP32 Serial Communication
SoftwareSerial espSerial(2, 3); // RX, TX

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  
  // Initialize ADXL345
  if(!accel.begin()) {
    Serial.println("No ADXL345 detected!");
    while(1);
  }
  
  // Set range to ±16g
  accel.setRange(ADXL345_RANGE_16_G);
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);
  
  // Format data as CSV
  String dataString = String(event.acceleration.x) + "," +
                     String(event.acceleration.y) + "," +
                     String(event.acceleration.z);
  
  // Send to ESP32
  espSerial.println(dataString);
  
  // Also print to Serial for debugging
  Serial.println(dataString);
  
  delay(100); // 10Hz sampling rate
}