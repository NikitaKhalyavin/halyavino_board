#include "system_status_supervisor.h"
#include <status_led.h>

static void errorHandler (void* this)
{
    setStatusLedOn();
    while(1);
}


static void setDeviceStatus (void* this, DeviceStatus status)
{
    
}


static void setFileSystemStatus (void* this, FileSystemResultMessage status)
{
    
}

void systemStatusSupervisorInit(StatusOfModules* status)
{
    status->setDeviceStatus = setDeviceStatus;
    status->setFileSystemStatus = setFileSystemStatus;
    status->errorHandler = errorHandler;
    
    status->filesystemStatus = FILE_SUCCESS;
    status->deviceStatus = DEVICE_STATUS_FREE;
}
