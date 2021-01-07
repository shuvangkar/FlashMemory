#include "FlashMemory.h"
#include "FlashRegisters.h"

#define FLASH_DEBUG
#ifdef FLASH_DEBUG
#define debugFlash(...) Serial.print(__VA_ARGS__)
#define debugFlashln(...) Serial.println(__VA_ARGS__)
#else
#define debugFlash(...) __VA_ARGS__
#define debugFlashln(...) __VA_ARGS__
#endif

#if defined(ARDUINO_ARCH_STM32)
SPIClass mySPI;
#elif defined(ARDUINO_ARCH_AVR)
#define mySPI SPI
#elif defined(__STM32F1__)
SPIClass mySPI(2);
#else
#error " No Platform found"
#endif

#define csLow() (digitalWrite(_csPin, LOW))
#define csHigh() (digitalWrite(_csPin, HIGH))

Flash::Flash(uint8_t cs)
{
	_csPin = cs;
}

Flash::Flash(uint8_t cs, uint8_t hold)
{
	_csPin = cs;
	_holdPin = hold;
}

Flash::Flash(uint32_t mosi, uint32_t miso, uint32_t sck, uint32_t cs)
{
#if defined(ARDUINO_ARCH_STM32)
	mySPI.setMOSI(mosi);
	mySPI.setMISO(miso);
	mySPI.setSCLK(sck);
#endif
	_csPin = cs;
}

Flash::Flash(uint32_t mosi, uint32_t miso, uint32_t sck, uint32_t cs, uint32_t hold)
{
	Flash(mosi,miso,sck,cs);
// #if defined(ARDUINO_ARCH_STM32)
// 	mySPI.setMOSI(mosi);
// 	mySPI.setMISO(miso);
// 	mySPI.setSCLK(sck);
// #endif
// 	_csPin = cs;
	_holdPin = hold;
}

void Flash::begin(uint32_t spiSpeed)
{
	pinMode(_csPin, OUTPUT);
	csHigh();
	if (_holdPin != 255)
	{
		pinMode(_holdPin, OUTPUT);
		digitalWrite(_holdPin, HIGH);
	}

	// mySPI.begin();
	// mySPI.setDataMode(SPI_MODE0);
	// mySPI.setBitOrder(MSBFIRST);
#if defined(ARDUINO_ARCH_AVR)
	// mySPI.setClockDivider(SPI_CLOCK_DIV4);
	mySPI.beginTransaction(SPISettings(spiSpeed, MSBFIRST, SPI_MODE0));
	mySPI.begin();
#elif defined(ARDUINO_ARCH_STM32)
	// mySPI.setClockDivider(SPI_CLOCK_DIV64);
	mySPI.beginTransaction(_csPin, SPISettings(spiSpeed, MSBFIRST, SPI_MODE0));
	mySPI.begin();
#elif defined(__STM32F1__)
	// mySPI.setClockDivider(SPI_CLOCK_DIV16);
	mySPI.beginTransaction(_csPin, SPISettings(spiSpeed, MSBFIRST, SPI_MODE0));
#endif

	delay(1);

	_writeStatusReg(1, 0x00);
	_writeStatusReg(2, 0x00);
	// _writeStatusReg(3,0x00);

	// // _writeStatusReg(1,0b01000000);
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

void Flash::hold()
{
	digitalWrite(_holdPin, LOW);
	// Serial.println(F("Flash hold"));
}

void Flash::holdRelease()
{
	digitalWrite(_holdPin, HIGH);
	// Serial.println(F("Flash Release"));
}

void Flash::read(uint32_t addr, uint8_t *buf, uint16_t len)
{
	uint8_t *ptr = buf;
	_busyWait();
	csLow();
	mySPI.transfer(FLASH_READ_DATA);
	_spiSendAddr(addr);
	for (uint16_t i = 0; i < len; i++)
	{
		ptr[i] = mySPI.transfer(0);
	}
	csHigh();
}

void Flash::write(uint32_t addr, uint8_t *buf, uint16_t len)
{
	uint8_t *ptr = buf;
	uint16_t N;
	uint16_t pageRem = 256 - (addr % 256);
	uint16_t offset = 0;
	while (len > 0)
	{
		N = (len <= pageRem) ? len : pageRem;
		_busyWait();
		_writeEnable();
		csLow();
		mySPI.transfer(FLASH_PAGE_PROGRAM);
		_spiSendAddr(addr);
		for (uint16_t i = 0; i < N; i++)
		{
			mySPI.transfer(ptr[i + offset]);
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
	Serial.print(F("Read Page : "));
	Serial.println(pageAddr);
	uint32_t addr = pageAddr << 8;
	read(addr, buf, 256);
}

void Flash::writePage(uint32_t pageAddr, uint8_t *data)
{
	Serial.print(F("Write Page : "));
	Serial.println(pageAddr);
	uint32_t addr = pageAddr << 8;
	write(addr, data, 256);
}

void Flash::printPageBytes(byte *pageBuf)
{
	for (byte i = 0; i < 16; i++)
	{
		for (byte j = 0; j < 16; j++)
		{
			Serial.print(pageBuf[i * 16 + j], DEC);
			Serial.print(" ");
		}
		Serial.println();
	}
}

void Flash::dumpPage(uint32_t pageAddr, uint8_t *buf)
{
	Serial.print(F("Dumping Page : "));
	Serial.println(pageAddr);
	readPage(pageAddr, buf);
	printPageBytes(buf);
}

void Flash::printBytes(byte *buf, byte len)
{
	for (byte i = 0; i < len; i++)
	{
		debugFlash(buf[i]);
		debugFlash(" ");
	}
	debugFlashln();
}

void Flash::eraseChip()
{
	debugFlashln(F("Erasing Chip.."));
	_writeEnable();
	csLow();
	// mySPI.transfer(FLASH_CHIP_ERASE);
	mySPI.transfer(0x60);
	csHigh();
	_writeDisable();
	_busyWait();
	debugFlashln(F("Done"));
}

void Flash::eraseSector(uint32_t addr)
{
	// unlockSector(sectorAddr);
	debugFlash(F("Erasing Sector "));
	debugFlash(addr);
	_busyWait();

	Serial.print(F("\nbStatus 1: "));
	Serial.println(_readStatusReg(1), BIN);
	if (_writeEnable())
	{
		// _setUnlock(SECTOR,addr);
		// _setUnlock(GLOBAL);
		Serial.print(F("Status 1: "));
		Serial.println(_readStatusReg(1), BIN);
		csLow();
		mySPI.transfer(FLASH_SECTOR_ERASE);
		// mySPI.transfer(0x52);
		_spiSendAddr(addr);
		csHigh();

		uint8_t status;
		do
		{
			Serial.print(F("Status 1: "));
			Serial.println(_readStatusReg(1), BIN);
			status = _getStatus(WRITING_BIT);
			Serial.print(F("Erase status : "));
			Serial.println(status);
		} while (status);
		// _setLock(SECTOR,addr);
		debugFlashln(F("Sector Erase Done"));
	}
}

/****************************Flash I/O functions***********************************/

uint8_t Flash::_readStatusReg(uint8_t regNo)
{
	csLow();
	switch (regNo)
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

void Flash::_writeStatusReg(uint8_t reg, uint8_t value, uint8_t memType)
{
	_busyWait();
	_writeEnable(memType);
	csLow();
	switch (reg)
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
	return ((reg1 & (1 << bit)) >> bit);
}

bool Flash::_writeEnable(uint8_t memType)
{
	/*
	volatile status register will affetct WEL register
	non-volatile write enable change WEL register. 
	*/
	csLow();
	switch (memType)
	{
	case NON_VOLATILE:
		mySPI.transfer(FLASH_WRITE_ENABLE);
		break;
	case VOLATILE:
		mySPI.transfer(FLASH_WR_ENA_VOLATILE);
		break;
	}
	csHigh();

	if (memType == NON_VOLATILE)
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
	} while (busy);
}

void Flash::_spiSendAddr(uint32_t addr)
{
	uint8_t *ptr = (uint8_t *)&addr;
	mySPI.transfer(ptr[2]);
	mySPI.transfer(ptr[1]);
	mySPI.transfer(ptr[0]);
}

void Flash::_setWriteProtectSchema(schema_t schema)
{
	uint8_t reg;
	switch (schema)
	{
	case INDIVIDUAL_BLOCK:
		Serial.println(F("Individual Block Lock"));
		reg = _readStatusReg(3);
		_writeStatusReg(3, reg | (1 << WPS_BIT));
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
	switch (memSize)
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
	switch (memSize)
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