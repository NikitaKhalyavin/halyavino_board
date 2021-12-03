
#include "data_queue.h"
#include "critical.h"
#include <stdbool.h>
#include <string.h>

static DataQueueEnqueueResult enqueue(void* this, const void* message)
{
    DataQueue * queue = (DataQueue*)this;
    bool isOverflowed = false;
    
    ENTER_CRITICAL()
    
    if (queue->numberOfFullFields >= queue->maxLength)
        isOverflowed = true;
    else
    {        
        uint32_t lastIndex = queue->firstObjectIndex + queue->numberOfFullFields;
        if(lastIndex >= queue->maxLength)
            lastIndex -= queue->maxLength;
        uint8_t* arrayCellStart = queue->dataArray + (lastIndex * queue->dataSize);
        memcpy(arrayCellStart, message, queue->dataSize);
        
        queue->numberOfFullFields++;
    }
    EXIT_CRITICAL()
    if(isOverflowed)
        return DATA_QUEUE_ENQUEUE_RESULT_FULL;    
    
    return DATA_QUEUE_ENQUEUE_RESULT_SUCCESS;
}


static DataQueueDequeueResult dequeue(void* this, void* message)
{
    DataQueue * queue = (DataQueue*)this;
    bool isEmpty = false;
    
    ENTER_CRITICAL()
    
    if (queue->numberOfFullFields == 0)
        isEmpty = true;
    else
    {
        uint8_t* arrayCellStart = queue->dataArray + (queue->firstObjectIndex * queue->dataSize);
        memcpy(message, arrayCellStart, queue->dataSize);
    
        queue->numberOfFullFields--;
        queue->firstObjectIndex++;
        
        // set start to zero if possible
        if(queue->firstObjectIndex == queue->maxLength)
            queue->firstObjectIndex = 0;
    }
    EXIT_CRITICAL()
    
    if(isEmpty)
        return DATA_QUEUE_DEQUEUE_RESULT_EMPTY;
    
    return DATA_QUEUE_DEQUEUE_RESULT_SUCCESS;
}


void dataQueueInit( DataQueue* queue, uint32_t length, uint8_t* array, uint32_t dataTypeSize)
{
    queue->dataArray = array;
    queue->dataSize = dataTypeSize;
    queue->firstObjectIndex = 0;
    queue->numberOfFullFields = 0;
    queue->maxLength = length;
    queue->tryToDequeue = dequeue;
    queue->tryToEnqueue = enqueue;
}
