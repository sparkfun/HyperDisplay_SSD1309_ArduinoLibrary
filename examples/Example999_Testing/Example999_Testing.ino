#include "HyperDisplay_UG2856KLBAG01_I2C.h"

#define SERIAL_PORT Serial

UG2856KLBAG01_I2C myTOLED;
#define WIRE_PORT Wire

#define RES_PIN 3


#define MY_BUFFER_LENGTH BUFFER_LENGTH


// 

uint8_t color = 0x01;

void Fill_RAM(unsigned char Data)
{
  myTOLED.setColumnAddress(0, 127);
  myTOLED.setPageAddress(0, 7);
  for(uint8_t indj = 0; indj < 64; indj++)
  { 
    WIRE_PORT.beginTransmission(0x3C);
    WIRE_PORT.write(0x40);
    for(uint8_t indi = 0; indi < 16; indi++)
    {
      WIRE_PORT.write(Data);
    }
    WIRE_PORT.endTransmission();
  }
}





void setup() {
  Serial.begin(9600);

  Serial.println(MY_BUFFER_LENGTH);
  Serial.println(sizeof(SSD1309_Bite_t));

  myTOLED.begin(WIRE_PORT, false, SSD1309_ARD_I2C_UNUSED_PIN);
  Wire.setClock(400000);

//  myTOLED.pixel(126,2,(color_t)&color);

  uint8_t a = 0xFF;

  myTOLED.hwxline(0,63, 10, (color_t)&a);

  myTOLED.line(10, 10, 350, 20, 1, (color_t)&color); 

  myTOLED.rectangleSet(15,15, 45, 38, true);
  myTOLED.rectangleClear(20,20, 40, 33, true);

  myTOLED.circleSet(100, 32, 28, true);
  myTOLED.circleClear(88, 28, 17, true);

  

}

void loop() {

  myTOLED.pixelSet(126,2);
  delay(1000);

  myTOLED.pixelClear(126,2);
  delay(1000);
  

}
