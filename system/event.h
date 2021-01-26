// this header describes an interface to send requests for actions
// in other threads or just parts of system

#ifndef EVENT_H
#define EVENT_H


typedef void (*EventHandler)(void* params);

typedef struct
{
    EventHandler handlerReference;
    void* eventParams;
}EventMessage;


#endif
