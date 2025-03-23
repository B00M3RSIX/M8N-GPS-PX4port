#include <Arduino.h>
#include <TinyGPSPlus.h>

// GPS UART settings
#define GPS_BAUD 38400  // Default baud rate for M8N is 38400

// Debug mode
#define DEBUG_GPS_SENTENCES 1  // Set to 1 to see raw NMEA sentences

// Global variables
TinyGPSPlus gps;
unsigned long last_gps_display = 0;
unsigned long last_serial_check = 0;
uint32_t serial_bytes_received = 0;
uint32_t last_serial_bytes_count = 0;

// Buffer for debug NMEA sentence
char nmea_buffer[100];
uint8_t nmea_idx = 0;

// Function prototypes
void displayGPSData();
void checkSerialData();
void handleNMEASentence(char c);

void setup() {
  // Initialize Serial for USB communication
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for serial to be ready, but timeout after 3 seconds
  
  Serial.println("Holybro M8N GPS Demo");
  
  // Try each of the common baud rates for GPS modules
  Serial.println("Trying to identify GPS baud rate...");
  
  // Initialize GPS serial connection - try a few common baud rates
  uint32_t baud_rates[] = {38400, 9600, 57600, 115200};
  bool gps_detected = false;
  
  for (uint8_t i = 0; i < sizeof(baud_rates)/sizeof(baud_rates[0]); i++) {
    Serial.print("Trying baud rate: ");
    Serial.println(baud_rates[i]);
    
    Serial3.begin(baud_rates[i]);
    delay(1000); // Wait for GPS to send some data
    
    unsigned long start_time = millis();
    uint8_t bytes_received = 0;
    
    // Check for data at this baud rate
    while (millis() - start_time < 1000) {
      if (Serial3.available() > 0) {
        Serial3.read(); // Just discard the data
        bytes_received++;
      }
    }
    
    if (bytes_received > 10) {
      Serial.print("Detected GPS at baud rate: ");
      Serial.println(baud_rates[i]);
      gps_detected = true;
      break;
    }
    
    Serial3.end();
  }
  
  if (!gps_detected) {
    Serial.println("WARNING: Could not detect GPS. Defaulting to 38400 baud.");
    Serial3.begin(GPS_BAUD);
  }
  
  Serial.println("Setup complete. Waiting for GPS data...");
}

void loop() {
  // Read data from GPS
  while (Serial3.available() > 0) {
    char c = Serial3.read();
    
    // Store for debug
    if (DEBUG_GPS_SENTENCES) {
      handleNMEASentence(c);
    }
    
    gps.encode(c);
    serial_bytes_received++;
  }
  
  // Check if we're receiving serial data every 2 seconds
  if (millis() - last_serial_check > 2000) {
    checkSerialData();
    last_serial_check = millis();
  }
  
  // Display GPS data every second
  if (millis() - last_gps_display > 1000) {
    displayGPSData();
    last_gps_display = millis();
  }
}

void handleNMEASentence(char c) {
  // Store character in buffer
  if (c == '$') {
    // Start of a new sentence
    nmea_idx = 0;
  }
  
  if (nmea_idx < sizeof(nmea_buffer) - 1) {
    nmea_buffer[nmea_idx++] = c;
  }
  
  if (c == '\n') {
    // End of sentence
    nmea_buffer[nmea_idx] = '\0';
    Serial.print("NMEA: ");
    Serial.print(nmea_buffer);
    nmea_idx = 0;
  }
}

void checkSerialData() {
  uint32_t bytes_since_last_check = serial_bytes_received - last_serial_bytes_count;
  last_serial_bytes_count = serial_bytes_received;
  
  Serial.print("Bytes received in last 2 seconds: ");
  Serial.println(bytes_since_last_check);
  
  if (bytes_since_last_check == 0) {
    Serial.println("WARNING: No data received on Serial3! Check GPS connection and baud rate.");
  } else {
    Serial.println("Serial3 connection is working.");
    
    // Check if we're getting valid NMEA data
    if (gps.charsProcessed() > 0 && gps.sentencesWithFix() == 0) {
      Serial.println("NMEA data is being received but no valid fix obtained yet.");
      Serial.print("Valid sentences: ");
      Serial.println(gps.passedChecksum());
      Serial.print("Failed checksums: ");
      Serial.println(gps.failedChecksum());
    }
  }
}

void displayGPSData() {
  Serial.println("\n----- GPS Data -----");
  Serial.print("Valid fix: ");
  Serial.println(gps.location.isValid() ? "YES" : "NO");
  
  // Print processing stats
  Serial.print("GPS stats - Processed chars: ");
  Serial.print(gps.charsProcessed());
  Serial.print(", Sentences with fix: ");
  Serial.print(gps.sentencesWithFix());
  Serial.print(", Passed checksum: ");
  Serial.print(gps.passedChecksum());
  Serial.print(", Failed checksum: ");
  Serial.println(gps.failedChecksum());
  
  // Print raw NMEA sentence if data is coming in but no fix
  if (!gps.location.isValid() && serial_bytes_received > last_serial_bytes_count) {
    Serial.println("No valid fix, but GPS data is being received.");
    Serial.println("Possible causes: Poor satellite visibility, antenna issues, or time needed for first fix.");
    Serial.println("Try moving to a location with better sky visibility.");
  }
  
  if (gps.location.isValid()) {
    Serial.print("Location: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.println(gps.location.lng(), 6);
    
    Serial.print("Altitude: ");
    Serial.print(gps.altitude.meters());
    Serial.println(" m");
    
    Serial.print("Speed: ");
    Serial.print(gps.speed.kmph());
    Serial.println(" km/h");
    
    Serial.print("Course: ");
    Serial.print(gps.course.deg());
    Serial.println(" degrees");
    
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
    
    // Accuracy information
    Serial.print("HDOP: ");
    Serial.println(gps.hdop.value());
    
    float horizontalAccuracy = gps.hdop.value() * 2.5;
    Serial.print("Estimated Horizontal Accuracy: ");
    Serial.print(horizontalAccuracy, 1);
    Serial.println(" meters");
    
    // Vertical accuracy is typically 1.5x horizontal accuracy for GPS
    Serial.print("Estimated Vertical Accuracy: ");
    Serial.print(horizontalAccuracy * 1.5, 1);
    Serial.println(" meters");
    
    // Interpret the accuracy
    if (horizontalAccuracy < 2.0) {
      Serial.println("Position Quality: Excellent (RTK-like, <2m)");
    } else if (horizontalAccuracy < 5.0) {
      Serial.println("Position Quality: Good (2-5m)");
    } else if (horizontalAccuracy < 10.0) {
      Serial.println("Position Quality: Moderate (5-10m)");
    } else {
      Serial.println("Position Quality: Poor (>10m)");
    }
    
    Serial.print("Date/Time: ");
    if (gps.date.isValid() && gps.time.isValid()) {
      char dateTime[32];
      sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", 
              gps.date.year(), gps.date.month(), gps.date.day(),
              gps.time.hour(), gps.time.minute(), gps.time.second());
      Serial.println(dateTime);
    } else {
      Serial.println("INVALID");
    }
  }
}