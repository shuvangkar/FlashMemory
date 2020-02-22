#include "flash_memory.h"

Flash flash(10);
byte arr[] = {1, 2, 3, 4, 5, 6, 7, 8};
byte readArr[8];
void setup(void)
{

  Serial.begin(9600);
  Serial.println("");
  flash.begin();
  Serial.println("Ready");
  //  flash.eraseChipData();
  //flash.printPage(10);
  flash.writeBytes(6016, arr, sizeof(arr));
  flash.readBytes(6016, readArr, sizeof(readArr));
}

void loop(void) 
{
  //flash.printPage(19);
  //flash.readBytes(5000, readArr, sizeof(readArr));
  delay(3000);

}


