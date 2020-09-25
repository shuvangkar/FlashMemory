#ifndef _H_FLAH_MEM_
#define _H_FLAH_MEM_
#include <Arduino.h>



class Flash
{
  public:
    Flash(byte chipSelect);
    Flash(byte CS, uint32_t startAddr, uint16_t packetSz);
    void setFlashSize(byte sizeMbit);
    void printPage(unsigned int pageNum);
    void printPageBytes(byte *pageBuf);
    void printBytes(byte *buf, byte len);
    bool writeBytes(uint32_t logicalAddr, byte *data, byte length);
    bool readBytes(uint32_t logicalAddr, byte *data, byte length);
    void begin();
    byte  *_readPage(unsigned int pageNum, byte *page_buffer);
    void _writePage(unsigned int PageNum, byte *pageBuf);
    void eraseChipData();
  private:
    byte _csPin;
    uint32_t _startAddr;
    uint16_t _packetSz;
    byte _flashSz = 0;;
    // byte *_buf;
    void _busyWait();
    
    void _chipErase();
    void _erasePage(uint16_t pageNum);
    uint32_t _getNextAddr(uint32_t currentAddr);// the function returns next address for similar type of packet,return zero if no memory available
};
#endif

