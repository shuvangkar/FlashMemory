#ifndef _H_FLAH_MEM_
#define _H_FLAH_MEM_
#include <Arduino.h>



class Flash
{
  public:
    Flash(byte chipSelect);
    void printPage(unsigned int pageNum);
    void printPageBytes(byte *pageBuf);
    void printBytes(byte *buf, byte len);
    bool writeBytes(uint32_t logicalAddr, byte *data, byte length);
    bool readBytes(uint32_t logicalAddr, byte *data, byte length);
    void begin();
    void eraseChipData();
  private:
    byte _csPin;
    byte *_buf;
    void _busyWait();
    void _readPage(unsigned int pageNum, byte *page_buffer);
    void _writePage(unsigned int PageNum, byte *pageBuf);
    void _chipErase();
    void _erasePage(uint16_t pageNum);
};
#endif

