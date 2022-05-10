// based on a slider tutorial: https://randomnerdtutorials.com/esp32-web-server-slider-pwm/
#include "M5Atom.h"
#include <ESP32Servo.h>
//Servo1 PIN 26
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
Servo myservo;


int servoPin = 26;
//int pos = 0;
//bool flag = true;
//int servoCMD = 0;

String sliderValue = "1500";
const char* PARAM_INPUT = "value";


// Set these to your desired credentials.
//char ssid[] = "M5StackAP";
//char password[] = "12345678";
const char* ssid     = "M5StackAP";
const char* password = "12345678";

// access point configurations
AsyncWebServer server(80);
IPAddress ip(192,168,10,1);
IPAddress gateway(192,168,10,1);
IPAddress subnet(255,255,255,0);


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Scare Crow Server</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #FFD65C;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 
  </style>
</head>
<body>
  <h2>Scare Crow Server</h2>
  <p><span id="textSliderValue">%SLIDERVALUE%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this)" id="pwmSlider" min="500" max="2400" value="%SLIDERVALUE%" step="1" class="slider"></p>
<script>
function updateSliderPWM(element) {
  var sliderValue = document.getElementById("pwmSlider").value;
  document.getElementById("textSliderValue").innerHTML = sliderValue;
  console.log(sliderValue);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+sliderValue, true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if (var == "SLIDERVALUE"){
    return sliderValue;
  }
  return String();
}

void setup()
{
    M5.begin(true, false, true);
    delay(10);
    
    // Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    myservo.setPeriodHertz(50);    // standard 50 hz servo
    myservo.attach(servoPin, 500, 2400); // attaches the servo on pin 26 to the servo object

    WiFi.mode(WIFI_AP);
    Serial.begin(115200);
    Serial.println();
    Serial.println("Configuring access point...");
    // You can remove the password parameter if you want the AP to be open.
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(ip,gateway,subnet);
    //IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(ip);

      // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html, processor);
    });
    Serial.println("Server started");
    
    // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
    server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = inputMessage;
      myservo.write(sliderValue.toInt());
      //ledcWrite(ledChannel, sliderValue.toInt());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  
  // Start server
  server.begin();
}


void loop()
{
//  if(M5.Btn.wasPressed()){
//      flag = !flag;
//      flag ? servoCMD = COUNT_LOW : servoCMD = COUNT_HIGH;
//  }


//WiFiClient client = server.available();    // listen for incoming clients
//  if (client) {                             // if you get a client,
//    Serial.println("New Client.");           // print a message out the serial port
//    //String currentLine = "";               // make a String to hold incoming data from the client
//    unsigned long timeout = millis();
//    while (client.connected()) {            // loop while the client's connected
//      if (client.available()) {             // if there's bytes to read from the client,
//        timeout = millis();
//        char c = client.read();             // read a byte, then
//        Serial.write(c);                    // print it out the serial monitor
//      }else if ((millis() - timeout) > 10000) { // client timeout - search for a better solution
//            Serial.println(">>> Client Timeout !");
//            client.stop();
//            return;
//      } 
//    }
//    // close the connection:
//    client.stop();
//    Serial.println("Client Disconnected.");
//  }
//  
//  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
//    // in steps of 1 degree
//    myservo.write(pos);    // tell servo to go to position in variable 'pos'
//    delay(15);             // waits 15ms for the servo to reach the position
//  }
//  
//  Serial.println(servoCMD);
//
//  delay(100);
//  M5.update();
}
