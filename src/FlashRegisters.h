/***********Register Map******************/
#define FLASH_WRITE_ENABLE       0x06
#define FLASH_WR_ENA_VOLATILE    0x50
#define FLASH_WRITE_DISABLE      0x04
#define FLASH_READ_STATUS_1      0x05
#define FLASH_READ_STATUS_2      0x35
#define FLASH_READ_STATUS_3      0x15
#define FLASH_WRITE_STATUS_1     0x01
#define FLASH_WRITE_STATUS_2     0x31
#define FLASH_WRITE_STATUS_3     0x11

#define FLASH_READ_DATA          0x03
#define FLASH_PAGE_PROGRAM       0x02
#define FLASH_CHIP_ERASE         0xc7
#define FLASH_SECTOR_ERASE       0x20

#define BLOCK_SECTOR_LOCK		 	0x36
#define BLOCK_SECTOR_UNLOCK		 	0x39
#define READ_BLOCK_SECTOR_LOCK		0x3d
#define GLOBAL_BLOCK_SECTOR_LOCK 	0x7e
#define GLOBAL_BLOCK_SECTOR_UNLOCK 	0x98