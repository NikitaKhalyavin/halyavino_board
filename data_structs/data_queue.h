#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include <stdint.h>

typedef enum {DATA_QUEUE_ENQUEUE_RESULT_SUCCESS, DATA_QUEUE_ENQUEUE_RESULT_FULL} DataQueueEnqueueResult;
typedef enum {DATA_QUEUE_DEQUEUE_RESULT_SUCCESS, DATA_QUEUE_DEQUEUE_RESULT_EMPTY} DataQueueDequeueResult;

typedef DataQueueEnqueueResult (*DataQueueEnqueue)(void* this, const void* message);
typedef DataQueueDequeueResult (*DataQueueDequeue)(void* this, void* message);

typedef struct
{
    uint32_t maxLength;
    DataQueueEnqueue tryToEnqueue;
    DataQueueDequeue tryToDequeue;
    
    //private data
    uint32_t dataSize; 
    volatile uint8_t* dataArray;
    volatile uint32_t firstObjectIndex;
    volatile uint32_t numberOfFullFields;
    
} DataQueue;

void dataQueueInit( DataQueue* queue, uint32_t length, uint8_t* array, uint32_t dataTypeSize);

#endif // DATA_QUEUE_H
