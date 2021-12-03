#include "emulated_file.h"

#include "emulated_file_hard_code.h"
#include <string.h>


void emulatedFileInitFoo(void* this, const char* fileName)
{
    if(!isEmulatedFilesListInitialized)
        initializeList();
    
    EmulatedFileStorageDescriptor* fileStorage = NULL;
    for(int i = 0; i < emulatedFilesNumber; i++)
    {
        if(strcmp(fileName, emulatedFiles[i].name) == 0)
        {
            fileStorage = &emulatedFiles[i];
            break;
        }
    }
    
    EmulatedFileDescriptor* descriptor = (EmulatedFileDescriptor*)this;
    if(fileStorage == NULL)
    {
        descriptor->status = EMULATED_FILE_ERROR_NOT_FOUND;
        descriptor->data = NULL;
        descriptor->size = 0;
    }
    else
    {
        descriptor->status = EMULATED_FILE_OK;
        descriptor->data = fileStorage->data;
        descriptor->size = fileStorage->size;
        if(descriptor->size == 0)
            descriptor->status = EMULATED_FILE_ENDED;
    }
    
    descriptor->readedAlready = 0;
}

FileSize emulatedFileGetDataFoo(void* this, uint8_t* destination, FileSize size)
{
    EmulatedFileDescriptor* descriptor = (EmulatedFileDescriptor*)this;
    
    // if can't return all requested data - return as many as possible
    FileSize sizeRemains = descriptor->size - descriptor->readedAlready;
    if(sizeRemains < size)
        size = sizeRemains;
    
    memcpy(destination, descriptor->data + descriptor->readedAlready, size);
    descriptor->readedAlready += size;
    return size;
}

void initEmulatedFileDescriptor(EmulatedFileDescriptor* descriptor)
{
    descriptor->init = emulatedFileInitFoo;
    descriptor->getData = emulatedFileGetDataFoo;
    
    descriptor->readedAlready = 0;
    descriptor->status = EMULATED_FILE_OK;
    descriptor->data = NULL;
    descriptor->size = 0;
}


