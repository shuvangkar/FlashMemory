#include "FlashMemory.h"

Flash flash(5); //CS PIN = 10

byte arr_to_write[] = {20,30,40,50,60,70};
byte readArr[8];
void setup(void)
{
  pinMode(9,OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(9,HIGH);
  digitalWrite(10, LOW);
  Serial.begin(9600);
  Serial.println("");
  flash.begin();
  Serial.println("Ready");
  //flash.eraseChipData();
  //flash.printPage(10);
  Serial.println("Writing few bytes in 6016 address");
  flash.writeBytes(300, arr_to_write, sizeof(arr_to_write));
  Serial.println("Reading bytes from 6016 address");
  flash.readBytes(300, readArr, sizeof(readArr));
  Serial.println("Setup done");
}

void loop(void) 
{
  //flash.printPage(19);
  //flash.readBytes(5000, readArr, sizeof(readArr));
  delay(3000);

}


