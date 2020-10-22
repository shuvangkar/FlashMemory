#include "FlashMemory.h"
#include "FlashRegisters.h"

uint8_t Flash::_readStatusReg(uint8_t regNo)
{
  csLow();
  switch(regNo)
  {
    case 1:
      mySPI.transfer(FLASH_READ_STATUS_1);
    break;
    case 2:
      mySPI.transfer(FLASH_READ_STATUS_2);
    break;
    case 3:
      mySPI.transfer(FLASH_READ_STATUS_3);
    break;
  }
  uint8_t reg = mySPI.transfer(0);
  csHigh();
  return reg;
}

void Flash::_writeStatusReg(uint8_t reg,uint8_t value, uint8_t memType)
{
  _busyWait();
  _writeEnable(memType);
  csLow();
  switch(reg)
  {
    case 1:
      mySPI.transfer(FLASH_WRITE_STATUS_1);
    break;
    case 2:
      mySPI.transfer(FLASH_WRITE_STATUS_2);
    break;
    case 3:
      mySPI.transfer(FLASH_WRITE_STATUS_3);
    break;
  }
  mySPI.transfer(value);
  csHigh();
  _writeDisable();
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
			mySPI.transfer(FLASH_WRITE_ENABLE);
		break;
		case VOLATILE:
			mySPI.transfer(FLASH_WR_ENA_VOLATILE);
		break;
	}
	csHigh();

	if(memType == NON_VOLATILE)
	{
		return _getStatus(WRITING_BIT);
	}
	return 0;
}

void Flash::_writeDisable()
{
  csLow();
  mySPI.transfer(FLASH_WRITE_DISABLE);
  csHigh();
}

void Flash::_busyWait()
{
    bool busy = true;
    do
    {
     busy = _getStatus(BUSY_BIT);
     // Serial.println(busy);
    }while(busy);
}

void Flash::_spiSendAddr(uint32_t addr)
{
	uint8_t *ptr = (uint8_t*)&addr;
	mySPI.transfer(ptr[2]);	
    mySPI.transfer(ptr[1]);
    mySPI.transfer(ptr[0]);
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
			mySPI.transfer(BLOCK_SECTOR_LOCK);
			_spiSendAddr(bAddress);
		break;
		case GLOBAL:
			mySPI.transfer(GLOBAL_BLOCK_SECTOR_LOCK);
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
			mySPI.transfer(BLOCK_SECTOR_UNLOCK);
			_spiSendAddr(bAddress);
		break;
		case GLOBAL:
			mySPI.transfer(GLOBAL_BLOCK_SECTOR_UNLOCK);
		break;
	}
	csHigh();
}

bool Flash::_readSectorLock(uint32_t addr)
{
	csLow();
	mySPI.transfer(READ_BLOCK_SECTOR_LOCK);
	_spiSendAddr(addr);
	uint8_t reg = mySPI.transfer(0);
	csHigh();
	return reg;
}