/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "flash_disk_access_wrapper.h"
#include <stdint.h>
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	
	(void)pdrv;
	
	FlashDiskAccessWrapper* drive = flashDiskWrapperLocate();
	if(!drive->isInited)
	{
		stat = STA_NOINIT;
	}
	else
	{
		stat = 0;
	}
	
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	
	(void)pdrv;
	
	FlashDiskAccessWrapper* drive = flashDiskWrapperLocate();
	if(!drive->isInited)
	{
		stat = STA_NOINIT;
	}
	else
	{
		stat = 0;
	}
	
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	(void)pdrv;
	
	FlashDiskAccessWrapper* drive = flashDiskWrapperLocate();
	if(flashDiskAccessWrapperLock(drive))
			return RES_NOTRDY;
	if(!drive->flash.status.isReady)
	{
			flashDiskAccessWrapperUnlock(drive);  
			return RES_NOTRDY;
	}
	SPI_FlashBlockRead(&drive->flash, sector, buff);
	flashDiskAccessWrapperUnlock(drive);  

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{

	switch (cmd) {
	case GET_SECTOR_SIZE:
		*(uint16_t*)buff = 512;
		return RES_OK;

	case GET_BLOCK_SIZE:
		*(uint16_t*)buff = 512;
		return RES_OK;

	case CTRL_SYNC:
		return RES_OK;
		// Process of the command the USB drive
	default:
		return RES_PARERR;
	}
}

