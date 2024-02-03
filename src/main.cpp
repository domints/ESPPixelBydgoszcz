#include <Arduino.h>
#include <Pixel.hpp>
#include <SoftwareSerial.h>

SoftwareSerial pixelSerial(D7, D8);
PixelClass Pixel(pixelSerial, D6, D5);

void setup() {
  Serial.begin(115200);
  pixelSerial.begin(4800, SoftwareSerialConfig::SWSERIAL_8E1);
  Pixel.begin();
  Serial.println("Sleeping for a moment...");
  delay(2000);
  Serial.println("Checking GID...");
  char respMsg[2137];
  uint16_t respMsgLen;
  uint8_t errCode = Pixel.readStringCommand(0, "GID", respMsg, 2137, respMsgLen);
  Serial.print("Got response code: ");
  Serial.println(errCode);
  Serial.println(respMsg);
}

void loop() {
  // put your main code here, to run repeatedly:
}