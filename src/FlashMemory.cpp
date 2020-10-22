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




Flash::Flash(byte chipSelect)
{
  _csPin = chipSelect;  
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

  _writeStatusReg(1,0x00);
  _writeStatusReg(2,0x00);
  // _writeStatusReg(3,0x00);

  // _writeStatusReg(1,0b01000000);
  // _setWriteProtectSchema(INDIVIDUAL_BLOCK);
  // _setUnlock(GLOBAL);
  // uint8_t reg = _readStatusReg(1);
  // _writeStatusReg(1, 0b01100100); // protect first 4k blocs


  // uint8_t reg = _readStatusReg(1);
  // _writeStatusReg(1,reg & 0xC3); //
  // Serial.println(_readStatusReg(1),BIN);
  // _setWriteProtectSchema(INDIVIDUAL_BLOCK);
  // // _setUnlock(GLOBAL);
  // _writeStatusReg(1, 0b01000000);
  //uint8_t reg = _readStatusReg(1);
  // _writeStatusReg(1,0x00); //
  //reg = _readStatusReg(2);
  // _writeStatusReg(2, 0x00);
  
  //reg = _readStatusReg(3);
  // _writeStatusReg(3, 0x00);
}

void Flash::read(uint32_t addr, uint8_t *buf, uint16_t len)
{
	uint8_t *ptr = buf;
	_busyWait();
	csLow();
	SPI.transfer(FLASH_READ_DATA);
	_spiSendAddr(addr);
	for (uint16_t i = 0; i < len; i++)
	{
		ptr[i] = SPI.transfer(0);
	}
	csHigh();
}

void Flash::write(uint32_t addr, uint8_t *buf, uint16_t len)
{
	uint8_t *ptr = buf;
	uint16_t N;
	uint16_t pageRem = 256 - (addr % 256);
	uint16_t offset = 0;
	while(len > 0)
	{
		N = (len <= pageRem) ? len : pageRem;
		_busyWait();
		_writeEnable();
		csLow();
		SPI.transfer(FLASH_PAGE_PROGRAM);
		_spiSendAddr(addr);
		for( uint16_t i = 0; i < N ; i++)
		{
			SPI.transfer(ptr[i + offset]);
		}
		csHigh();
		addr += N;
		offset += N;
		len -= N;
		pageRem = 256;
	}
}

uint8_t *Flash::readPage(uint32_t pageAddr, uint8_t *buf)
{
	Serial.print(F("Read Page : "));Serial.println(pageAddr);
	uint32_t addr = pageAddr<<8;
	read(addr, buf, 256);
}

void Flash::writePage(uint32_t pageAddr, uint8_t *data)
{
	Serial.print(F("Write Page : "));Serial.println(pageAddr);
	uint32_t addr = pageAddr << 8 ;
	write(addr,data,256);
}

void Flash::printPageBytes(byte *pageBuf)
{
  for (byte i = 0; i < 16; i++)
  {
    for (byte j = 0; j < 16; j++)
    {
      Serial.print(pageBuf[i * 16 + j], DEC); Serial.print(" ");
    }
    Serial.println();
  }
}

void Flash::dumpPage(uint32_t pageAddr, uint8_t *buf)
{
	Serial.print(F("Dumping Page : "));Serial.println(pageAddr);
	readPage(pageAddr,buf);
	printPageBytes(buf);
}


void Flash::printBytes(byte *buf, byte len)
{
  for (byte i = 0; i < len; i++)
  {
    debugFlash(buf[i]); debugFlash(" ");
  }
  debugFlashln();
}


void Flash::eraseChip()
{
  debugFlashln(F("Erasing Chip.."));
  _writeEnable();
  csLow();
  SPI.transfer(FLASH_CHIP_ERASE);
  csHigh();
  _writeDisable();
  _busyWait();
  debugFlashln(F("Done"));
}


void Flash::eraseSector(uint32_t addr)
{
  // unlockSector(sectorAddr);
  debugFlash(F("Erasing Sector "));debugFlash(addr);
  _busyWait();

  Serial.print(F("\nbStatus 1: "));Serial.println(_readStatusReg(1),BIN);
  if(_writeEnable())
  {
  	// _setUnlock(SECTOR,addr);
  	// _setUnlock(GLOBAL);
  	Serial.print(F("Status 1: "));Serial.println(_readStatusReg(1),BIN);
  	csLow();
  	SPI.transfer(FLASH_SECTOR_ERASE);
  	// SPI.transfer(0x52);
  	_spiSendAddr(addr);
    csHigh();

  	uint8_t status;
  	do
  	{
  		Serial.print(F("Status 1: "));Serial.println(_readStatusReg(1),BIN);
  		status = _getStatus(WRITING_BIT);
  		Serial.print(F("Erase status : "));Serial.println(status);
  	}while(status);
  	// _setLock(SECTOR,addr);
  	debugFlashln(F("Sector Erase Done"));
  }
}

