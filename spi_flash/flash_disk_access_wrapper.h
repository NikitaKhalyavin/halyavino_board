#ifndef FLASH_DISK_ACCESS_WRAPPER_H_
#define FLASH_DISK_ACCESS_WRAPPER_H_

#include "spi_flash.h"
#include <stdbool.h>

typedef struct 
{
    SPI_FlashHeader flash;
    bool isBusy;
    bool isInited;
} FlashDiskAccessWrapper;

FlashDiskAccessWrapper* flashDiskWrapperLocate(void); 
void flashDiskAccessWrapperInit(FlashDiskAccessWrapper* wrapper);

bool flashDiskAccessWrapperLock(FlashDiskAccessWrapper* wrapper);
void flashDiskAccessWrapperUnlock(FlashDiskAccessWrapper* wrapper);


#endif // FLASH_DISK_ACCESS_WRAPPER_H_
