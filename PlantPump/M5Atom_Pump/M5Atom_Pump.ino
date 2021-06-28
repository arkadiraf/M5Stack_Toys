#include "M5Atom.h"

#define INPUT_PIN 32
#define PUMP_PIN 26

bool flag = true;
int rawADC;

void setup()
{
    M5.begin(true, false, true);
    delay(10);
    pinMode(INPUT_PIN,INPUT);
    pinMode(PUMP_PIN,OUTPUT);
}


void loop()
{
  rawADC = analogRead(INPUT_PIN);
  //Serial.print("Watering ADC value: ");
  Serial.println(rawADC);
  if(M5.Btn.wasPressed()){
      digitalWrite(PUMP_PIN,flag);
      flag = !flag;
  }
    delay(50);
    M5.update();
}
