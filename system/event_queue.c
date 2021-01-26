
#include "event_queue.h"
#include "critical.h"
#include <stdbool.h>

EnqueueResult enqueue(void* this, EventMessage message)
{
    EventQueue * queue = (EventQueue*)this;
    bool isOverflowed = false;
    
    ENTER_CRITICAL()
    
    if (queue->numberOfFullFields >= queue->maxLength)
        isOverflowed = true;
    else
    {        
        uint32_t lastIndex = queue->firstObjectIndex + queue->numberOfFullFields;
        if(lastIndex >= queue->maxLength)
            lastIndex -= queue->maxLength;
        queue->dataArray[lastIndex] = message;
        
        queue->numberOfFullFields++;
    }
    EXIT_CRITICAL()
    if(isOverflowed)
        return ENQUEUE_RESULT_FULL;    
    
    return ENQUEUE_RESULT_SUCCESS;
}


DequeueResult dequeue(void* this, EventMessage* message)
{
    EventQueue * queue = (EventQueue*)this;
    bool isEmpty = false;
    
    ENTER_CRITICAL()
    
    if (queue->numberOfFullFields == 0)
        isEmpty = true;
    else
    {
        *message = queue->dataArray[queue->firstObjectIndex];
    
        queue->numberOfFullFields--;
        queue->firstObjectIndex++;
        if(queue->firstObjectIndex == queue->maxLength)
            queue->firstObjectIndex = 0;
    }
    EXIT_CRITICAL()
    
    if(isEmpty)
        return DEQUEUE_RESULT_EMPTY;
    
    return DEQUEUE_RESULT_SUCCESS;
}


void eventQueueInit( EventQueue* queue, uint32_t length, EventMessage* array)
{
    queue->dataArray = array;
    queue->firstObjectIndex = 0;
    queue->numberOfFullFields = 0;
    queue->maxLength = length;
    queue->tryToDecue = dequeue;
    queue->tryToEnque = enqueue;
}
