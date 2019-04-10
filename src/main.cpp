#include "Arduino.h"
#include "Wire.h"

void setup()
{
  Serial.begin(115200);
  delay(4000);
}

void loop()
{
  Serial.println("Hello");
  delay(1000);
}