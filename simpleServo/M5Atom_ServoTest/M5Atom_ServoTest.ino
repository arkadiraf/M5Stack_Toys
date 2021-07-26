#include "M5Atom.h"
#include <ESP32Servo.h>
//Servo1 PIN 26
//Servo2 PIN 32

Servo myservo1;
Servo myservo2;

int servoPin1 = 26;
int servoPin2 = 32;
int pos = 0;
bool flag = true;
int servoCMD = 0;

void setup()
{

  
    M5.begin(true, false, true);
    delay(10);
    
    // Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    myservo1.setPeriodHertz(50);    // standard 50 hz servo
    myservo2.setPeriodHertz(50);    // standard 50 hz servo
    myservo1.attach(servoPin1, 500, 2400); // attaches the servo on pin 18 to the servo object
    myservo2.attach(servoPin2, 500, 2400); // attaches the servo on pin 18 to the servo object
    
}


void loop()
{
//  if(M5.Btn.wasPressed()){
//      flag = !flag;
//      flag ? servoCMD = COUNT_LOW : servoCMD = COUNT_HIGH;
//  }

  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo1.write(pos);    // tell servo to go to position in variable 'pos'
    delay(15);             // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo2.write(pos);    // tell servo to go to position in variable 'pos'
    delay(15);             // waits 15ms for the servo to reach the position
  }
  
  Serial.println(servoCMD);

  delay(100);
  M5.update();
}
