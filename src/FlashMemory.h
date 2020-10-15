#ifndef _H_FLAH_MEM_
#define _H_FLAH_MEM_
#include <Arduino.h>



#define NON_VOLATILE 0
#define VOLATILE     1

#define BUSY_BIT         0
#define WRITING_BIT      1
#define WPS_BIT          2   



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

#define csLow() (digitalWrite(_csPin, LOW))
#define csHigh() (digitalWrite(_csPin, HIGH))


class Flash
{
  public:
    Flash(byte chipSelect);
    Flash(byte CS, uint32_t startAddr, uint16_t packetSz);
    void begin();
    uint8_t *readPage(uint32_t pageNo, uint8_t *pageBuffer);

    void setFlashSize(byte sizeMbit);
    void printPage(unsigned int pageNum);
    void printPageBytes(byte *pageBuf);
    void printBytes(byte *buf, byte len);
    bool writeBytes(uint32_t logicalAddr, byte *data, byte length);
    bool readBytes(uint32_t logicalAddr, byte *data, byte length);
    
    byte  *_readPage(unsigned int pageNum, byte *page_buffer);
    void _writePage(unsigned int PageNum, byte *pageBuf);
    void eraseChipData();
    void eraseSector(uint32_t addr);


    uint8_t _readStatusReg(uint8_t regNo);
    bool _readSectorLock(uint32_t addr);
    void _setUnlock(memSize_t memSize, uint32_t bAddress = 0);
  private:
    byte _csPin;
    uint32_t _startAddr;
    uint16_t _packetSz;
    byte _flashSz = 0;;

    void _chipErase();
    bool _isbusy = true;

    // uint8_t _readStatusReg(uint8_t regNo);
    bool _getWriteStatus();
    bool _getStatus(uint8_t bit);
    bool  _writeEnable(uint8_t memType = NON_VOLATILE);
    void  _writeDisable();
    void _writeStatusReg(uint8_t regNo,uint8_t reg);
    void _busyWait();

    void _spiSendAddr(uint32_t addr);
    void _setWriteProtectSchema(schema_t schema);
    void _setLock(memSize_t memSize, uint32_t bAddress = 0);
    // void _setUnlock(memSize_t memSize, uint32_t bAddress = 0);
    // bool _readSectorLock(uint32_t addr);

    uint32_t _getNextAddr(uint32_t currentAddr);// the function returns next address for similar type of packet,return zero if no memory available
};
#endif

