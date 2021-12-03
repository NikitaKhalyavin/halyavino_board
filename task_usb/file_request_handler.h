#ifndef FILE_REQUEST_HANDLER_H
#define FILE_REQUEST_HANDLER_H

#include <stdint.h>
#include "packet_request.h"

typedef enum {FILE_REQUEST_HANDLER_FREE, FILE_REQUEST_HANDLER_BUSY} FileRequestHandlerStatus;


void sendFileRequest(RequestMessage request);

void repeatLastSended();
void terminateTransmission();

//for calling after message receiving
void handleNewResponse(uint8_t* packet);
void handleNewRequest(uint8_t* packet);


#endif
