#ifndef EMULATED_FILE_H_
#define EMULATED_FILE_H_

#include "common_file_defines.h"

typedef enum {EMULATED_FILE_OK, EMULATED_FILE_ENDED, EMULATED_FILE_ERROR_NOT_FOUND} EmulatedFileDescriptorStatus;

typedef FileSize (*EmulatedFileGetDataFoo)(void* this, uint8_t* destination, FileSize size);
typedef void (*EmulatedFileInitFoo)(void* this, const char* fileName);

typedef struct
{
    EmulatedFileDescriptorStatus status;
    EmulatedFileGetDataFoo getData;
    EmulatedFileInitFoo init;
    
    // private members
    const uint8_t* data;
    FileSize size;
    uint32_t readedAlready;
} EmulatedFileDescriptor;


void initEmulatedFileDescriptor(EmulatedFileDescriptor* descriptor);

#endif // EMULATED_FILE_H_
