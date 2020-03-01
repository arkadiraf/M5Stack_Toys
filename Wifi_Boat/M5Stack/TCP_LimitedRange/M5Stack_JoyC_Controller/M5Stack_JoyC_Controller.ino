
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
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <esp_wifi.h>

// M5Stcikc status led
#define LED_BUILTIN 10

// time stamp
unsigned long millisStamp = 0;

// Wifi settings:
const char* ssid     = "M5StackAP";
const char* password = "12345678";

// access point configurations
WiFiServer server(23);
IPAddress ip(192,168,10,1);
IPAddress gateway(192,168,10,1);
IPAddress subnet(255,255,255,0);

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

char text_buff[100];
String line;  
String recMSG;        
void setup()
{
    M5.begin();
    Wire.begin(0, 26, 400000);
    img.createSprite(80, 160);
    joyc.SetLedColor(0x0000ff);
    WiFi.mode(WIFI_AP);
    
    int a= esp_wifi_set_protocol( WIFI_IF_AP, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N ); //WIFI_PROTOCOL_11B WIFI_PROTOCOL_LR
    // Call esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) to set the station/AP to BGN and the Espressif-specific mode.
    // https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/wifi.html
    Serial.println(a);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);
    Serial.begin(115200);
    Serial.println();
    Serial.println("Configuring access point...");
  
    // You can remove the password parameter if you want the AP to be open.
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(ip,gateway,subnet);
    //IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(ip);
    server.begin();
  
    Serial.println("Server started");
    // clear image
    img.fillSprite(TFT_BLACK);
    img.drawCentreString("IP Address", 40, 70, 1);
    sprintf(text_buff, "192.168.10.1");
    img.drawCentreString(text_buff, 40, 84, 1);
    sprintf(text_buff, "Port 23");
    img.drawCentreString(text_buff, 40, 98, 1);
    // update image
    img.pushSprite(0, 0);
    delay(1000);
    line = "";  
    recMSG = ""; 
}

void loop()
{
 WiFiClient client = server.available();    // listen for incoming clients

  if (client) {                             // if you get a client,
    digitalWrite(LED_BUILTIN,LOW);
    Serial.println("New Client.");           // print a message out the serial port
    while (client.connected()) {            // loop while the client's connected     
          // Read Joystick and print to screen  
          M5.update();
          
          // measure fps
          unsigned long fps = millis() - millisStamp;
          millisStamp = millis();         
          // clear image
          img.fillSprite(TFT_BLACK);
          // get biases
          if(M5.BtnA.wasPressed())
          {
            rightJoy.posXbias = -(int)joyc.GetX(1);
            rightJoy.posYbias = (int)joyc.GetY(1);
            leftJoy.posXbias = -(int)joyc.GetX(0);
            leftJoy.posYbias = (int)joyc.GetY(0);
          }
          readJoystick();

          // send it to the client 
//          client.sprintf("Data:%d,%d,%d,%d,%d,%d\r\n",leftJoy.posX,leftJoy.posY,rightJoy.posX,rightJoy.posY,leftJoy.button,rightJoy.button)
          client.print("Data:");
          client.print(leftJoy.posX);
          client.print(",");
          client.print(leftJoy.posY);
          client.print(",");
          client.print(rightJoy.posX);
          client.print(",");
          client.print(rightJoy.posY);
          client.print(","); 
          client.print(leftJoy.button);
          client.print(",");
          client.println(rightJoy.button);
          while (client.available()) {             // if there's bytes to read from the client,
            char c = client.read(); 
            line += c;
            if  (c == '\n'){
              Serial.print(line);                    // print it out the serial monitor 
              recMSG = "";
              recMSG = line;
              line = "";
              break;
            }
          }

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

          sprintf(text_buff, "fps  %d", 1000/fps);
          img.drawCentreString(text_buff, 40, 90, 1); 

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
          delay(50); // limit refresh rate
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }else{
    // Read Joystick and print to screen  
    M5.update();

    // measure fps
    unsigned long fps = millis() - millisStamp;
    millisStamp = millis();
    // clear image
    img.fillSprite(TFT_BLACK);
    
    if(M5.BtnA.wasPressed())
    {
      rightJoy.posXbias = -(int)joyc.GetX(1);
      rightJoy.posYbias = (int)joyc.GetY(1);
      leftJoy.posXbias = -(int)joyc.GetX(0);
      leftJoy.posYbias = (int)joyc.GetY(0);
    }
    
    readJoystick();

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
  
    img.drawCentreString("IP Address", 40, 90, 1);
    sprintf(text_buff, "192.168.10.1");
    img.drawCentreString(text_buff, 40, 104, 1);
    sprintf(text_buff, "Port 23");
    img.drawCentreString(text_buff, 40, 118, 1);
    sprintf(text_buff, "fps  %d", 1000/fps);
    img.drawCentreString(text_buff, 40, 132, 1); 
    sprintf(text_buff, "Connect");
    img.drawCentreString(text_buff, 40, 146, 1);  
    // update image
    img.pushSprite(0, 0);
    digitalWrite(LED_BUILTIN,HIGH); 
  }
}
