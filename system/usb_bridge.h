#ifndef USB_BRIDGE_H
#define USB_BRIDGE_H

#include "multy_thread_buffer.h"

typedef uint8_t (*USB_ReportHandler)(void* this, uint8_t* reportData);
    
typedef struct
{
    DataBuffer bufferUSB_RTX;
    DataBuffer bufferUSB_RTX_Secondary;
    
    USB_ReportHandler interruptSideHandler;
    
}UsbBridge;

void usbBridgeInit(UsbBridge* bridge);

#endif
