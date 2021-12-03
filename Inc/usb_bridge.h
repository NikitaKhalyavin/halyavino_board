#ifndef USB_BRIDGE_H
#define USB_BRIDGE_H

#include "multy_thread_buffer.h"
#include <stdbool.h>

typedef uint8_t (*USB_ReportHandler)(void* this, uint8_t* reportData);
typedef bool (*GetDataFromMainThread)(void* this, uint8_t* dst);
typedef void (*SendUsbPacket)(uint8_t* data);
    
typedef struct
{
    DataBuffer bufferUSB_RTX;
    DataBuffer bufferUSB_RTX_Secondary;
    
    USB_ReportHandler interruptSideHandler;
    GetDataFromMainThread mainThreadSideReader;
    SendUsbPacket sendFromMainThread;
    
}UsbBridge;

void usbBridgeInit(UsbBridge* bridge);

#endif
