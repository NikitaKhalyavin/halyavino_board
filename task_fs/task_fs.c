#include "task_fs.h"
#include "task_usb.h"
#include "stream_file_buffer.h"
#include "file_device_wrapper.h"
#include "critical.h"
#include "fs_api.h"


#define QUEUE_LENGTH 10

static uint8_t messageBufferFS[sizeof(TaskFS_Msg) * QUEUE_LENGTH];
static StaticQueue_t fsStaticQueue;

#define STACK_SIZE 300
StackType_t fsTaskStack[ STACK_SIZE ];
StaticTask_t fsTaskBuffer;

#define TASK_PERIOD_MS 10

#define MAX_FILES_COUNT 15
static FileDeviceWrapper fileWrappers[MAX_FILES_COUNT];
QueueHandle_t fsTaskMessageQueue;

static void fsTaskInitFoo(void* args);
static void sendMsgFS(TaskFS_Msg msg);
static void handleMsg(TaskFS_Msg msg);


static void sendMsgFS(TaskFS_Msg msg)
{
    xQueueSend(fsTaskMessageQueue, &msg, 0);
}

void initTaskFS(void)
{
    fsTaskMessageQueue = xQueueCreateStatic(QUEUE_LENGTH, sizeof(TaskFS_Msg), messageBufferFS, &fsStaticQueue);
    configASSERT(fsTaskMessageQueue);
    
    TaskHandle_t fsTaskHandle = xTaskCreateStatic(
                  fsTaskInitFoo,
                  "FS_TASK",
                  STACK_SIZE,
                  ( void * ) 1,
                  1,
                  fsTaskStack,
                  &fsTaskBuffer );
                  
    for(int i = 0; i < MAX_FILES_COUNT; i++)
    {
        fileDeviceWrapperInit(&(fileWrappers[i]));
    }
    
}


static void fsTaskInitFoo(void* args)
{
		initFatFS();
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while(1) 
    {
        for(int i = 0; i < MAX_FILES_COUNT; i++)
        {
            fileWrappers[i].runTransfer(&fileWrappers[i]);
        }
        TaskFS_Msg msg;
        
        // 0 means do not wait for messages
        BaseType_t res = xQueueReceive(fsTaskMessageQueue, &msg, 0);
        if(res == pdTRUE)
            handleMsg(msg);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TASK_PERIOD_MS));
    }
}



FS_ApiResult linkFileToBuffer(StreamFileBuffer* buffer, const char* fileName, FileTransferDirection dir, FileStorageDevice device)
{
    TaskFS_Msg msg;
    msg.type = FILE_MESSAGE_LINK;
    SimpleCriticalSemaphoreFS lock;
    msg.executionSem = &lock;
    FileLinkMessage linkMsg;
    linkMsg.direction = dir;
    linkMsg.fileName = fileName;
    linkMsg.fileSource = device;
    linkMsg.receivingBuffer = buffer;
    msg.params.link = linkMsg;

    fsSemLock(&lock);
    sendMsgFS(msg);
    fsSemWait(&lock);
    return lock.result;
}

FS_ApiResult unlinkFileFromBuffer(StreamFileBuffer* buffer)
{
    TaskFS_Msg msg;
    msg.type = FILE_MESSAGE_UNLINK;
    SimpleCriticalSemaphoreFS lock;
    msg.executionSem = &lock;
    FileUnlinkMessage unlinkMsg;
    unlinkMsg.receivingBuffer = buffer;
    msg.params.unlink = unlinkMsg;

    fsSemLock(&lock);
    sendMsgFS(msg);
    fsSemWait(&lock);
    return lock.result;
}


static void handleMsg(TaskFS_Msg msg)
{
    switch(msg.type)
    {
        case FILE_MESSAGE_LINK:
        {
            FileLinkMessage params = msg.params.link;
            for(int index = 0; index < MAX_FILES_COUNT; index++)
            {
                // find any free wrapper
                if(fileWrappers[index].status == FILE_DEVICE_WRAPPER_STATUS_IDLE)
                {
                    FileDeviceWrapperLinkingResult res =
                    fileWrappers[index].linkFile(&fileWrappers[index], params.fileName, params.fileSource, params.direction);
                    if(res == FILE_DEVICE_WRAPPER_LINKING_ERROR_NOFILE)
                    {
                        fsSemUnlock(msg.executionSem, FS_API_RESULT_NOT_FOUND);
                        return;
                    }
                    if(res == FILE_DEVICE_WRAPPER_LINKING_ERROR_FILE_BUSY)
                    {
                        fsSemUnlock(msg.executionSem, FS_API_RESULT_INT_ERR);
                        return;
                    }
                    if(res == FILE_DEVICE_WRAPPER_LINKING_ERROR_NOT_ALLOWED)
                    {
                        fsSemUnlock(msg.executionSem, FS_API_RESULT_REQUEST_ERROR);
                        return;
                    }
                    fileWrappers[index].setBuffer(&fileWrappers[index], params.receivingBuffer);
                    fsSemUnlock(msg.executionSem, FS_API_RESULT_SUCCESS);
                    return;
                }
            }
            // error: no free buffers
            fsSemUnlock(msg.executionSem, FS_API_RESULT_INT_ERR);
            return;
        }
        case FILE_MESSAGE_UNLINK:
        {
            FileUnlinkMessage params = msg.params.unlink;
            StreamFileBuffer* bufferPtr = params.receivingBuffer;
            for(int index = 0; index < MAX_FILES_COUNT; index++)
            {
                // find a wrapper, which handles specified buffer
                if(fileWrappers[index].receivingBuffer == bufferPtr)
                {
                    fileWrappers[index].reset(&fileWrappers[index]);
                    fsSemUnlock(msg.executionSem, FS_API_RESULT_SUCCESS);
                    return;
                }
            }
            fsSemUnlock(msg.executionSem, FS_API_RESULT_NOT_FOUND);
            return;
        }
    }
}



void fsSemLock(SimpleCriticalSemaphoreFS * sem)
{
    ENTER_CRITICAL()
    sem->locked = true;
    EXIT_CRITICAL()
}

void fsSemUnlock(SimpleCriticalSemaphoreFS * sem, FS_ApiResult res)
{
    
    ENTER_CRITICAL()
    sem->locked = false;
    sem->result = res;
    EXIT_CRITICAL()
}

void fsSemWait(SimpleCriticalSemaphoreFS * sem)
{
    while(sem->locked)
        vTaskDelay(100);
}

