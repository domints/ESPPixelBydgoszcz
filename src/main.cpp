#include <Arduino.h>
#include <Pixel.hpp>

PixelClass Pixel(Serial2, 22, 23);

uint8_t datablock[] = "9E00000105000195000600504961F19797F1F12797C12851285521216211212191510379715522833225122155121321232A131042419338211228212211315133315149970379E21212731212132415122D697314192419158215827A6025A7261151285151233251211131379602241933122421221233222112112223321221242213391420179637324722515116215835C3073C493D3014D390F8D7";

void setup() {
  Serial.begin(115200);
  Serial2.begin(4800, SERIAL_8E1, 19, 18);
  Pixel.begin();
  Serial.println("Checking GID...");
  char respMsg[2137];
  uint8_t errCode = Pixel.displayDataBlock(0, datablock, 317);
  Serial.print("Got response code: ");
  Serial.println(errCode);
  Serial.println(respMsg);
}

void loop() {
  // put your main code here, to run repeatedly:
}