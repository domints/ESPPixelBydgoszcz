#include <Arduino.h>
#include <Pixel.hpp>
#include <Adafruit_GFX_Pixel.hpp>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>

PixelClass Pixel(Serial2, 22, 23);
Adafruit_Pixel Pixel_GFX(Pixel, 84);

void setup() {
  Serial.begin(115200);
  Serial2.begin(4800, SERIAL_8E1, 19, 18);
  Pixel.begin();
  Serial.println("Sleeping some...");
  delay(5000);
  Serial.println("Initializing driver...");
  Pixel_GFX.init();
  Serial.println("Drawing line...");
  Pixel_GFX.setFont(&FreeSerif9pt7b);
  Pixel_GFX.setCursor(0, 15);
  Pixel_GFX.print("Test text...");
  Pixel_GFX.selectBuffer(1);
  Pixel_GFX.setCursor(0, 15);
  Pixel_GFX.setFont(&FreeMono12pt7b);
  Pixel_GFX.print("Let's see this...");
  uint8_t errCode = Pixel_GFX.commitBufferToPage(0);
  Serial.print("Got response code: ");
  Serial.println(errCode);
  delay(1000);
  errCode = Pixel_GFX.commitBufferToPage(1);
  Serial.print("Got response code: ");
  Serial.println(errCode);
}

void loop() {
  // put your main code here, to run repeatedly:
}