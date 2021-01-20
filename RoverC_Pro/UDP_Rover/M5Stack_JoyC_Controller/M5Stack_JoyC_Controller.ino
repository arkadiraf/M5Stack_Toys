
/*
 * @Author: Sorzn
 * @Date: 2019-11-22 14:48:10
 * @LastEditTime: 2019-11-22 15:45:27
 * @Description: M5Stack project
 * @FilePath: /M5StickC/examples/Hat/JoyC/JoyC.ino
 */

#include "M5StickC.h"
#include "JoyC.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
WiFiUDP Udp; // Creation of wifi Udp instance

char packetBuffer[255]; // udp com
char text_buff[100];    // screen
String recMSG;
#define LED_BUILTIN 10   // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED
// Set these to your desired credentials.
const char* ssid     = "M5StackJoy";
const char* password = "12345678";
// access point configurations
unsigned int localPort = 2050;
IPAddress ip(192,168,10,1);
IPAddress gateway(192,168,10,1);
IPAddress subnet(255,255,255,0);
IPAddress clientIP(192,168,10,2);// default ip address
unsigned int clientPort = 2050;

// time stamp
unsigned long millisStamp = 0;
unsigned long lastMSG = 0;

// Joystick
JoyC joyc;
TFT_eSprite img = TFT_eSprite(&M5.Lcd);
struct JOYSTICK{
  int posX;
  int posY;
  int posXbias;
  int posYbias;
  uint8_t button;
};

// Left righ controllers
JOYSTICK leftJoy = {100,100,-100,100,0};
JOYSTICK rightJoy = {100,100,-100,100,0};

void readJoystick(void){
    // read joystick
    leftJoy.posX = -(int)joyc.GetX(0) - leftJoy.posXbias;
    leftJoy.posY = (int)joyc.GetY(0) - leftJoy.posYbias;
    leftJoy.button = joyc.GetPress(0); 
    rightJoy.posX = -(int)joyc.GetX(1) - rightJoy.posXbias;
    rightJoy.posY = (int)joyc.GetY(1) - rightJoy.posYbias;
    rightJoy.button = joyc.GetPress(1);

    // set saturation
    if (rightJoy.posX > 100) rightJoy.posX = 100;
    if (rightJoy.posX < -100) rightJoy.posX = -100;
    if (rightJoy.posY > 100) rightJoy.posY = 100;
    if (rightJoy.posY < -100) rightJoy.posY = -100;
    if (leftJoy.posX > 100) leftJoy.posX = 100;
    if (leftJoy.posX < -100) leftJoy.posX = -100;
    if (leftJoy.posY > 100) leftJoy.posY = 100;
    if (leftJoy.posY < -100) leftJoy.posY = -100;
}

void setup()
{
    M5.begin();
    Serial.begin(115200);
    Wire.begin(0, 26, 400000);
    img.createSprite(80, 160);
    joyc.SetLedColor(0x0000ff);

    WiFi.mode(WIFI_AP);
    //here config LR mode
    //int a= esp_wifi_set_protocol( WIFI_IF_AP, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N ); //WIFI_PROTOCOL_LR
    int a= esp_wifi_set_protocol(WIFI_IF_AP,WIFI_PROTOCOL_LR); //WIFI_PROTOCOL_LR
    Serial.print("Mode: ");
    Serial.println(a);
    int8_t power = 0;
    esp_wifi_get_max_tx_power(&power);
    //esp_wifi_set_max_tx_power(78);
    Serial.print("Power (78 max, 19.5dB): ");
    Serial.println(power); 
    WiFi.softAP(ssid, password);  // ESP-32 as access point
    WiFi.softAPConfig(ip,gateway,subnet);
    // init access point
    Udp.begin(localPort); 
    //Serial.print("LocalIP: ");
    //Serial.println( WiFi.localIP() );
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);

    // clear image
    img.fillSprite(TFT_BLACK);
    img.drawCentreString("IP Address", 40, 70, 1);
    sprintf(text_buff, "192.168.10.1");
    img.drawCentreString(text_buff, 40, 84, 1);
    sprintf(text_buff, "Port 2050");
    img.drawCentreString(text_buff, 40, 98, 1);
    // update image
    img.pushSprite(0, 0);
    delay(2000);
}

void loop()
{
  M5.update();
  // get biases
  if(M5.BtnA.wasPressed())
  {
    rightJoy.posXbias = -(int)joyc.GetX(1);
    rightJoy.posYbias = (int)joyc.GetY(1);
    leftJoy.posXbias = -(int)joyc.GetX(0);
    leftJoy.posYbias = (int)joyc.GetY(0);
    Udp.stop(); 
    Udp.begin(localPort); 
    digitalWrite(LED_BUILTIN,HIGH);
  }
  int packetSize = Udp.parsePacket();
  if (packetSize) {
      int len = Udp.read(packetBuffer, 255);
      if (len > 0) packetBuffer[len-1] = 0;
      Serial.print("Got Packet: ");
      clientIP = Udp.remoteIP();
      clientPort = Udp.remotePort();
      Serial.print(clientIP);Serial.print(" / ");
      Serial.print(clientPort);Serial.print(" / ");
      Serial.print(packetSize);Serial.print(" / ");
      Serial.println(packetBuffer);
      recMSG = packetBuffer;
      //Serial.println(recMSG);
      digitalWrite(LED_BUILTIN,LOW);
      lastMSG = millis();
   }
  // check for disconnection from AP
  if ((millis() -  lastMSG) > 5000) {
    lastMSG = millis();
    Serial.println("AP Reset");
    Udp.stop(); 
    Udp.begin(localPort); 
    digitalWrite(LED_BUILTIN,HIGH);
  }
  static unsigned long loopTime = millis();
  if ((millis() - loopTime) > 40 ){
    loopTime = millis();
      // measure fps
      unsigned long fps = millis() - millisStamp;
      millisStamp = millis();         
      // clear image
      img.fillSprite(TFT_BLACK);
      readJoystick();
      
      // send packet
      Udp.beginPacket(clientIP,clientPort);
      sprintf(packetBuffer,"Data:%d,%d,%d,%d,%d,%d\r\n",leftJoy.posX,leftJoy.posY,rightJoy.posX,rightJoy.posY,leftJoy.button,rightJoy.button);
      Udp.printf(packetBuffer);
      Udp.printf("\r\n");
      Udp.endPacket();
    
    
      // print results to screen
      img.drawCentreString("Joy Left", 40, 6, 1);
      sprintf(text_buff, "%d  %d", leftJoy.posX, leftJoy.posY);
      img.drawCentreString(text_buff, 40, 20, 1);
    
      img.drawCentreString("Joy Right", 40, 34, 1);
      sprintf(text_buff, "%d  %d", rightJoy.posX, rightJoy.posY);
      img.drawCentreString(text_buff, 40, 48, 1);
    
      img.drawCentreString("Joy Press", 40, 62, 1);
      sprintf(text_buff, "%d  %d", leftJoy.button, rightJoy.button);
      img.drawCentreString(text_buff, 40, 76, 1);
    
      //sprintf(text_buff, "fps  %d", 1000/fps);
      //img.drawCentreString(text_buff, 40, 90, 1); 
    
      // print messages in two raws of 10 chars per raw.
      img.drawCentreString("Recieved MSG", 40, 104, 1);
      String subString = "";
      if (recMSG.length() >= 12){ // "/r/n" 
        subString = recMSG.substring(0, 10); 
        sprintf(text_buff, "%s", subString);
        img.drawCentreString(text_buff, 40, 118, 1);
        if  (recMSG.length() <= 22){ // do not try more than possible (there are '/r/n' chars
          subString = recMSG.substring(10,recMSG.length()-2);
          sprintf(text_buff, "%s", subString);
          img.drawCentreString(text_buff, 40, 132, 1);
        }else{
          img.drawCentreString("Long MSG", 40, 132, 1);
        }
      }else{
        subString = recMSG.substring(0, recMSG.length()-2);
        sprintf(text_buff, "%s", subString);
        img.drawCentreString(text_buff, 40, 118, 1);
      }
      // update image
      img.pushSprite(0, 0);
      //delay(50); // limit refresh rate

  }
}
