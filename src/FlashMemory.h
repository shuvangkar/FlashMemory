#ifndef _H_FLAH_MEM_
#define _H_FLAH_MEM_

#include <Arduino.h>
#include <SPI.h>

#define NON_VOLATILE 0
#define VOLATILE     1

#define BUSY_BIT         0
#define WRITING_BIT      1
#define WPS_BIT          2  

#if defined(ARDUINO_ARCH_STM32)
  SPIClass mySPI;
#elif defined(ARDUINO_ARCH_AVR)
  #define mySPI SPI
#endif  

#define csLow() (digitalWrite(_csPin, LOW))
#define csHigh() (digitalWrite(_csPin, HIGH))

typedef enum memSchema
{
    INDIVIDUAL_BLOCK,
    FIRST_BLOCK,
    SECOND_BLOCK,
}schema_t;

typedef enum memSize
{
    SECTOR,
    BLOCK,
    GLOBAL,
}memSize_t;


class Flash
{
  public:
    Flash(uint8_t cs);
    Flash(uint8_t mosi, uint8_t miso, uint8_t sck, uint8_t cs);
    void begin();
    void read(uint32_t addr, uint8_t *buf, uint16_t len);
    void write(uint32_t addr, uint8_t *buf, uint16_t len);
    uint8_t *readPage(uint32_t pageAddr, uint8_t *buf);
    void writePage(uint32_t pageAddr, uint8_t *data);


    void printPageBytes(byte *pageBuf);
    void printBytes(byte *buf, byte len);
    void dumpPage(uint32_t pageAddr, uint8_t *buf);
    
    void eraseChip();
    void eraseSector(uint32_t addr);
    
  private:
    byte _csPin;

    void  _softReset();
    void _powerDown();
    void _releasePowerDown();

   
    bool  _writeEnable(uint8_t memType = NON_VOLATILE);
    void  _writeDisable();

    void _writeStatusReg(uint8_t reg,uint8_t value, uint8_t memType = NON_VOLATILE);
    uint8_t _readStatusReg(uint8_t regNo);

     bool _getStatus(uint8_t bit);
    void _busyWait();

    void _spiSendAddr(uint32_t addr);

    void _setWriteProtectSchema(schema_t schema);
    void _setLock(memSize_t memSize, uint32_t bAddress = 0);
    void _setUnlock(memSize_t memSize, uint32_t bAddress = 0);
    bool _readSectorLock(uint32_t addr);

    uint32_t _getNextAddr(uint32_t currentAddr);// the function returns next address for similar type of packet,return zero if no memory available
};
#endif

