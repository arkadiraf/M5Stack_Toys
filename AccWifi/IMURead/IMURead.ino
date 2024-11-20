#include <M5StickCPlus.h>
#include <WiFi.h>

// Wi-Fi credentials
const char* ssid = "M5StickC_AP";
const char* password = "12345678";

// TCP server settings
WiFiServer server(8080);

// Sampling rate (100 ms -> 10 Hz)
const unsigned long sampleRate = 1; 
unsigned long previousMillis = 0;

// Buffer for accelerometer data with timestamps
struct AccelerometerData {
  float ax, ay, az;
  float timestamp; // Time in seconds with 0.0001 resolution
};
#define BUFFERSIZE 100
AccelerometerData buffer[BUFFERSIZE];
int bufferIndex = 0;

WiFiClient client; // Global client variable to track connected client

// Function to initialize Wi-Fi in AP mode
void setupWiFi() {
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();

  // Display Wi-Fi information on the LCD
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("SSID:");
  M5.Lcd.println(ssid);
  M5.Lcd.println("PWD:");
  M5.Lcd.println(password);
  M5.Lcd.println("IP:");
  M5.Lcd.println(IP);

  server.begin();
  Serial.println("Wi-Fi AP started");
  Serial.print("IP Address: ");
  Serial.println(IP);
}

// Function to initialize the IMU
void setupIMU() {
  M5.begin();
  M5.IMU.Init();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("IMU Ready");
  delay(2000);
  M5.Lcd.fillScreen(BLACK);
}

// Update the screen with client connection status
void updateClientStatus(const char* status) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Client Status:");
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.println(status);
}

// Collect accelerometer data with timestamp
void collectIMUData() {
  float ax, ay, az;
  M5.IMU.getAccelData(&ax, &ay, &az);

  // Store accelerometer data and timestamp in the buffer
  buffer[bufferIndex] = {ax, ay, az, micros() / 1000000.0}; // Convert to seconds with 0.0001 resolution
  bufferIndex++;

  if (bufferIndex == BUFFERSIZE) {
    sendBuffer();
    bufferIndex = 0;
  }
}

// Send the buffer data to the client
void sendBuffer() {
  if (client) { // Check if the client is connected
    for (int i = 0; i < BUFFERSIZE; i++) {
      // Include timestamp in seconds with accelerometer data
      client.printf("%.4f, %.4f, %.4f, %.4f\n", buffer[i].timestamp, buffer[i].ax, buffer[i].ay, buffer[i].az);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupIMU();
  setupWiFi();
}

void loop() {
  // Accept a new client if there's no active connection
  if (!client || !client.connected()) {
    client = server.available(); // Accept a new client
    if (client) {
      Serial.println("New client connected");
      updateClientStatus("Connected");
    }
  }

  // Collect data and send to the connected client
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > sampleRate) {
    previousMillis = currentMillis;
    collectIMUData();
  }

  // If the client disconnects, stop the client object
  if (client && !client.connected()) {
    Serial.println("Client disconnected");
    updateClientStatus("Disconnected");
    client.stop();
  }
}
