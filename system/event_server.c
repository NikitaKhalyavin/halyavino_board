#include "event_server.h"
#include "event_queue.h"

#define QUEUE_LENGTH 20

EventQueue queue;

EventMessage buffer[QUEUE_LENGTH];

void invokeEvents(void* this)
{
    EventMessage event;
    uint8_t eventBytes[sizeof(EventMessage)];
    BufferReadingResult res;
    
    EventServer* server = (EventServer*) this;
    while(1)
    {
        res = server->eventQueue.tryToRead((void*)(&server->eventQueue), eventBytes, sizeof(EventMessage));
        if(res == BUFFER_READING_BUSY)
            // buffer is busy by other thread. We'll do it next time
            break;
        // invoke event handler
        event = *((EventMessage*)eventBytes);
        event.handlerReference(event.eventParams);
    }
    
    //secondary queue processing
    res = server->secondaryEventQueue.tryToRead((void*)(&server->secondaryEventQueue), eventBytes, sizeof(EventMessage));
    if(res == BUFFER_READING_BUSY)
        // buffer is busy by other thread. We'll do it next time
        return;
    // invoke event handler
    event = *((EventMessage*)eventBytes);
    event.handlerReference(event.eventParams);
}    


ServerSendResult sendEvent(void* this, EventMessage event)
{
    EventServer* server = (EventServer*) this;
    void* eventBytes = (void*)(&event);
    BufferWritingResult res;
    res = server->eventQueue.tryToWrite((void*)(&server->eventQueue),eventBytes, sizeof(EventHandler));
    if(res == BUFFER_WRITING_BUSY)
    {
        res = server->secondaryEventQueue.tryToWrite((void*)(&server->secondaryEventQueue),eventBytes, sizeof(EventHandler));
    }
    if(res != BUFFER_WRITING_SUCCESFULL)
    {
        //TO-DO: error handling
        return SERVER_EVENT_SEND_FULL;
    }
    return SERVER_EVENT_SEND_SUCCESSFULL;
}    

void eventServerInit(EventServer* server)
{
    bufferInit(&(server->eventQueue), eventBuffer, QUEUE_LENGTH * sizeof(EventHandler));
    bufferInit(&(server->secondaryEventQueue), secondaryEventBuffer, sizeof(EventHandler));
    server->invokeEvents = invokeEvents;
    server->sendEvent = sendEvent;
}
