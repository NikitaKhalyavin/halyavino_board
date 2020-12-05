#ifndef SYSTEM_STATUS_SUPERVISOR_H
#define SYSTEM_STATUS_SUPERVISOR_H

#include "filesystem_status.h"
#include "device_manager_status.h"

typedef enum {NO_GLOBAL_RECORDING_ERROR, GLOBAL_RECORDING_ERROR_OCCURED} GlobalRecordingErrorStatus;

typedef void(*ErrorHandler)(void* this);
typedef void(*SetDeviceStatus)(void* this, DeviceStatus status);
typedef void(*SetFileSystemStatus)(void* this, FileSystemResultMessage status);

typedef struct 
{
    GlobalRecordingErrorStatus globalStatus;
    ErrorHandler errorHandler;
    
    FileSystemResultMessage filesystemStatus;
    void* lastFile;
    SetFileSystemStatus setFileSystemStatus;
    
    DeviceStatus deviceStatus;
    void* lastDevice;
    SetDeviceStatus setDeviceStatus;
    
    
}StatusOfModules;

void systemStatusSupervisorInit(StatusOfModules* status);

#endif