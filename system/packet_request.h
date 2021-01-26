#ifndef PACKET_REQUEST_H
#define PACKET_REQUEST_H
#include <stdint.h>

typedef enum {PACKET_REQUEST_SUCCESS, PACKET_REQUEST_ERROR} PacketRequestResult;

typedef void (*PacketReceivedCallBack)(void* owner, PacketRequestResult result, uint8_t* receivedData);


typedef struct
{
    PacketReceivedCallBack callback;
    void* owner;
    uint32_t packetNumber;
    const char* fileName;
} RequestMessage;


#endif
