#include "FlashMemory.h"
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
#define FLASH_WRITE_ENABLE       0x06
#define FLASH_WRITE_DISABLE      0x04
#define FLASH_READ_STATUS_1      0x05
#define FLASH_READ_STATUS_2      0x35
#define FLASH_READ_STATUS_3      0x15
#define FLASH_WRITE_STATUS_1     0x01
#define FLASH_WRITE_STATUS_2     0x31
#define FLASH_WRITE_STATUS_3     0x11
#define FLASH_READ_DATA          0x03

#define WB_CHIP_ERASE         0xc7
#define WB_SECTOR_ERASE       0x20
#define WB_READ_STATUS_REG_1  0x05
#define WB_READ_DATA          0x03
#define WB_PAGE_PROGRAM       0x02
#define WB_JEDEC_ID           0x9f
#define SPIFLASH_STATUSWRITE      0x01        // write status register
/*********************************/

#define chipEnable() (digitalWrite(_csPin, LOW))
#define chipDisable() (digitalWrite(_csPin, HIGH))

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
  chipDisable();

  SPI.begin();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  delay(1);
  // _setIndividualSectorLock();
  // _setGlobalSectorUnlock();
  // SPI.transfer(0xFF);
}

void Flash::_writeEnable()
{
  chipEnable();
  SPI.transfer(FLASH_WRITE_ENABLE);
  chipDisable();
}
void Flash::_writeDisable()
{
  chipEnable();
  SPI.transfer(FLASH_WRITE_DISABLE);
  chipDisable();
}
void Flash::_writeEnableVolatile()
{
  chipEnable();
  SPI.transfer(0x50);
  chipDisable();
}
uint8_t Flash::_readStatus(uint8_t regNo)
{
  chipEnable();
  switch(regNo)
  {
    case 1:
      SPI.transfer(FLASH_READ_STATUS_1);
    break;
    case 2:
      SPI.transfer(FLASH_READ_STATUS_2);
    break;
    case 3:
      SPI.transfer(FLASH_READ_STATUS_3);
    break;
  }
  uint8_t reg = SPI.transfer(0);
  chipDisable();
  return reg;
}
void Flash::_writeStatus(uint8_t regNo,uint8_t reg)
{
  _writeEnable();
  chipEnable();
  switch(regNo)
  {
    case 1:
      SPI.transfer(FLASH_WRITE_STATUS_1);
    break;
    case 2:
      SPI.transfer(FLASH_WRITE_STATUS_2);
    break;
    case 3:
      SPI.transfer(FLASH_WRITE_STATUS_3);
    break;
  }
  SPI.transfer(reg);
  chipDisable();
}

void Flash::_busyWait()
{
  // if(_isbusy)
  // {
    uint8_t reg1;
    do
    {
      reg1 = _readStatus(1);
      _isbusy = reg1 & 0x01;
      // Serial.println(reg1,BIN);
    }while(_isbusy);
  // }
}

bool Flash::_getWriteEnableStatus()
{
  uint8_t reg1 = _readStatus(1);
  if(reg1 & 0b00000010)
  {
    return true;
  }
  else 
  {
    return false;
  }
}

 void Flash::_setIndividualSectorLock()
 {
   uint8_t reg = _readStatus(3);
   _writeStatus(3, reg | 0x04) ; //set WPS = 1;
 }
void Flash::_setGlobalSectorUnlock()
{
  _writeEnable();
  chipEnable();
  SPI.transfer(0x98);
  chipDisable();
}
uint8_t *Flash::readPage(uint32_t pageNo, uint8_t *pageBuffer)
{
  _busyWait();
  chipEnable();
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
  chipDisable();
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
  SPI.transfer(WB_READ_DATA);
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
  _writeEnable();
  chipEnable();
  uint8_t *p = (uint8_t*)&PageNum;
  SPI.transfer(WB_PAGE_PROGRAM);
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




// void Flash::_busyWait()
// {
//   // digitalWrite(_csPin, HIGH);
//   digitalWrite(_csPin, LOW);
//   SPI.transfer(WB_READ_STATUS_REG_1);
//   while (SPI.transfer(0) & 1) {};
//   digitalWrite(_csPin, HIGH);
// }

void Flash::_chipErase()
{
  _writeEnable();
  chipEnable();
  SPI.transfer(WB_CHIP_ERASE);
  chipDisable();
  _writeDisable();
  _busyWait();

}
bool Flash::unlockSector(uint32_t sector)
{
  _writeEnable();
  Serial.print(F("Enable : ")); Serial.println(_getWriteEnableStatus());
  chipEnable();
  SPI.transfer(0x39);
  SPI.transfer((uint8_t)sector>>16);
  SPI.transfer((uint8_t)sector>>8);
  SPI.transfer((uint8_t)sector);
  chipDisable();
  _busyWait();
}
uint8_t Flash::readSectorLock(uint32_t sectorAddr)
{
  chipEnable();
  SPI.transfer(0x3d);
  SPI.transfer(sectorAddr>>16);
  SPI.transfer(sectorAddr>>8);
  SPI.transfer(sectorAddr);
  uint8_t reg = SPI.transfer(0);
  chipDisable();
  _busyWait();
  return reg;

}

void Flash::eraseSector(uint32_t sectorAddr)
{
  // unlockSector(sectorAddr);
  debugFlash(F("Erasing Sector "));debugFlash(sectorAddr);
  _busyWait();
  _writeEnable();
  chipEnable();
  Serial.print(F("TP1 : "));Serial.println(_readStatus(1),BIN);
  SPI.transfer(WB_SECTOR_ERASE);
  SPI.transfer((uint8_t)sectorAddr >> 16);
  SPI.transfer((uint8_t)sectorAddr >> 8);
  SPI.transfer((uint8_t)sectorAddr);
  Serial.print(F("TP2 : "));Serial.println(_readStatus(1),BIN);
  chipDisable();
  _writeDisable();
  Serial.print(F("TP3 : "));Serial.println(_readStatus(1),BIN);
  debugFlashln(F("Sector Erase Done"));
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