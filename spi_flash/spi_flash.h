
#include "stm32f1xx_hal.h"
#include "main.h"
#include <stdbool.h>

typedef enum {FLASH_PAGE_SIZE_256, FLASH_PAGE_SIZE_264} FlashPageSize;
typedef enum {FLASH_DENSITY_4MB, FLASH_DENSITY_UNKNOWN} FlashDensity;

typedef struct
{
    bool isReady;
    bool compareResultError;
    bool protectionEnabled;
    bool sectorLockdownEnabled;
    
    bool eraseOrProgramError;
    bool eraseIsSuspended;
    bool bufferOneIsSuspended;
    bool bufferTwoIsSuspended;
    
    FlashPageSize pageSize;
    FlashDensity density;
    
    
} FlashDeviceStatus;

typedef struct
{
    SPI_HandleTypeDef* flashSpiPort;
    GPIO_TypeDef* flashSS_GPIO_Port;
    uint16_t flashSS_GPIO_Pin;
    
    bool allowHighSpeed;
    
    FlashDeviceStatus status;
    FlashPageSize prefferedPageSize;
    
} SPI_FlashHeader;


int SPI_FlashInit(SPI_FlashHeader* flash);
int SPI_FlashBlockWrite(SPI_FlashHeader* flash, uint32_t blockNumber, const uint8_t* data);
void SPI_FlashBlockRead(SPI_FlashHeader* flash, uint32_t blockNumber, uint8_t* data);
