#include "FlashMemory.h"
#include <SPI.h>
#include <SPI.h>
#include "FlashRegisters.h"


/***********Bit masking*****************/
#define BUSY_BIT 	0
#define WEL_BIT 	1






// #define csLow() (digitalWrite(_csPin, LOW))
// #define csHigh() (digitalWrite(_csPin, HIGH))

uint8_t Flash::_readStatusReg(uint8_t regNo)
{
  csLow();
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
  csHigh();
  return reg;
}

void Flash::_writeStatusReg(uint8_t regNo,uint8_t reg)
{
  // _writeEnable();
  _writeEnable(VOLATILE);
  csLow();
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
  csHigh();

  csLow();
  SPI.transfer(reg);
  csHigh();
  // _writeDisable();
}

bool Flash::_getWriteStatus()
{
  uint8_t reg1 = _readStatusReg(1);
  return ((reg1 & (1<<WEL_BIT))>>WEL_BIT);
}

bool Flash::_getStatus(uint8_t bit)
{
	uint8_t reg1 = _readStatusReg(1);
	return ((reg1 & (1<<bit))>>bit);
}
bool  Flash::_writeEnable(uint8_t memType)
{
	/*
	volatile status register will affetct WEL register
	non-volatile write enable change WEL register. 
	*/
	csLow();
	switch(memType)
	{
		case NON_VOLATILE:
			SPI.transfer(FLASH_WRITE_ENABLE);
		break;
		case VOLATILE:
			SPI.transfer(FLASH_WR_ENA_VOLATILE);
		break;
	}
	csHigh();

	if(memType == NON_VOLATILE)
	{
		return _getWriteStatus();
	}
	return 0;
}

void Flash::_writeDisable()
{
  csLow();
  SPI.transfer(FLASH_WRITE_DISABLE);
  csHigh();
}

void Flash::_busyWait()
{
  // if(_isbusy)
  // {
    uint8_t reg1;
    do
    {
      reg1 = _readStatusReg(1);
      _isbusy = reg1 & (1<<BUSY_BIT);
      // Serial.println(reg1,BIN);
    }while(_isbusy);
  // }
}

void Flash::_spiSendAddr(uint32_t addr)
{
	uint8_t *ptr = (uint8_t*)&addr;
	SPI.transfer(ptr[2]);
    SPI.transfer(ptr[1]);
    SPI.transfer(ptr[0]);
}

void Flash::_setWriteProtectSchema(schema_t schema)
{
	uint8_t reg;
	switch(schema)
	{
		case INDIVIDUAL_BLOCK:
			Serial.println(F("Individual Block Lock"));
			reg = _readStatusReg(3);
			_writeStatusReg(3, reg | (1<<WPS_BIT));
		break;
		case FIRST_BLOCK:
		break;
		case SECOND_BLOCK:
		break;
	}
}

void Flash::_setLock(memSize_t memSize, uint32_t bAddress)
{
	_writeEnable(VOLATILE);
	csLow();
	switch(memSize)
	{
		case SECTOR:
		case BLOCK:
			SPI.transfer(BLOCK_SECTOR_LOCK);
			_spiSendAddr(bAddress);
		break;
		case GLOBAL:
			SPI.transfer(GLOBAL_BLOCK_SECTOR_LOCK);
		break;
	}
	csHigh();
}

void Flash::_setUnlock(memSize_t memSize, uint32_t bAddress)
{
	_writeEnable(VOLATILE);
	// _writeEnable();
	csLow();
	switch(memSize)
	{
		case SECTOR:
		case BLOCK:
			SPI.transfer(BLOCK_SECTOR_UNLOCK);
			_spiSendAddr(bAddress);
		break;
		case GLOBAL:
			SPI.transfer(GLOBAL_BLOCK_SECTOR_UNLOCK);
		break;
	}
	csHigh();
}

bool Flash::_readSectorLock(uint32_t addr)
{
	csLow();
	SPI.transfer(READ_BLOCK_SECTOR_LOCK);
	_spiSendAddr(addr);
	uint8_t reg = SPI.transfer(0);
	csHigh();
	return reg;
}