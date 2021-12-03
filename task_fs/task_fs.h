#ifndef TASK_FS_H_
#define TASK_FS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "common_file_defines.h"
#include "stream_file_buffer.h"
#include "fs_api.h"

typedef struct 
{
    bool locked;
    FS_ApiResult result;
} SimpleCriticalSemaphoreFS;

void fsSemLock(SimpleCriticalSemaphoreFS * sem);
void fsSemWait(SimpleCriticalSemaphoreFS * sem);
void fsSemUnlock(SimpleCriticalSemaphoreFS * sem, FS_ApiResult);

typedef enum {FILE_MESSAGE_LINK, FILE_MESSAGE_UNLINK} FileMessageType;

typedef struct
{
    FileStorageDevice fileSource;
    FileTransferDirection direction;
    const char* fileName;
    StreamFileBuffer* receivingBuffer;
} FileLinkMessage;

typedef struct
{
    StreamFileBuffer* receivingBuffer;
} FileUnlinkMessage;

typedef struct
{
    FileMessageType type;
    union
    {
        FileLinkMessage link;
        FileUnlinkMessage unlink;
    }params;
    SimpleCriticalSemaphoreFS* executionSem;
    
} TaskFS_Msg;

#endif // TASK_FS_H_
