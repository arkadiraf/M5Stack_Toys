#include "M5Atom.h"
#include <WiFi.h>
// Motor settings
const int motor1_pin = 26;
const int motor2_pin = 32;
int freq = 10000;
int Motor1Channel = 0;
int Motor2Channel = 1;
int resolution = 8;
int superspeed = 1;

extern const unsigned char AtomImageData[375 + 2];

uint8_t DisBuff[2 + 5 * 5 * 3];

void setBuff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata)
{
    DisBuff[0] = 0x05;
    DisBuff[1] = 0x05;
    for (int i = 0; i < 25; i++)
    {
        DisBuff[2 + i * 3 + 0] = Rdata;
        DisBuff[2 + i * 3 + 1] = Gdata;
        DisBuff[2 + i * 3 + 2] = Bdata;
    }
}

// Wifi network
const char* ssid     = "M5StackAP";
const char* password = "12345678";

// Use WiFiClient class to create TCP connections
WiFiClient client;
const int httpPort = 23;

// recieved commands
String line;  
String recMSG;   

void controlMotors(int LeftMotor,int RightMotor){
  if ((LeftMotor > 5) || (RightMotor > 5)){
    if (LeftMotor < 5) LeftMotor = 0;
    if (RightMotor < 5) RightMotor = 0;
    ledcWrite(Motor1Channel, LeftMotor);
    ledcWrite(Motor2Channel, RightMotor);
  }else{
    ledcWrite(Motor1Channel, 0);
    ledcWrite(Motor2Channel, 0);
  }
} // end cotrol function

void parseMSG(){
  //Serial.print(recMSG);
  int posLX = 0;
  int posLY = 0;
  int posRX = 0;
  int posRY = 0;
  int botL = 0;
  int botR = 0;
  char msgChars[100];
  strcpy(msgChars, recMSG.c_str());
  sscanf (msgChars,"Data:%d,%d,%d,%d,%d,%d",&posLX, &posLY, &posRX, &posRY, &botL, &botR);
  Serial.print("Parsed: ");
  Serial.print(posLX);
  Serial.print(",");
  Serial.print(posLY);
  Serial.print(",");
  Serial.print(posRX);
  Serial.print(",");
  Serial.print(posRY);
  Serial.print(",");
  Serial.print(botL);
  Serial.print(",");
  Serial.println(botR);
  if (botL == 1) superspeed = 1;
  if (botR == 1) superspeed = 2;
  // update motors
  controlMotors(posLY*superspeed,posRY*superspeed);
}
void setup()
{
    M5.begin(true, false, true);
    Serial.begin(115200);
    delay(10);
    setBuff(0x20, 0x20, 0x00);
    M5.dis.displaybuff(DisBuff);

    // init pwm
    ledcSetup(Motor1Channel, freq, resolution);
    ledcAttachPin(motor1_pin, Motor1Channel);
    ledcSetup(Motor2Channel, freq, resolution);
    ledcAttachPin(motor2_pin, Motor2Channel);
    controlMotors(0,0);

    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    line = "";  
    recMSG = ""; 
}// end setup
int counter = 0;
void loop()
{
  M5.update();
  if (WiFi.status() != WL_CONNECTED){ // Reconnect to network
    Serial.println("Dropped from network, attempting to recconect");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }else if (!client.connect("192.168.10.1", httpPort)) { // try to connect to the target ip
        Serial.println("connection failed");
        // try to recconnect to network
        WiFi.disconnect();
        Serial.println("Disconnected from network");
  }else{ // connected and all is well
    long rssi = WiFi.RSSI();
    //Serial.print("RSSI:");
    //Serial.println(rssi);
    client.print("RSSI:");
    client.println(rssi);
    setBuff(0x00, 0x40, 0x00);
    M5.dis.displaybuff(DisBuff); 
    while (client.connected()) {            // loop while the client's connected
      while (client.available()) {             // if there's bytes to read from the client,
        char c = client.read(); 
        line += c;
        if  (c == '\n'){
          counter++;
          client.print("MSGs: ");
          client.println(counter);
          //Serial.print(line);                    // print it out the serial monitor 
          recMSG = "";
          recMSG = line;
          line = "";
          parseMSG();        
          break;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Disconnected");
    setBuff(0x40, 0x00, 0x00);
    M5.dis.displaybuff(DisBuff);
  }   
} // end main
