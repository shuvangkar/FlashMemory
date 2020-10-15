#include "FlashMemory.h"
#include <SPI.h>
#include <SPI.h>
#include "FlashRegisters.h"

#define FLASH_DEBUG
#ifdef FLASH_DEBUG
#define debugFlash(...) Serial.print(__VA_ARGS__)
#define debugFlashln(...) Serial.println(__VA_ARGS__)
#else
#define debugFlash(...)   __VA_ARGS__
#define debugFlashln(...)  __VA_ARGS__
#endif

// /*********flash commands**********/
// #define FLASH_WRITE_ENABLE       0x06
// #define FLASH_WRITE_DISABLE      0x04
// #define FLASH_READ_STATUS_1      0x05
// #define FLASH_READ_STATUS_2      0x35
// #define FLASH_READ_STATUS_3      0x15
// #define FLASH_WRITE_STATUS_1     0x01
// #define FLASH_WRITE_STATUS_2     0x31
// #define FLASH_WRITE_STATUS_3     0x11
// #define FLASH_READ_DATA          0x03

// #define WB_CHIP_ERASE         0xc7
// #define WB_SECTOR_ERASE       0x20
// #define WB_READ_STATUS_REG_1  0x05
// #define WB_READ_DATA          0x03
// #define WB_PAGE_PROGRAM       0x02
// #define WB_JEDEC_ID           0x9f
// #define SPIFLASH_STATUSWRITE      0x01        // write status register
// /*********************************/


Flash::Flash(byte chipSelect)
{
  _csPin = chipSelect;
  
}
Flash::Flash(byte CS, uint32_t startAddr, uint16_t packetSz)
{
	_csPin = CS;
  _startAddr = startAddr;
  _packetSz = packetSz;
}

void Flash::begin()
{
  pinMode(_csPin, OUTPUT);
  csHigh();

  SPI.begin();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  delay(1);
  // _setWriteProtectSchema(INDIVIDUAL_BLOCK);
  _setUnlock(GLOBAL);
}


uint8_t *Flash::readPage(uint32_t pageNo, uint8_t *pageBuffer)
{
  _busyWait();
  csLow();
  SPI.transfer(FLASH_READ_DATA);
  // SPI.transfer((uint8_t)(pageNo>>16));
  SPI.transfer((uint8_t)(pageNo>>8));
  SPI.transfer((uint8_t)(pageNo));
  SPI.transfer(0);

  uint8_t *ptr = pageBuffer;
  for(int i = 0; i < 256; i++)
  {
    ptr[i] = SPI.transfer(0);
  }
  csHigh();
}

void Flash::setFlashSize(byte sizeMbit)
{
  _flashSz = sizeMbit;
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
  // _readPage(pageNum, pageBuf);
  readPage(pageNum,pageBuf);
  printPageBytes(pageBuf);

}
byte  *Flash::_readPage(unsigned int pageNum, byte *page_buffer)
{
  byte *buf = page_buffer;
  digitalWrite(_csPin, HIGH);
  digitalWrite(_csPin, LOW);
  SPI.transfer(FLASH_READ_DATA);
  // Construct the 24-bit address from the 16-bit page
  SPI.transfer((pageNum >> 8) & 0xFF); //Least 8 bits
  SPI.transfer((pageNum >> 0) & 0xFF); //Most 8 bits
  SPI.transfer(0);					    //MSB of 24 bit address

  for (int i = 0; i < 256; ++i) {
    buf[i] = SPI.transfer(0);
  }
  digitalWrite(_csPin, HIGH);
  _busyWait();

  return page_buffer;

}
void Flash::_writePage(unsigned int PageNum, byte *pageBuf)
{
  _busyWait();
  // _setUnlock(SECTOR,1);
  _writeEnable();
  csLow();
  uint8_t *p = (uint8_t*)&PageNum;
  SPI.transfer(FLASH_PAGE_PROGRAM);
  SPI.transfer(p[1]);
  SPI.transfer(p[0]);
  SPI.transfer(0);
  for (int i = 0; i < 256; ++i) 
  {
    SPI.transfer(pageBuf[i]);
  }
  digitalWrite(_csPin,HIGH);
  _writeDisable();
}


void Flash::_chipErase()
{
  _writeEnable();
  csLow();
  SPI.transfer(FLASH_CHIP_ERASE);
  csHigh();
  _writeDisable();
  _busyWait();

}


void Flash::eraseSector(uint32_t addr)
{
  // unlockSector(sectorAddr);
  debugFlash(F("Erasing Sector "));debugFlash(addr);
  _busyWait();

  Serial.print(F("\nbStatus 1: "));Serial.println(_readStatusReg(1),BIN);
  if(_writeEnable())
  {
  	Serial.print(F("Status 1: "));Serial.println(_readStatusReg(1),BIN);
  	// SPI.transfer(FLASH_SECTOR_ERASE);
  	// csHigh();
  	// _spiSendAddr(sectorAddr);
  	csLow();
  	SPI.transfer(0x52);
  	uint8_t *ptr = (uint8_t*)&addr;
    SPI.transfer(ptr[2]);
    SPI.transfer(ptr[1]);
    SPI.transfer(ptr[0]);
    csHigh();

  	uint8_t status;
  	do
  	{
  		Serial.print(F("Status 1: "));Serial.println(_readStatusReg(1),BIN);
  		status = _getWriteStatus();
  		Serial.print(F("Erase status : "));Serial.println(status);
  	}while(status);
  	debugFlashln(F("Sector Erase Done"));
  }
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

 uint32_t Flash::_getNextAddr(uint32_t currentAddr)
 {
  uint32_t nxtAddr = currentAddr + _packetSz;
  //verify that next generated address is the multiple of packetSz
  uint16_t mod = (nxtAddr - _startAddr) % _packetSz;//if mod = 0. valid next address
  if( mod != 0 )
  {
    //find next valid address, start from current address
    nxtAddr = currentAddr;
    do
    {
      nxtAddr++;
      mod = (nxtAddr - _startAddr) % _packetSz;
      if(nxtAddr > (currentAddr + 2*_packetSz))
      {
        break;
      }
    }while(mod);

  }
  //Check memory availability
  if(_flashSz !=0)
  {
    uint32_t maxSize = (_flashSz * 1000000UL)/8;
    if((nxtAddr+_packetSz)>maxSize)
    {
      return 0;
    }
  }
  return nxtAddr;

 }