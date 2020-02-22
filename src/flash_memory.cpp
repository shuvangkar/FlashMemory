#include "flash_memory.h"
#include <SPI.h>
#include <SPI.h>

#define FLASH_DEBUG
#ifdef FLASH_DEBUG
#define debugFlash(...) Serial.print(__VA_ARGS__)
#define debugFlashln(...) Serial.println(__VA_ARGS__)
#else
#define debugFlash(...)   __VA_ARGS__
#define debugFlashln(...)  __VA_ARGS__
#endif
/*********flash commands**********/
#define WB_WRITE_ENABLE       0x06
#define WB_WRITE_DISABLE      0x04
#define WB_CHIP_ERASE         0xc7
#define WB_READ_STATUS_REG_1  0x05
#define WB_READ_DATA          0x03
#define WB_PAGE_PROGRAM       0x02
#define WB_JEDEC_ID           0x9f
/*********************************/

Flash::Flash(byte chipSelect)
{
  _csPin = chipSelect;
  pinMode(_csPin, OUTPUT);
}
void Flash::begin()
{
  SPI.begin();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);
}

void Flash::eraseChipData()
{
  debugFlashln(F("Erasing Chip.."));
  _chipErase();
  debugFlashln(F("Done"));
}
void Flash::printPageBytes(byte *pageBuf)
{
  for (byte i = 0; i < 16; i++)
  {
    for (byte j = 0; j < 16; j++)
    {
      debugFlash(pageBuf[i * 16 + j], DEC); debugFlash(" ");
    }
    debugFlashln();
  }
}

void Flash::printBytes(byte *buf, byte len)
{
  for (byte i = 0; i < len; i++)
  {
    debugFlash(buf[i]); debugFlash(" ");
  }
  debugFlashln();
}
void Flash::printPage(unsigned int pageNum)
{
  debugFlash(F("Page Read: ")); debugFlashln(pageNum);
  byte pageBuf[256];
  _readPage(pageNum, pageBuf);
  printPageBytes(pageBuf);

}
void Flash::_readPage(unsigned int pageNum, byte *page_buffer)
{
  digitalWrite(_csPin, HIGH);
  digitalWrite(_csPin, LOW);
  SPI.transfer(WB_READ_DATA);
  // Construct the 24-bit address from the 16-bit page
  SPI.transfer((pageNum >> 8) & 0xFF);
  SPI.transfer((pageNum >> 0) & 0xFF);
  SPI.transfer(0);

  for (int i = 0; i < 256; ++i) {
    page_buffer[i] = SPI.transfer(0);
  }
  digitalWrite(_csPin, HIGH);
  _busyWait();

}
void Flash::_writePage(unsigned int PageNum, byte *pageBuf)
{
  digitalWrite(_csPin, HIGH);
  digitalWrite(_csPin, LOW);
  SPI.transfer(WB_WRITE_ENABLE);

  digitalWrite(_csPin, HIGH);
  digitalWrite(_csPin, LOW);
  SPI.transfer(WB_PAGE_PROGRAM);
  SPI.transfer((PageNum >>  8) & 0xFF);
  SPI.transfer((PageNum >>  0) & 0xFF);
  SPI.transfer(0);

  for (int i = 0; i < 256; ++i) {
    SPI.transfer(pageBuf[i]);
  }
  digitalWrite(_csPin, HIGH);
  digitalWrite(_csPin, LOW);
  SPI.transfer(WB_WRITE_DISABLE);
  digitalWrite(_csPin, HIGH);
  _busyWait();
}
bool Flash::readBytes(uint32_t logicalAddr, byte *data, byte length)
{
  uint16_t page = logicalAddr / 256;
  uint8_t offset = logicalAddr % 256;

  byte lenPage[2] = {0};
  lenPage[0] = length;
  int lastOffset = offset + length;
  if (lastOffset > 255)
  {
    lenPage[0] = 256 - offset;
    lenPage[1] = length - lenPage[0];
  }
  debugFlash(F("Reading(page, offset): "));
  debugFlash(page); debugFlash(" "); debugFlashln(offset);

  byte pageDataBuf[256];
  byte lastIndex = 0;
  for (byte nxtPage = 0; nxtPage < 2; nxtPage++)
  {
    int currentPage = page + nxtPage;
    _readPage(currentPage, pageDataBuf);
    debugFlash(F("----Read Page : ")); debugFlashln(currentPage);
#ifdef FLASH_DEBUG
    printPageBytes(pageDataBuf);
#endif
    byte j;
    for ( j = 0; j < lenPage[nxtPage]; j++)
    {
      data[j + lastIndex] =  pageDataBuf[offset + j];
    }
    //_writePage(currentPage, pageDataBuf); //Writing data into flash
    //debugFlash(F("----Write Page : ")); debugFlashln(currentPage);
    //printPageBytes(pageDataBuf);

    if (lenPage[1] > 0)
    {
      offset = 0;
      lastIndex = j;
    }
    else
    {
      break;
    }
  }
  printBytes(data, length);
}
bool Flash::writeBytes(uint32_t logicalAddr, byte *data, byte length)
{
  uint16_t page = logicalAddr / 256;
  uint8_t offset = logicalAddr % 256;

  byte lenPage[2] = {0};
  lenPage[0] = length;
  int lastOffset = offset + length;
  if (lastOffset > 255)
  {
    lenPage[0] = 256 - offset;
    lenPage[1] = length - lenPage[0];
  }
  debugFlash(F("Writing(page, offset): "));
  debugFlash(page); debugFlash(" "); debugFlashln(offset);

  byte pageDataBuf[256];
  byte lastIndex = 0;
  for (byte nxtPage = 0; nxtPage < 2; nxtPage++)
  {
    int currentPage = page + nxtPage;
    _readPage(currentPage, pageDataBuf);
    debugFlash(F("----Read Page : ")); debugFlashln(currentPage);
#ifdef FLASH_DEBUG
    printPageBytes(pageDataBuf);
#endif
    byte j;
    for ( j = 0; j < lenPage[nxtPage]; j++)
    {
      pageDataBuf[offset + j] = data[j + lastIndex];
    }
    _writePage(currentPage, pageDataBuf); //Writing data into flash
    debugFlash(F("----Write Page : ")); debugFlashln(currentPage);
#ifdef FLASH_DEBUG
    printPageBytes(pageDataBuf);
#endif

    if (lenPage[1] > 0)
    {
      offset = 0;
      lastIndex = j;
    }
    else
    {
      break;
    }
  }
}
void Flash::_busyWait()
{
  digitalWrite(_csPin, HIGH);
  digitalWrite(_csPin, LOW);
  SPI.transfer(WB_READ_STATUS_REG_1);
  while (SPI.transfer(0) & 1) {};
  digitalWrite(_csPin, HIGH);
}

void Flash::_chipErase()
{
  byte instruction[3] = {WB_WRITE_ENABLE,WB_CHIP_ERASE,WB_WRITE_DISABLE};
  for(byte i = 0; i<3; i++)
  {
    digitalWrite(_csPin, HIGH);
    digitalWrite(_csPin,LOW);
    SPI.transfer(instruction[i]);
  }
  digitalWrite(_csPin, HIGH);
  _busyWait();
}

