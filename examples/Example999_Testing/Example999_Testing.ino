#include "HyperDisplay_SSD1309.h"

#define SERIAL_PORT Serial

UG2856KLBAG01_I2C myTOLED;

bool state = false;

void setup() {
  Serial.begin(9600);

  myTOLED.begin(Wire, false, SSD1309_ARD_I2C_UNUSED_PIN);

}

void loop() {
  // put your main code here, to run repeatedly:


  

//  uint8_t arry[] = {0xFF, 0xAA, 0x33, 0x00};
//  myTOLED.writeBytes(arry, true, sizeof(arry));
  
  myTOLED.overrideRam(state);

  state = !state;
  
  delay(200);

}
