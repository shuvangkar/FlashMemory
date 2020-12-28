#include "FlashMemory.h"

#define FLASH_CS 9
#define OTHER_CS 7 //consider other spi device is connected in bus
#define FLASH_W_ADDR 700

Flash flash(FLASH_CS);

byte arr_to_write[] = {1, 2, 3, 4, 5, 6};
byte readArr[8];

void setup(void)
{
  pinMode(OTHER_CS, OUTPUT);
  digitalWrite(OTHER_CS, HIGH);
  Serial.begin(250000);

  flash.begin();
  Serial.println("Ready");

  //flash.eraseChipData();
  //flash.printPage(10);

  Serial.println("Writing bytes");
  flash.write(FLASH_W_ADDR, arr_to_write, sizeof(arr_to_write));

  Serial.println("Reading bytes");
  flash.read(FLASH_W_ADDR, readArr, sizeof(readArr));
  flash.printBytes(readArr, sizeof(readArr));

  Serial.println("Setup done");
}

void loop(void)
{
  //flash.printPage(19);
  //flash.readBytes(5000, readArr, sizeof(readArr));
  delay(3000);

}
