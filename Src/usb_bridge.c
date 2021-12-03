#include "usb_bridge.h"
#include "usbd_custom_hid_if.h"
#include "proto_defines.h"
#include "critical.h"

#include <stdbool.h>

extern USBD_HandleTypeDef hUsbDeviceFS;

static DataBuffer firstBuffer;
uint8_t firstBufferData[USB_PACKET_SIZE];
static DataBuffer secondBuffer;
uint8_t secondBufferData[USB_PACKET_SIZE];


static bool getDataFromMainThread(void* this, uint8_t* dst);

//always returns USBD_OK, logical parsing is doing on the higher layers
uint8_t USB_ReportHandlerFunction(void* this, uint8_t* reportData)
{
   
    UsbBridge* bridge = (UsbBridge*)this;
    //for sending in the event of main thread
    
    BufferWritingResult res = bridge->bufferUSB_RTX.tryToWrite((void*)(&(bridge->bufferUSB_RTX)), reportData, USB_PACKET_SIZE);
    if(res == BUFFER_WRITING_BUSY)
    {
        // cannot just wait while buffer will be free, because interrupt has the highest priority.
        // need to save data anywhere and return control
        // second buffer using is allowed ONLY while first is busy. This guarantees that at least one buffer is always free
        res = bridge->bufferUSB_RTX_Secondary.tryToWrite((void*)(&(bridge->bufferUSB_RTX_Secondary)), reportData, USB_PACKET_SIZE);
    }
    if(res != BUFFER_WRITING_SUCCESFULL)
    {
        return USBD_BUSY;
    }
    
    return USBD_OK;
        
}

static void sendUsbPacket(uint8_t* data)
{
    ENTER_CRITICAL()
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, data, USB_PACKET_SIZE);
    EXIT_CRITICAL()
}

static bool getDataFromMainThread(void* this, uint8_t* dst)
{
    UsbBridge* bridge = (UsbBridge*)this;
    BufferReadingResult res;
    
    if(bridge->bufferUSB_RTX.dataStatus == BUFFER_DATA_FULL)
    {
        res = bridge->bufferUSB_RTX.tryToRead(&(bridge->bufferUSB_RTX), dst, USB_PACKET_SIZE);
        if(res == BUFFER_READING_SUCCESFULL)
            return true;    
    }
    if(bridge->bufferUSB_RTX_Secondary.dataStatus == BUFFER_DATA_FULL)
    {
        res = bridge->bufferUSB_RTX_Secondary.tryToRead(&(bridge->bufferUSB_RTX), dst, USB_PACKET_SIZE);
        if(res == BUFFER_READING_SUCCESFULL)
            return true;    
    }
    return false;
}

void usbBridgeInit(UsbBridge* bridge)
{
    bridge->interruptSideHandler = USB_ReportHandlerFunction;
    bridge->mainThreadSideReader = getDataFromMainThread;
    bridge->sendFromMainThread = sendUsbPacket;
    bufferInit(&(bridge->bufferUSB_RTX), firstBufferData, USB_PACKET_SIZE);
    bufferInit(&(bridge->bufferUSB_RTX_Secondary), secondBufferData, USB_PACKET_SIZE);
}

