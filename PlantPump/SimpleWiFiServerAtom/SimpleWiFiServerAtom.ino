//https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
// SimpleWiFiServer example:  WiFi Web Server LED Blink

#include "M5Atom.h"
// Load Wi-Fi library
#include <WiFi.h>
#include <esp_wifi.h>

// network credentials
const char* ssid = "Contollab";
const char* password = "Controllab";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 1000;


void setup()
{
  // init m5stack atom
  M5.begin(false, false, true);
  delay(10);
  Serial.begin(115200);
  M5.dis.drawpix(0, 0x0000f0);
  
  // init wifi settings
  initWiFi();

}

// main code
void loop()
{
  unsigned long currentTime = millis(); 
  // check connection status:
  if (WiFi.status() == WL_CONNECTED){
    WiFiClient client = server.available();   // listen for incoming clients
    if (client) {                             // if you get a client,
      Serial.println("New Client.");          // print a message out the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      while (client.connected()) {            // loop while the client's connected
        if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();             // read a byte, then
          Serial.write(c);                    // print it out the serial monitor
          if (c == '\n') {                    // if the byte is a newline character
  
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
  
              // the content of the HTTP response follows the header:
              client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
              client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");
  
              // The HTTP response ends with another blank line:
              client.println();
              // break out of the while loop:
              break;
            } else {    // if you got a newline, then clear currentLine:
              currentLine = "";
            }
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
  
          // Check to see if the client request was "GET /H" or "GET /L":
          if (currentLine.endsWith("GET /H")) {
            M5.dis.drawpix(0, 0xf00000);
          }
          if (currentLine.endsWith("GET /L")) {
            M5.dis.drawpix(0, 0x000000);
          }
        }
      }
      // close the connection:
      client.stop();
      Serial.println("Client Disconnected.");
    }
  }
  // simple btn test
  if (M5.Btn.wasPressed()){
    static int i=0;
    Serial.print("M5Stack button: ");
    Serial.println(i++);
  }

delay(50);
M5.update();
}

// init wifi
void initWiFi() {
  // delete old config
  WiFi.disconnect(true);

  delay(1000);

  // power settings
  //  int8_t power = 0;
  //  esp_wifi_get_max_tx_power(&power);
  //  //esp_wifi_set_max_tx_power(78);
  //  Serial.print("Wifi Power (78 max, 19.5dB): ");
  //  Serial.println(power);

  // ESP-32 as client
  // WiFi.mode(WIFI_STA); 
  // delay(100);
  // int a = esp_wifi_set_protocol( WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N ); //WIFI_PROTOCOL_LR
  // Serial.print("Mode: ");
  // Serial.println(a);
  
  // wifi events
  WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

    /* Remove WiFi event
  Serial.print("WiFi Event ID: ");
  Serial.println(eventID);
  WiFi.removeEvent(eventID);*/

  // scan networks
  availableNetworks();
  

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ssid: ");
  Serial.print(ssid);  Serial.print("  password:  "); Serial.println(password);
  WiFi.begin(ssid, password);

}

// available networks
void availableNetworks(){
    Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");
}

// wifi events:
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Init Server Port: 80");
  server.begin();
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
  Serial.print("Trying to Reconnect  ssid:");
  Serial.print(ssid);  Serial.print("  password: "); Serial.println(password);
  WiFi.begin(ssid, password);
}
