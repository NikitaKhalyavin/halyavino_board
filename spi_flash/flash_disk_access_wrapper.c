#include "flash_disk_access_wrapper.h"
#include "critical.h"

extern SPI_HandleTypeDef hspi2;

FlashDiskAccessWrapper* flashDiskWrapperLocate()
{
	static FlashDiskAccessWrapper disk;
	return &disk;
}

void flashDiskAccessWrapperInit(FlashDiskAccessWrapper* wrapper)
{
	wrapper->isBusy = true;
	wrapper->isInited = false;
	
	wrapper->flash.flashSpiPort = &hspi2;
  wrapper->flash.flashSS_GPIO_Pin = SPI2_NSS_Pin;
  wrapper->flash.flashSS_GPIO_Port = SPI2_NSS_GPIO_Port;
  wrapper->flash.allowHighSpeed = false;
  wrapper->flash.prefferedPageSize = FLASH_PAGE_SIZE_256;
    
	bool res = SPI_FlashInit(&wrapper->flash);
	if(!res)
			wrapper->isInited = true;
	wrapper->isBusy = false;
}

bool flashDiskAccessWrapperLock(FlashDiskAccessWrapper* wrapper)
{
	bool result;
	ENTER_CRITICAL()
	if(wrapper->isBusy)
		result = true;
	else
	{
		result = false;
		wrapper->isBusy = true;
	}
	EXIT_CRITICAL()
	return result;
}
void flashDiskAccessWrapperUnlock(FlashDiskAccessWrapper* wrapper)
{
	wrapper->isBusy = false;
}

