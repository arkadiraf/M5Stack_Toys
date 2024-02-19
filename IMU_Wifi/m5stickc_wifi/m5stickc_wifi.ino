#include <M5StickCPlus.h>
#include <WiFi.h>

unsigned long lastMeasurementTime = 0; // Last measurement time

const char* ssid = "Controllab_Projects";
const char* password = "Controllab";
WiFiClient client;
const char* serverIP = "192.168.0.103"; // IP address of the server
const int serverPort = 60691; // Port number the server is listening on

struct Measurement {
  float accX, accY, accZ;
  float gyroX, gyroY, gyroZ;
  bool button;
  unsigned long timestamp;
};

Measurement measurements[10]; // Buffer for 10 measurements
int measurementIndex = 0; // Current index in the buffer

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.setCursor(50, 15);
  M5.Lcd.println("Connecting to WiFi...");
  M5.Imu.Init();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  M5.Lcd.println("Connected to WiFi");
  M5.Lcd.println(" X    Y    Z");
  Serial.begin(115200);
}

void loop() {
  M5.update();
  if (!client.connected()) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(50, 15);
    M5.Lcd.println("Reconnecting  server...");
    if (client.connect(serverIP, serverPort)) {
      M5.Lcd.println("Reconnected to server!");
    } else {
      M5.Lcd.println("Failed to reconnect to server.");
      return; // Don't proceed if we're not connected
    }
  }

  // measure intervals:
  unsigned long currentTime = millis();
  if (currentTime - lastMeasurementTime >= 10) { // Check if more than 10 ms have passed
    lastMeasurementTime = currentTime;
    M5.Imu.getGyroData(&measurements[measurementIndex].gyroX, &measurements[measurementIndex].gyroY, &measurements[measurementIndex].gyroZ);
    M5.Imu.getAccelData(&measurements[measurementIndex].accX, &measurements[measurementIndex].accY, &measurements[measurementIndex].accZ);
    measurements[measurementIndex].button = M5.BtnA.read();
    measurements[measurementIndex].timestamp = millis();

    measurementIndex++;

    // When buffer is full, send all measurements in one large buffer
    if (measurementIndex >= 10) {
      String dataToSend;
      for (int i = 0; i < 10; i++) {
        dataToSend += "IMU:" + String(measurements[i].timestamp) + "," +
                      String(measurements[i].accX, 3) + "," +
                      String(measurements[i].accY, 3) + "," +
                      String(measurements[i].accZ, 3) + "," +
                      String(measurements[i].gyroX, 2) + "," +
                      String(measurements[i].gyroY, 2) + "," +
                      String(measurements[i].gyroZ, 2) + "," +
                      String(measurements[i].button ? "1" : "0") + "\r\n";
      }
      client.print(dataToSend); // Send the entire string in one go
      measurementIndex = 0; // Reset buffer index
    }
  }

}