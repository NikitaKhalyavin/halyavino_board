#include "task_device.h"
#include "device_api.h"
#include "executing_manager.h"


#define QUEUE_LENGTH 10

static uint8_t messageBufferDevice[sizeof(TaskDeviceMsg) * QUEUE_LENGTH];
static StaticQueue_t deviceStaticQueue;

#define STACK_SIZE 200
StackType_t deviceTaskStack[ STACK_SIZE ];
StaticTask_t deviceTaskBuffer;
QueueHandle_t deviceTaskMessageQueue;

#define TASK_PERIOD_MS 5
static void deviceTaskInitFoo(void* args);


void initTaskDevice()
{
		deviceTaskMessageQueue = xQueueCreateStatic(QUEUE_LENGTH, sizeof(TaskDeviceMsg), 
							messageBufferDevice, &deviceStaticQueue);
    configASSERT(fsTaskMessageQueue);
    
    TaskHandle_t deviceTaskHandle = xTaskCreateStatic(
                  deviceTaskInitFoo,
                  "DEVICE_TASK",
                  STACK_SIZE,
                  ( void * ) 1,
                  2,
                  deviceTaskStack,
                  &deviceTaskBuffer );
}


static void deviceTaskInitFoo(void* args)
{
		executingManagerInit();
	
		TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
		uint32_t timeMS = xLastWakeTime * 1000 / configTICK_RATE_HZ;
			
		executingManagerReconfigBoard(timeMS);
	
    while(1) 
    {
				uint32_t timeMS = xLastWakeTime * 1000 / configTICK_RATE_HZ;
				executingManagerHandle(timeMS);
				/*if(current->status == DEVICE_STATUS_WAIT)
				{
						current->start(current);
						lastTime = timeSeconds;
				}
				if(current->status != DEVICE_STATUS_RECORDING)
				{
						current->link(current, "wav.wav");
						continue;
				}
				current->handle(current, timeSeconds - lastTime);*/
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TASK_PERIOD_MS));
    }
}




void executingManagerThreadDelay(void)
{
		vTaskDelay(100);
}


void servoChannelIntHandler(void)
{
		//servoChannelDescriptorProcess(&list);
}

void servoChannelFinishIntHandler(void)
{
    //servoChannelDescriptorFinishProcess(&list);
}
