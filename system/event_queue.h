#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stdint.h>
#include "event.h"

typedef enum {ENQUEUE_RESULT_SUCCESS, ENQUEUE_RESULT_FULL} EnqueueResult;
typedef enum {DEQUEUE_RESULT_SUCCESS, DEQUEUE_RESULT_EMPTY} DequeueResult;

typedef EnqueueResult (*Enqueue)(void* this, EventMessage message);
typedef DequeueResult (*Dequeue)(void* this, EventMessage* message);

typedef struct
{
    uint32_t maxLength;
    Enqueue tryToEnque;
    Dequeue tryToDecue;
    
    //private data
    volatile EventMessage* dataArray;
    volatile uint32_t firstObjectIndex;
    volatile uint32_t numberOfFullFields;
    
} EventQueue;

void eventQueueInit( EventQueue* queue, uint32_t length, EventMessage* array);

#endif
