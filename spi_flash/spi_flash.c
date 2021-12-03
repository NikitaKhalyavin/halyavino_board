
#include "spi_flash.h"
// defining the comand's opcodes-------------------------------------------------------------------------------------------------------
//*************************************************************************************************************************************

//main state commands
#define STATUS_REGISTER_READ 0xd7
#define MANUFACTURER_AND_DEVICE_ID_READ 0x9f

#define RESET_FIRST_BYTE 0xf0
#define RESET_SECOND_BYTE 0x00
#define RESET_THIRD_BYTE 0x00
#define RESET_FOURTH_BYTE 0x00

#define SET_PAGE_SIZE_FIRST_BYTE 0x3d
#define SET_PAGE_SIZE_SECOND_BYTE 0x2a
#define SET_PAGE_SIZE_THIRD_BYTE 0x80
#define SET_PAGE_SIZE_256_FOURTH_BYTE 0xa6
#define SET_PAGE_SIZE_264_FOURTH_BYTE 0xa7


//read commands

#define CONTINUOUS_ARRAY_READ_LOW_POWER 0x01
#define CONTINUOUS_ARRAY_READ_LOW_FREQUENCY 0x03
#define CONTINUOUS_ARRAY_READ_HIGH_FREQUENCY 0x0b
#define CONTINUOUS_ARRAY_READ_HIGH_FREQUENCY_2 0x1b

#define BUFFER_1_READ_LOW_FREQUENCY 0xd1
#define BUFFER_2_READ_LOW_FREQUENCY 0xd3
#define BUFFER_1_READ_HIGH_FREQUENCY 0xd4
#define BUFFER_2_READ_HIGH_FREQUENCY 0xd6

#define MEMORY_TO_BUFFER_1_TRANSFER 0x53
#define MEMORY_TO_BUFFER_2_TRANSFER 0x55

#define MEMORY_WITH_BUFFER_1_COMPARE 0x60
#define MEMORY_WITH_BUFFER_2_COMPARE 0x61

//write commands

#define BUFFER_1_WRITE 0x84
#define BUFFER_2_WRITE 0x87

#define BUFFER_1_TO_MEMORY_WRITE_PAGE_WITH_ERASE 0x83
#define BUFFER_2_TO_MEMORY_WRITE_PAGE_WITH_ERASE 0x86
#define BUFFER_1_TO_MEMORY_WRITE_PAGE_WITHOUT_ERASE 0x88
#define BUFFER_2_TO_MEMORY_WRITE_PAGE_WITHOUT_ERASE 0x89

#define PAGE_ERASE 0x81
#define BLOCK_ERASE 0x50
#define SECTOR_ERASE 0x7c

#define CHIP_ERASE_FIRST_BYTE 0xc7
#define CHIP_ERASE_SECOND_BYTE 0x94
#define CHIP_ERASE_THIRD_BYTE 0x80
#define CHIP_ERASE_FOURTH_BYTE 0x9a

#define PROGRAM_OR_ERASE_SUSPEND 0xb0
#define PROGRAM_OR_ERASE_RESUME 0xd0

//*************************************************************************************************************************************
// end of opcodes defining-------------------------------------------------------------------------------------------------------------


//local typedefs-----------------------------------------------------------------------------------------------------------------------
//*************************************************************************************************************************************

typedef enum {FLASH_OK, FLASH_COMMAND_SUCCESS, FLASH_COMMAND_ERROR} FlashFunctionStatus;

typedef enum {BUFFER_1, BUFFER_2} BufferName;

//*************************************************************************************************************************************
//end of local typedefs----------------------------------------------------------------------------------------------------------------


//SPI transfer functions---------------------------------------------------------------------------------------------------------------
//*************************************************************************************************************************************

static uint8_t SPI_Transfer(SPI_FlashHeader* flash, uint8_t sendingByte);
static void SPI_Transmit(SPI_FlashHeader* flash, uint8_t sendingByte);
static uint8_t SPI_Receive(SPI_FlashHeader* flash);
static void flashSelect(SPI_FlashHeader* flash);
static void flashDeselect(SPI_FlashHeader* flash);

//*************************************************************************************************************************************
//end of SPI transfer functions--------------------------------------------------------------------------------------------------------


//flash commands realisation-----------------------------------------------------------------------------------------------------------
//*************************************************************************************************************************************

//common commands
static void getFlashStatus(SPI_FlashHeader* flash);                                                                             
static FlashFunctionStatus setPageSize(SPI_FlashHeader* flash, FlashPageSize pageSize);    

static bool isLastCompareResultError(SPI_FlashHeader* flash);  
static bool isLastWriteResultError(SPI_FlashHeader* flash);    
static void waitOfFlashReady(SPI_FlashHeader* flash);


//write commands
static void bufferWrite(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data); 
static void bufferWriteByte(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data, uint32_t byteNumber); 
static FlashFunctionStatus bufferWriteReliable(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data);  

static void writePageWithErasing(SPI_FlashHeader* flash, BufferName buffer, uint32_t pageNumber);
static void writePageWithoutErasing(SPI_FlashHeader* flash, BufferName buffer, uint32_t pageNumber);

//read commands
static void bufferRead(SPI_FlashHeader* flash, BufferName buffer, uint8_t* data);                                          
static void pageToBufferTransfer(SPI_FlashHeader* flash, BufferName buffer, uint32_t pageNumber);                               
static void pageWithBufferCompare(SPI_FlashHeader* flash, BufferName buffer, uint32_t pageNumber);                             
                                                               
static bool compareBufferAndDataZeros(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data);                                                                 
static bool compareAndCorrectBufferAndData(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data);


//high-level commands
static FlashFunctionStatus writeTwoPagesWithoutUnnecessaryErasing(SPI_FlashHeader* flash, const uint8_t* data, uint32_t pairOfPagesNumber);   
static void readTwoPages(SPI_FlashHeader* flash, uint8_t* data, uint32_t pairOfPagesNumber);

//*************************************************************************************************************************************
//end of flash command realisation-----------------------------------------------------------------------------------------------------


//external functions
//*************************************************************************************************************************************

int SPI_FlashInit(SPI_FlashHeader* flash)
{
    getFlashStatus(flash);
    if(flash->status.density != FLASH_DENSITY_4MB)
        return -1;
    if(flash->status.pageSize != flash->prefferedPageSize)
    {
        FlashFunctionStatus result;
        for(int i = 0; i < 3; i++)
        {
            result = setPageSize(flash, flash->prefferedPageSize);
            if((result == FLASH_OK) || (result == FLASH_COMMAND_SUCCESS))
                break;
        }
        if(result == FLASH_COMMAND_ERROR)
            return -1;
    }       
    return 0;
}

int SPI_FlashBlockWrite(SPI_FlashHeader* flash, uint32_t blockNumber, const uint8_t* data)
{
    if(writeTwoPagesWithoutUnnecessaryErasing(flash, data, blockNumber) != FLASH_COMMAND_ERROR)
        return 0;
    else return -1;
}

void SPI_FlashBlockRead(SPI_FlashHeader* flash, uint32_t blockNumber, uint8_t* data)
{
    readTwoPages(flash, data, blockNumber);
}


static FlashFunctionStatus writeTwoPagesWithoutUnnecessaryErasing(SPI_FlashHeader* flash, const uint8_t* data, uint32_t pairOfPagesNumber)
{
    int firstPageNumber = pairOfPagesNumber * 2;
    int secondPageNumber = firstPageNumber + 1;
    
    pageToBufferTransfer(flash, BUFFER_1, firstPageNumber);
    pageToBufferTransfer(flash, BUFFER_2, secondPageNumber);
    
    bool canWriteFirstPageWithoutErasing = compareBufferAndDataZeros(flash, BUFFER_1, data);
    
    if(bufferWriteReliable(flash, BUFFER_1, data) != FLASH_COMMAND_SUCCESS)
        return FLASH_COMMAND_ERROR;
    
    if(canWriteFirstPageWithoutErasing)
        writePageWithoutErasing(flash, BUFFER_1, firstPageNumber);
    else
        writePageWithErasing(flash, BUFFER_1, firstPageNumber);
    
    int pageSize;
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
        pageSize = 256;
    else
        pageSize = 264;
    
    bool canWriteSecondPageWithoutErasing = compareBufferAndDataZeros(flash, BUFFER_2, &data[pageSize]);
    
    if( bufferWriteReliable(flash, BUFFER_2, &data[pageSize]) != FLASH_COMMAND_SUCCESS)
        return FLASH_COMMAND_ERROR;
    
    if(isLastWriteResultError(flash))
        return FLASH_COMMAND_ERROR;
    
    if(canWriteSecondPageWithoutErasing)
        writePageWithoutErasing(flash, BUFFER_2, secondPageNumber);
    else
        writePageWithErasing(flash, BUFFER_2, secondPageNumber);
    
    if(isLastWriteResultError(flash))
        return FLASH_COMMAND_ERROR;
    
    pageWithBufferCompare(flash, BUFFER_1, firstPageNumber);
    
    if(isLastCompareResultError(flash))
    {   
        //second and last attempt to write buffer 1 in memory
        if(canWriteFirstPageWithoutErasing)
            writePageWithoutErasing(flash, BUFFER_1, firstPageNumber);
        else
            writePageWithErasing(flash, BUFFER_1, firstPageNumber);
        
        if(isLastWriteResultError(flash))
            return FLASH_COMMAND_ERROR;
        
        if(isLastCompareResultError(flash))
            return FLASH_COMMAND_ERROR;
    }
    
    pageWithBufferCompare(flash, BUFFER_2, secondPageNumber);
    
    if(isLastCompareResultError(flash))
    {   
        //second and last attempt to write buffer 2 in memory
        if(canWriteFirstPageWithoutErasing)
            writePageWithoutErasing(flash, BUFFER_2, secondPageNumber);
        else
            writePageWithErasing(flash, BUFFER_2, secondPageNumber);
        
        if(isLastWriteResultError(flash))
            return FLASH_COMMAND_ERROR;
        
        if(isLastCompareResultError(flash))
            return FLASH_COMMAND_ERROR;
    }
    
    return FLASH_COMMAND_SUCCESS;
    
}

static void readTwoPages(SPI_FlashHeader* flash, uint8_t* data, uint32_t pairOfPagesNumber)
{
    int firstPageNumber = pairOfPagesNumber * 2;
    int secondPageNumber = firstPageNumber + 1;
    
    pageToBufferTransfer(flash, BUFFER_1, firstPageNumber);
    pageToBufferTransfer(flash, BUFFER_2, secondPageNumber);
    
    bufferRead(flash, BUFFER_1, data);
    
    int pageSize;
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
        pageSize = 256;
    else
        pageSize = 264;

    waitOfFlashReady(flash);
    bufferRead(flash, BUFFER_2, &data[pageSize]);
}

//*************************************************************************************************************************************
//end of external functions


//command realisation------------------------------------------------------------------------------------------------------------------
//*************************************************************************************************************************************

static void getFlashStatus(SPI_FlashHeader* flash)
{
    flashSelect(flash);
    SPI_Transmit(flash, STATUS_REGISTER_READ);
    uint8_t firstSR_Byte = SPI_Receive(flash);
    uint8_t secondSR_Byte = SPI_Receive(flash);
    flashDeselect(flash);

    
    if(firstSR_Byte & 0x40)
        flash->status.compareResultError = true;
    else
        flash->status.compareResultError = false;
    
    if(firstSR_Byte & 0x02)
        flash->status.protectionEnabled = true;
    else
        flash->status.protectionEnabled = false;
    
    if(firstSR_Byte & 0x01)
        flash->status.pageSize = FLASH_PAGE_SIZE_256;
    else
        flash->status.pageSize = FLASH_PAGE_SIZE_264;
    
    switch((firstSR_Byte & 0x3c) >> 2)
    {
        case 0x7:
            flash->status.density = FLASH_DENSITY_4MB;
            break;
        default:
            flash->status.density = FLASH_DENSITY_UNKNOWN;
    }
    
    if(secondSR_Byte & 0x80)
        flash->status.isReady = true;
    else
        flash->status.isReady = false;
    
    if(secondSR_Byte & 0x20)
        flash->status.eraseOrProgramError = true;
    else
        flash->status.eraseOrProgramError = false;
    
    if(secondSR_Byte & 0x08)
        flash->status.sectorLockdownEnabled = true;
    else
        flash->status.sectorLockdownEnabled = false;
    
    if(secondSR_Byte & 0x04)
        flash->status.bufferTwoIsSuspended = true;
    else
        flash->status.bufferTwoIsSuspended = false;
    
    if(secondSR_Byte & 0x02)
        flash->status.bufferOneIsSuspended = true;
    else
        flash->status.bufferOneIsSuspended = false;
    
    if(secondSR_Byte & 0x01)
        flash->status.eraseIsSuspended = true;
    else
        flash->status.eraseIsSuspended = false;
}

static void waitOfFlashReady(SPI_FlashHeader* flash)
{
    do
    {
        getFlashStatus(flash);
    } while(flash->status.isReady == false); 
}

static FlashFunctionStatus setPageSize(SPI_FlashHeader* flash, FlashPageSize pageSize)
{
    getFlashStatus(flash);
    if(flash->status.pageSize == pageSize)
        return FLASH_OK;
    flashSelect(flash);
    SPI_Transmit(flash, SET_PAGE_SIZE_FIRST_BYTE);
    SPI_Transmit(flash, SET_PAGE_SIZE_SECOND_BYTE);
    SPI_Transmit(flash, SET_PAGE_SIZE_THIRD_BYTE);
    if(pageSize == FLASH_PAGE_SIZE_256)
        SPI_Transmit(flash, SET_PAGE_SIZE_256_FOURTH_BYTE);
    else
        SPI_Transmit(flash, SET_PAGE_SIZE_264_FOURTH_BYTE);
    flashDeselect(flash);
    getFlashStatus(flash);
    if(flash->status.pageSize == pageSize)
        return FLASH_COMMAND_SUCCESS;
    else
        return FLASH_COMMAND_ERROR;
}

static void bufferWrite(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data)
{
    flashSelect(flash);
    if(buffer == BUFFER_1)
        SPI_Transmit(flash, BUFFER_1_WRITE);
    else
        SPI_Transmit(flash, BUFFER_2_WRITE);
    
    SPI_Transmit(flash, 0x00);
    SPI_Transmit(flash, 0x00);
    SPI_Transmit(flash, 0x00);
    
    int numberOfBytes;
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
        numberOfBytes = 256;
    else
        numberOfBytes = 264;
    
    for(int i = 0; i < numberOfBytes; i++)
        SPI_Transmit(flash, data[i]);
    flashDeselect(flash);
}

static void bufferWriteByte(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data, uint32_t byteNumber)
{
    flashSelect(flash);
    if(buffer == BUFFER_1)
        SPI_Transmit(flash, BUFFER_1_WRITE);
    else
        SPI_Transmit(flash, BUFFER_2_WRITE);
    
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
    {
        SPI_Transmit(flash, 0x00);
        SPI_Transmit(flash, 0x00);
        SPI_Transmit(flash, byteNumber & 0xff);
    
    }
    else
    {
        SPI_Transmit(flash, 0x00);
        SPI_Transmit(flash, (byteNumber >> 8) & 0x01);
        SPI_Transmit(flash, byteNumber & 0xff);
    }
    
    SPI_Transmit(flash, data[byteNumber]);
    flashDeselect(flash);
}

static FlashFunctionStatus bufferWriteReliable(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data)
{
    bufferWrite(flash, buffer, data);
    const int numberOfAttempts = 3;
    for(int i = 0; i < numberOfAttempts; i++)
    {
        if(compareAndCorrectBufferAndData(flash, buffer, data) == true)
        {
            return FLASH_COMMAND_SUCCESS;
        }
    }
    return FLASH_COMMAND_ERROR;
}

static void bufferRead(SPI_FlashHeader* flash, BufferName buffer, uint8_t* data)
{
    flashSelect(flash);
    if(flash->allowHighSpeed)
    {
        if(buffer == BUFFER_1)
            SPI_Transmit(flash, BUFFER_1_READ_HIGH_FREQUENCY);
        else
            SPI_Transmit(flash, BUFFER_2_READ_HIGH_FREQUENCY);
    }
    else
    {
        if(buffer == BUFFER_1)
            SPI_Transmit(flash, BUFFER_1_READ_LOW_FREQUENCY);
        else
            SPI_Transmit(flash, BUFFER_2_READ_LOW_FREQUENCY);
    }
    
    SPI_Transmit(flash, 0x00);
    SPI_Transmit(flash, 0x00);
    SPI_Transmit(flash, 0x00);
    
    int numberOfBytes;
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
        numberOfBytes = 256;
    else
        numberOfBytes = 264;
    
    for(int i = 0; i < numberOfBytes; i++)
        data[i] = SPI_Receive(flash);
    flashDeselect(flash);
}


//return 0 if buffer contains zero bits, which are 1 in data
static bool compareBufferAndDataZeros(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data)
{
    flashSelect(flash);
    if(flash->allowHighSpeed)
    {
        if(buffer == BUFFER_1)
            SPI_Transmit(flash, BUFFER_1_READ_HIGH_FREQUENCY);
        else
            SPI_Transmit(flash, BUFFER_2_READ_HIGH_FREQUENCY);
    }
    else
    {
        if(buffer == BUFFER_1)
            SPI_Transmit(flash, BUFFER_1_READ_LOW_FREQUENCY);
        else
            SPI_Transmit(flash, BUFFER_2_READ_LOW_FREQUENCY);
    }
    
    SPI_Transmit(flash, 0x00);
    SPI_Transmit(flash, 0x00);
    SPI_Transmit(flash, 0x00);
    
    int numberOfBytes;
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
        numberOfBytes = 256;
    else
        numberOfBytes = 264;
    
    
    bool isAbleToNotErase = true;
        
    for(int i = 0; i < numberOfBytes; i++)
    {
        uint8_t fromMemory = SPI_Receive(flash);
        uint8_t fromNewData = data[i];
        
        uint8_t commonZeros = (~fromMemory) & (~fromNewData);
        uint8_t uniqueMemoryZeros = (~fromMemory) ^ commonZeros;
        
        if(uniqueMemoryZeros != 0)
        {
            isAbleToNotErase = false;
            break;
        }

    }
   
    flashDeselect(flash);
    return isAbleToNotErase;
}

//return 0 if data and buffer are not similar
static bool compareAndCorrectBufferAndData(SPI_FlashHeader* flash, BufferName buffer, const uint8_t* data)
{
    flashSelect(flash);
    if(flash->allowHighSpeed)
    {
        if(buffer == BUFFER_1)
            SPI_Transmit(flash, BUFFER_1_READ_HIGH_FREQUENCY);
        else
            SPI_Transmit(flash, BUFFER_2_READ_HIGH_FREQUENCY);
    }
    else
    {
        if(buffer == BUFFER_1)
            SPI_Transmit(flash, BUFFER_1_READ_LOW_FREQUENCY);
        else
            SPI_Transmit(flash, BUFFER_2_READ_LOW_FREQUENCY);
    }
    
    SPI_Transmit(flash, 0x00);
    SPI_Transmit(flash, 0x00);
    SPI_Transmit(flash, 0x00);
    
    int numberOfBytes;
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
        numberOfBytes = 256;
    else
        numberOfBytes = 264;
    
    
    bool isSimilar = true;
        
    for(int i = 0; i < numberOfBytes; i++)
    {
        uint8_t fromMemory = SPI_Receive(flash);
        uint8_t fromData = data[i];
        
        if(fromMemory != fromData)
        {
            isSimilar = false;
            bufferWriteByte(flash, buffer, data, i);
        }
    }
   
    flashDeselect(flash);
    return isSimilar;
}


static void pageToBufferTransfer(SPI_FlashHeader* flash, BufferName buffer, uint32_t pageNumber)
{
    waitOfFlashReady(flash);
    
    flashSelect(flash);
    
    if(buffer == BUFFER_1)
        SPI_Transmit(flash, MEMORY_TO_BUFFER_1_TRANSFER);
    else
        SPI_Transmit(flash, MEMORY_TO_BUFFER_2_TRANSFER);
    
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
    {
        SPI_Transmit(flash, (pageNumber >> 8) & 0x07);
        SPI_Transmit(flash, pageNumber & 0xff);
        SPI_Transmit(flash, 0x00);
    }
    else
    {
        SPI_Transmit(flash, (pageNumber >> 7) & 0x0f);
        SPI_Transmit(flash, (pageNumber << 1) & 0xfe);
        SPI_Transmit(flash, 0x00);
    }
        
    flashDeselect(flash);
}

static void writePageWithErasing(SPI_FlashHeader* flash, BufferName buffer, uint32_t pageNumber)
{
    waitOfFlashReady(flash);
        
    flashSelect(flash);
    
    if(buffer == BUFFER_1)
        SPI_Transmit(flash, BUFFER_1_TO_MEMORY_WRITE_PAGE_WITH_ERASE);
    else
        SPI_Transmit(flash, BUFFER_2_TO_MEMORY_WRITE_PAGE_WITH_ERASE);
    
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
    {
        SPI_Transmit(flash, (pageNumber >> 8) & 0x07);
        SPI_Transmit(flash, pageNumber & 0xff);
        SPI_Transmit(flash, 0x00);
    }
    else
    {
        SPI_Transmit(flash, (pageNumber >> 7) & 0x0f);
        SPI_Transmit(flash, (pageNumber << 1) & 0xfe);
        SPI_Transmit(flash, 0x00);
    }
        
    flashDeselect(flash);
}


static void writePageWithoutErasing(SPI_FlashHeader* flash, BufferName buffer, uint32_t pageNumber)
{
    waitOfFlashReady(flash);
    
    flashSelect(flash);
    
    if(buffer == BUFFER_1)
        SPI_Transmit(flash, BUFFER_1_TO_MEMORY_WRITE_PAGE_WITHOUT_ERASE);
    else
        SPI_Transmit(flash, BUFFER_2_TO_MEMORY_WRITE_PAGE_WITHOUT_ERASE);
    
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
    {
        SPI_Transmit(flash, (pageNumber >> 8) & 0x07);
        SPI_Transmit(flash, pageNumber & 0xff);
        SPI_Transmit(flash, 0x00);
    }
    else
    {
        SPI_Transmit(flash, (pageNumber >> 7) & 0x0f);
        SPI_Transmit(flash, (pageNumber << 1) & 0xfe);
        SPI_Transmit(flash, 0x00);
    }
        
    flashDeselect(flash);
}


static void pageWithBufferCompare(SPI_FlashHeader* flash, BufferName buffer, uint32_t pageNumber)
{
    waitOfFlashReady(flash);
    
    flashSelect(flash);
    
    if(buffer == BUFFER_1)
        SPI_Transmit(flash, MEMORY_WITH_BUFFER_1_COMPARE);
    else
        SPI_Transmit(flash, MEMORY_WITH_BUFFER_2_COMPARE);
    
    if(flash->status.pageSize == FLASH_PAGE_SIZE_256)
    {
        SPI_Transmit(flash, (pageNumber >> 8) & 0x07);
        SPI_Transmit(flash, pageNumber & 0xff);
        SPI_Transmit(flash, 0x00);
    }
    else
    {
        SPI_Transmit(flash, (pageNumber >> 7) & 0x0f);
        SPI_Transmit(flash, (pageNumber << 1) & 0xfe);
        SPI_Transmit(flash, 0x00);
    }
        
    flashDeselect(flash);
}

static bool isLastCompareResultError(SPI_FlashHeader* flash)
{
    waitOfFlashReady(flash);
    return flash->status.compareResultError;
}

static bool isLastWriteResultError(SPI_FlashHeader* flash)
{
    waitOfFlashReady(flash);
    return flash->status.eraseOrProgramError;
}


//*************************************************************************************************************************************
//end of command realisation-----------------------------------------------------------------------------------------------------------




//SPI interface commands---------------------------------------------------------------------------------------------------------------
//*************************************************************************************************************************************

static uint8_t SPI_Transfer(SPI_FlashHeader* flash, uint8_t sendingByte)
{
    uint8_t result;
    HAL_SPI_TransmitReceive(flash->flashSpiPort, &sendingByte, &result, 1, 0x100);
    return result;
}

static void SPI_Transmit(SPI_FlashHeader* flash, uint8_t sendingByte)
{
    SPI_Transfer(flash, sendingByte);
}

static uint8_t SPI_Receive(SPI_FlashHeader* flash)
{
    return SPI_Transfer(flash, 0x00);
}


static void flashSelect(SPI_FlashHeader* flash)
{
    HAL_GPIO_WritePin(flash->flashSS_GPIO_Port, flash->flashSS_GPIO_Pin, GPIO_PIN_RESET);
}
static void flashDeselect(SPI_FlashHeader* flash)
{
    HAL_GPIO_WritePin(flash->flashSS_GPIO_Port, flash->flashSS_GPIO_Pin, GPIO_PIN_SET);
}
//*************************************************************************************************************************************
//end of SPI interface commands--------------------------------------------------------------------------------------------------------
