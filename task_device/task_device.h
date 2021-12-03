#ifndef TASK_DEVICE_H_
#define TASK_DEVICE_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"


typedef struct
{
    uint32_t dummy;
    
} TaskDeviceMsg;

#endif // TASK_DEVICE_H_
