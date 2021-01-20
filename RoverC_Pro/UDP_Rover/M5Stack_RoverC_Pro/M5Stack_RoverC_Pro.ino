#include "M5Atom.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
WiFiUDP Udp;  // Creation of wifi Udp instance

// Motor settings
const int motor1_pin = 26;
const int motor2_pin = 32;
int freq = 10000;
int Motor1Channel = 0;
int Motor2Channel = 1;
int resolution = 8;
int superspeed = 1;
#define BRIGHTNESS_LED 0xf0
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
char packetBuffer[255];

// Set these to your desired credentials.
const char* ssid     = "M5StackJoy";
const char* password = "12345678";
// access point configurations
unsigned int localPort = 2050;

IPAddress ipServer(192, 168, 10, 1);   // Declaration of default IP for server
IPAddress ipClient(192, 168, 10, 2);  // Static ip option
IPAddress Subnet(255, 255, 255, 0);

void controlMotors(int LeftMotor, int RightMotor) {
  if ((LeftMotor > 5) || (RightMotor > 5)) {
    if (LeftMotor < 5) LeftMotor = 0;
    if (RightMotor < 5) RightMotor = 0;
    ledcWrite(Motor1Channel, LeftMotor);
    ledcWrite(Motor2Channel, RightMotor);
  } else {
    ledcWrite(Motor1Channel, 0);
    ledcWrite(Motor2Channel, 0);
  }
} // end cotrol function

void parseMSG() {
  //Serial.print(recMSG);
  int posLX = 0;
  int posLY = 0;
  int posRX = 0;
  int posRY = 0;
  int botL = 0;
  int botR = 0;
  //char msgChars[255];
    //memcpy(msgChars, packetBuffer);
    sscanf (packetBuffer,"Data:%d,%d,%d,%d,%d,%d",&posLX, &posLY, &posRX, &posRY, &botL, &botR);
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
    //controlMotors(posLY*superspeed,posRY*superspeed);
    controlMotors((posRY+posRX)*superspeed,(posRY-posRX)*superspeed);
    //controlMotors(int((posRY+posRX)*superspeed),int((posRY-posRX)*superspeed));
}
void setup()
{
  M5.begin(true, false, true);
  Serial.begin(115200);
  setBuff(BRIGHTNESS_LED, 0x00, 0x00);
  M5.dis.displaybuff(DisBuff);
  // init pwm
  ledcSetup(Motor1Channel, freq, resolution);
  ledcAttachPin(motor1_pin, Motor1Channel);
  ledcSetup(Motor2Channel, freq, resolution);
  ledcAttachPin(motor2_pin, Motor2Channel);
  controlMotors(0, 0);

  // set up wifi
  WiFi.mode(WIFI_STA); // ESP-32 as client
  //here config LR mode
  //int a = esp_wifi_set_protocol( WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N ); //WIFI_PROTOCOL_LR
  int a= esp_wifi_set_protocol(WIFI_IF_STA,WIFI_PROTOCOL_LR); //WIFI_PROTOCOL_LR
  // a= 0  configured ok :
  Serial.print("Mode: ");
  Serial.println(a);
  WiFi.config(ipClient, ipServer, Subnet);  // static ip option
  int8_t power = 0;
  esp_wifi_get_max_tx_power(&power);
  //esp_wifi_set_max_tx_power(78);
  Serial.print("Power (78 max, 19.5dB): ");
  Serial.println(power);
  WiFi.begin(ssid, password);
  //Wifi connection, we connect to master
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  setBuff(0x00, BRIGHTNESS_LED, 0x00);
  M5.dis.displaybuff(DisBuff);
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //long rssi = WiFi.RSSI();
  //Serial.print("RSSI:");
  //Serial.println(rssi);
  Udp.begin(localPort);
}// end setup


void loop()
{
  M5.update();
  static unsigned long timeoutMillis = millis();
  if ((millis() - timeoutMillis) > 1000){ 
    timeoutMillis = millis();
    controlMotors(0,0);
    Serial.println( "timeout!!! " );
  }
  //problems whith connection
  if ( WiFi.status() != WL_CONNECTED )
  {
    setBuff(BRIGHTNESS_LED, 0x00, 0x00);
    M5.dis.displaybuff(DisBuff);
    controlMotors(0,0);
    Serial.println( "|" );
    int tries = 0;
    WiFi.begin( ssid, password );
    while ( WiFi.status() != WL_CONNECTED ) {
      tries++;
      Serial.print("attempt: ");
      Serial.print(tries);
      Serial.print(" status: ");
      Serial.println(( WiFi.status()));
      delay( 500 );
    }
    Serial.print( "Connected " );
    Serial.println( WiFi.localIP() );
    setBuff(0x00, BRIGHTNESS_LED, 0x00);
    M5.dis.displaybuff(DisBuff);
  }
  static unsigned long sendMillis = 0;
  if ((millis() -  sendMillis) > 1000) {
    sendMillis = millis();
    int rssi = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi);
    Udp.beginPacket(ipServer,localPort);   //Initiate transmission of data
    char buf[20];   // buffer to hold the string to append
    sprintf(buf, "RSSI: %d \r\n", rssi);  // appending the millis to create a char
    Udp.printf(buf);  // print the char
    Udp.endPacket();  // Close communication
    
    for (int i = 0 ; i<25 ; i++){
      DisBuff[0] = 0x05;
      DisBuff[1] = 0x05;
      if (rssi/(i+1) < -5){
        DisBuff[2 + i * 3 + 0] = BRIGHTNESS_LED;
        DisBuff[2 + i * 3 + 1] = 0x00;
        DisBuff[2 + i * 3 + 2] = 0x00;
      }else{
        DisBuff[2 + i * 3 + 0] = 0x00;
        DisBuff[2 + i * 3 + 1] = BRIGHTNESS_LED;
        DisBuff[2 + i * 3 + 2] = 0x00;
      }
    }
    M5.dis.displaybuff(DisBuff);
  }

  //RECEPTION
  int packetSize = Udp.parsePacket();   // Size of packet to receive
  if (packetSize) {       // If we received a package
    timeoutMillis = millis(); // reset timeout
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len - 1] = 0;
//    Serial.print("Recieved: ");
//    Serial.print(Udp.remoteIP()); Serial.print(" / ");
//    Serial.print(Udp.remotePort()); Serial.print(" / ");
//    Serial.print(packetSize); Serial.print(" / ");
//    Serial.print(millis()); Serial.print(" / ");
//    Serial.println(packetBuffer);
    parseMSG();
  }
} // end main
