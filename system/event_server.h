#ifndef EVENT_SERVER_H
#define EVENT_SERVER_H

#include "event.h"
#include "multy_thread_buffer.h"

typedef enum {SERVER_EVENT_SEND_SUCCESSFULL, SERVER_EVENT_SEND_FULL} ServerSendResult;

typedef void (*InvokeEvents)(void* this);   
typedef ServerSendResult (*SendEvent)(void* this, EventMessage event);   

typedef struct
{
    InvokeEvents invokeEvents;
    SendEvent sendEvent;
    //private data
    DataBuffer eventQueue;
    DataBuffer secondaryEventQueue;
}EventServer;

void eventServerInit(EventServer* server);

#endif
