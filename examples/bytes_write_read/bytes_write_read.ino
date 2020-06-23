#include "FlashMemory.h"

Flash flash(10); //CS PIN = 10

byte arr_to_write[] = {1, 2, 3, 4, 5, 6, 7, 8};
byte readArr[8];
void setup(void)
{

  Serial.begin(9600);
  Serial.println("");
  flash.begin();
  Serial.println("Ready");
  //flash.eraseChipData();
  //flash.printPage(10);
  Serial.println("Writing few bytes in 6016 address");
  flash.writeBytes(6016, arr_to_write, sizeof(arr_to_write));
  Serial.println("Reading bytes from 6016 address");
  flash.readBytes(6016, readArr, sizeof(readArr));
  Serial.println("Setup done");
}

void loop(void) 
{
  //flash.printPage(19);
  //flash.readBytes(5000, readArr, sizeof(readArr));
  delay(3000);

}


