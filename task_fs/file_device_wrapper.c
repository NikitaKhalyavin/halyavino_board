#include "file_device_wrapper.h"
#include "filesystem_object_descriptor.h"
#include <string.h>

static FATFS fs;

static FileDeviceWrapperLinkingResult fileDeviceWrapperLinkFile(void* this, const char* fileName, 
                        FileStorageDevice source, FileTransferDirection dir);
 
static void fileDeviceWrapperSetReceivingBuffer(void* this, StreamFileBuffer* buffer);
     
static void fileDeviceWrapperRunTransferIfPossible(void* this);
static void fileDeviceWrapperReset(void* this);
static void fileDeviceWrapperCloseFile(void* this);

static size_t fileDeviceWrapperReadData(void* this, uint8_t* dest, size_t size);
static size_t fileDeviceWrapperWriteData(void* this, const uint8_t* source, size_t size);

// no thread protection, because these functions may be called from one thread only
#define TEMP_BUFFER_SIZE 512
static uint8_t tempBuffer[TEMP_BUFFER_SIZE];

void fileDeviceWrapperInit(FileDeviceWrapper* wrapper)
{
    wrapper->status = FILE_DEVICE_WRAPPER_STATUS_IDLE;
    wrapper->fileDevice = FILE_DEVICE_EMULATED;
    wrapper->transferDirection = FILE_TRANSFER_DIRECTION_GET;
    wrapper->receivingBuffer = NULL;
    wrapper->isFileOpened = false;
    
    wrapper->linkFile = fileDeviceWrapperLinkFile;
    wrapper->reset = fileDeviceWrapperReset;
    wrapper->setBuffer = fileDeviceWrapperSetReceivingBuffer;
    wrapper->runTransfer = fileDeviceWrapperRunTransferIfPossible;
    
    wrapper->read = fileDeviceWrapperReadData;
    wrapper->write = fileDeviceWrapperWriteData;
    wrapper->closeFile = fileDeviceWrapperCloseFile;
}


static FileDeviceWrapperLinkingResult fileDeviceWrapperLinkFile(void* this, const char* fileName, 
                        FileStorageDevice source, FileTransferDirection dir)
{
    FileDeviceWrapper* wrapper = (FileDeviceWrapper*)this;
    wrapper->transferDirection = dir;
    wrapper->fileDevice = source;
    
    wrapper->closeFile(wrapper);
    
    switch(source)
    {
        case FILE_DEVICE_EMULATED:
        {
            if(dir == FILE_TRANSFER_DIRECTION_GIVE)
            {
                return FILE_DEVICE_WRAPPER_LINKING_ERROR_NOT_ALLOWED;
            }
            EmulatedFileDescriptor file;
            initEmulatedFileDescriptor(&file);
            file.init(&file, fileName);
            if(file.status == EMULATED_FILE_ERROR_NOT_FOUND)
                return FILE_DEVICE_WRAPPER_LINKING_ERROR_NOFILE;
            
            wrapper->file.emulatedFile = file;
            break;
        }
        case FILE_DEVICE_STORAGE:
        {
            FRESULT res;
            if(dir == FILE_TRANSFER_DIRECTION_GIVE)
							// error: storage fs is readonly
								return FILE_DEVICE_WRAPPER_LINKING_ERROR_NOT_ALLOWED;
            else
                res = f_open(&(wrapper->file.fatFS_File), fileName, FA_READ);
            if(res != FR_OK)
            switch(res)
            {
                case FR_NO_FILE:
                    return FILE_DEVICE_WRAPPER_LINKING_ERROR_NOFILE;
                case FR_LOCKED:
                    return FILE_DEVICE_WRAPPER_LINKING_ERROR_FILE_BUSY;
                default:
                    return FILE_DEVICE_WRAPPER_LINKING_ERROR_NOT_ALLOWED;
            }
            break;
        }
    }
    wrapper->isFileOpened = true;
    return FILE_DEVICE_WRAPPER_LINKING_SUCCESS;
}
 
static void fileDeviceWrapperSetReceivingBuffer(void* this, StreamFileBuffer* buffer)
{
    FileDeviceWrapper* wrapper = (FileDeviceWrapper*)this;
    if(wrapper->status == FILE_DEVICE_WRAPPER_STATUS_IDLE)
    wrapper->receivingBuffer = buffer;
    buffer->setStatus(buffer, STREAM_BUFFER_STATUS_IN_PROCESS);
    if(wrapper->isFileOpened)
        wrapper->status = FILE_DEVICE_WRAPPER_STATUS_IN_PROCESS;
}
     
static void fileDeviceWrapperRunTransferIfPossible(void* this)
{
    FileDeviceWrapper* wrapper = (FileDeviceWrapper*)this;
    if(wrapper->status != FILE_DEVICE_WRAPPER_STATUS_IN_PROCESS)
        return;
    if(!wrapper->receivingBuffer)
        return;
    if(!wrapper->isFileOpened)
        return;
    
    if(wrapper->transferDirection == FILE_TRANSFER_DIRECTION_GET)
    {
        
        uint32_t sizeForReading = wrapper->receivingBuffer->dataBufferSize - wrapper->receivingBuffer->size;
        if(sizeForReading > TEMP_BUFFER_SIZE)
            sizeForReading = TEMP_BUFFER_SIZE;
        size_t readed = wrapper->read(wrapper, tempBuffer, sizeForReading);
        if(readed == sizeForReading)
            wrapper->receivingBuffer->write(wrapper->receivingBuffer, tempBuffer, readed);
        else
				{
            wrapper->receivingBuffer->writeLast(wrapper->receivingBuffer, tempBuffer, readed);
						wrapper->status = FILE_DEVICE_WRAPPER_STATUS_ENDED;
				}
    }
    else
    {
        if(wrapper->receivingBuffer->status == STREAM_BUFFER_STATUS_FILE_END)
        {
            uint32_t sizeForWriting = wrapper->receivingBuffer->size;
            if(sizeForWriting > TEMP_BUFFER_SIZE)
                sizeForWriting = TEMP_BUFFER_SIZE;
            
            wrapper->receivingBuffer->read(wrapper->receivingBuffer, tempBuffer, sizeForWriting);
            wrapper->write(wrapper, tempBuffer, sizeForWriting);
            wrapper->closeFile(wrapper);
            return;
        }
        
        // if too few data - wait for more
        if(wrapper->receivingBuffer->size < wrapper->receivingBuffer->dataBufferSize / 2)
            return; 
       
        uint32_t sizeForWriting = wrapper->receivingBuffer->size;
        if(sizeForWriting > TEMP_BUFFER_SIZE)
            sizeForWriting = TEMP_BUFFER_SIZE;
        wrapper->receivingBuffer->read(wrapper->receivingBuffer, tempBuffer, sizeForWriting);
        wrapper->write(wrapper, tempBuffer, sizeForWriting);
    }
}

static void fileDeviceWrapperReset(void* this)
{
    FileDeviceWrapper* wrapper = (FileDeviceWrapper*)this;
    wrapper->runTransfer(wrapper);
    if(wrapper->receivingBuffer)
        wrapper->receivingBuffer->setStatus(wrapper->receivingBuffer, STREAM_BUFFER_STATUS_IDLE);
    wrapper->closeFile(wrapper);
    wrapper->status = FILE_DEVICE_WRAPPER_STATUS_IDLE;
    
}

static size_t fileDeviceWrapperReadData(void* this, uint8_t* dest, size_t size)
{
    FileDeviceWrapper* wrapper = (FileDeviceWrapper*)this;
    if(!wrapper->isFileOpened)
        return 0;
    if(wrapper->transferDirection != FILE_TRANSFER_DIRECTION_GET)
        return 0;
    switch(wrapper->fileDevice)
    {
        case FILE_DEVICE_EMULATED:
        {
            return wrapper->file.emulatedFile.getData(&(wrapper->file.emulatedFile), dest, size);
        }
        case FILE_DEVICE_STORAGE:
        {
            FRESULT res;
            size_t readed = 0;
            res = f_read(&(wrapper->file.fatFS_File), dest, size, &readed);
            if(res != FR_OK)
            {
                wrapper->closeFile(wrapper);
                wrapper->status = FILE_DEVICE_WRAPPER_STATUS_IDLE;
                wrapper->receivingBuffer->setStatus(wrapper->receivingBuffer, STREAM_BUFFER_STATUS_ERROR);
            }
            return readed;
        }
        default:
            return 0;
    }
}

static size_t fileDeviceWrapperWriteData(void* this, const uint8_t* source, size_t size)
{
    FileDeviceWrapper* wrapper = (FileDeviceWrapper*)this;
    if(!wrapper->isFileOpened)
        return 0;
    if(wrapper->transferDirection != FILE_TRANSFER_DIRECTION_GIVE)
        return 0;
    switch(wrapper->fileDevice)
    {
        case FILE_DEVICE_EMULATED:
        {
            // error: can't write emulated files
            return 0;
        }
        case FILE_DEVICE_STORAGE:
        {
					// error: fatfs is readonly
					return 0;
        }
        default:
            return 0;
    }
}


static void fileDeviceWrapperCloseFile(void* this)
{
    FileDeviceWrapper* wrapper = (FileDeviceWrapper*)this;
    if(wrapper->isFileOpened)
    {
        if(wrapper->fileDevice == FILE_DEVICE_STORAGE)
        {
            FRESULT res;
            res = f_close(&(wrapper->file.fatFS_File));
            return;
        }
        wrapper->isFileOpened = false;
    }
}

static void blockingWriteInBuffer(StreamFileBuffer* buffer, uint8_t* src, uint32_t size)
{
    uint32_t writed = buffer->write(buffer, src, size);
    while(writed < size)
        writed += buffer->write(buffer, src + writed, size - writed);
}

bool writeInBufferWithCheck(StreamFileBuffer* buffer, uint8_t* src, uint32_t size)
{
    uint32_t writed = buffer->write(buffer, src, size);
    return (writed != size);
}





void initFatFS()
{
    FRESULT res;
    // mount the default drive
    res = f_mount(&fs, "", 0);
    if(res != FR_OK) 
    {
       return;
    }
}

