#ifndef _H_FLAH_MEM_
#define _H_FLAH_MEM_
#include <Arduino.h>



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
    void eraseSector(uint32_t sectorAddr);

    uint8_t _readStatus(uint8_t regNo);

    bool unlockSector(uint32_t sector);
    uint8_t readSectorLock(uint32_t sectorAddr);
  private:
    byte _csPin;
    uint32_t _startAddr;
    uint16_t _packetSz;
    byte _flashSz = 0;;
    // byte *_buf;
    void _busyWait();
    void _writeEnable();
    void _writeDisable();
    bool _getWriteEnableStatus();
    void _setIndividualSectorLock();
    void _setGlobalSectorUnlock();
    void _writeEnableVolatile();
    
    void _writeStatus(uint8_t regNo,uint8_t reg);

    void _chipErase();
    void _erasePage(uint16_t pageNum);
    bool _isbusy = true;

    uint32_t _getNextAddr(uint32_t currentAddr);// the function returns next address for similar type of packet,return zero if no memory available
};
#endif

