#include "usb_bridge.h"
#include "event.h"
#include "usbd_custom_hid_if.h"
#include "event_queue.h"
#include "file_request_handler.h"
#include "device_request_handler.h"

#include "usb_proto_defines.h"

#include <global_device_manager.h>

extern USBD_HandleTypeDef hUsbDeviceFS;
extern EventQueue eventQueue;

static uint8_t sendBuffer[1 + MAX_PACKET_LENGTH];
static uint8_t processingBuffer[MAX_PACKET_LENGTH];



static DataBuffer firstBuffer;
uint8_t firstBufferData[MAX_PACKET_LENGTH];
static DataBuffer secondBuffer;
uint8_t secondBufferData[MAX_PACKET_LENGTH];

void mainEventParserHandler(void* data);

//always returns USBD_OK, logical parsing is doing on the higher layers
uint8_t USB_ReportHandlerFunction(void* this, uint8_t* reportData)
{
   
    uint8_t header = reportData[0];
    
    if(header & USB_PACKET_DIRECTION_BIT_POSITION)
        // error: incorrect value of the direction bit
        return USBD_OK;

    UsbBridge* bridge = (UsbBridge*)this;
    //for sending in the event of main thread
    DataBuffer* bufferWithNewMessage = &(bridge->bufferUSB_RTX);
    
    BufferWritingResult res = bridge->bufferUSB_RTX.tryToWrite((void*)(&(bridge->bufferUSB_RTX)), reportData, MAX_PACKET_LENGTH);
    if(res == BUFFER_WRITING_BUSY)
    {
        // cannot just wait while buffer will be free, because interrupt has the highest priority.
        // need to save data anywhere and return control
        // second buffer using is allowed ONLY while first is busy. This guarantees that at least one buffer is always free
        res = bridge->bufferUSB_RTX_Secondary.tryToWrite((void*)(&(bridge->bufferUSB_RTX_Secondary)), reportData, MAX_PACKET_LENGTH);
        DataBuffer* bufferWithNewMessage = &(bridge->bufferUSB_RTX_Secondary);
    }
    if(res != BUFFER_WRITING_SUCCESFULL)
    {
        //report ID:out
        sendBuffer[0] = 2;
        
        //header of the response
        uint8_t responseHeader = header;
        responseHeader &= ~USB_PACKET_DIRECTION_BIT_POSITION;
        responseHeader |= USB_PACKET_DIRECTION_FROM_DEVICE;
        responseHeader &= ~USB_PACKET_STATUS_POSITION;
        
        // if we could not save data - return the error message
        if(res == BUFFER_WRITING_FULL)
        {
            //buffer is overflowed, but it's possible to do that later
            responseHeader |= USB_PACKET_STATUS_WAIT;
        }
        else
        {
            responseHeader |= USB_PACKET_STATUS_ERROR;
        }
        sendBuffer[1] = responseHeader;
        USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, sendBuffer, FULL_REPORT_LENGTH);
        return USBD_OK;
    }
    
    EventMessage message;
    message.handlerReference = mainEventParserHandler;
    message.eventParams = (void*)bufferWithNewMessage;
    
    EnqueueResult enqueueRes = eventQueue.tryToEnque((void*)(&eventQueue), message);
    if(enqueueRes == ENQUEUE_RESULT_FULL)
    {
        //clear buffer to avoid it's dead lock (it will be locked for writing until be readed)
        bufferWithNewMessage->tryToRead((void*)bufferWithNewMessage, &(reportData[1]), MAX_PACKET_LENGTH);
        
        //form and send wait-packet
        //report ID:out
        sendBuffer[0] = 2;
        
        //header of the response
        uint8_t responseHeader = header;
        responseHeader &= ~USB_PACKET_DIRECTION_BIT_POSITION;
        responseHeader |= USB_PACKET_DIRECTION_FROM_DEVICE;
        responseHeader &= ~USB_PACKET_STATUS_POSITION;
        
        //event queue is overflowed, but it's possible to do that later
        responseHeader |= USB_PACKET_STATUS_WAIT;

        sendBuffer[1] = responseHeader;
        USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, sendBuffer, FULL_REPORT_LENGTH);
        return USBD_OK;
    }
    return USBD_OK;
        
}

void mainEventParserHandler(void* data)
{
    DataBuffer* buffer = (DataBuffer*)data;
    BufferReadingResult res = buffer->tryToRead((void*)buffer, processingBuffer, MAX_PACKET_LENGTH);
    
    //to-do: error handling
    
    uint8_t header = processingBuffer[0]; 
    //status request processing
    if( (header & USB_PACKET_COMMAND_TYPE_POSITION) == USB_PACKET_COMMAND_TYPE_STATUS)
    {
        // process the status request
        if( (header & USB_PACKET_TYPE_POSITION) == USB_PACKET_TYPE_RESPONSE)
            //error: host can send status request only, not response
            return;
        if( (header & USB_PACKET_STATUS_POSITION ) != USB_PACKET_STATUS_OK)
            //error: status request with non-OK status - it's impossible, 
            //something went wrong, ignore the packet
            return;
        //send response
        uint8_t responseHeader = USB_PACKET_DIRECTION_FROM_DEVICE | USB_PACKET_COMMAND_TYPE_STATUS | 
                                USB_PACKET_TYPE_RESPONSE | USB_PACKET_STATUS_OK;
        sendBuffer[1] = responseHeader;
        //report ID:out
        sendBuffer[0] = 2;
        USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, sendBuffer, FULL_REPORT_LENGTH);
        return;
    }
    
    //file request processing
    if( (header & USB_PACKET_COMMAND_TYPE_POSITION) == USB_PACKET_COMMAND_TYPE_FILE)
    {
        // process the file packet
        if( (header & USB_PACKET_TYPE_POSITION) == USB_PACKET_TYPE_REQUEST)
        {   
            // if it's request
            if( (header & USB_PACKET_STATUS_POSITION ) != USB_PACKET_STATUS_OK)
            {
                if( (header & USB_PACKET_STATUS_POSITION ) == USB_PACKET_STATUS_WAIT)
                {
                    repeatLastSended();
                    return;
                }
                //error status received, terminate transmission
                terminateTransmission();
                return;
            }
            handleNewRequest(processingBuffer);
            return;
        }
        
        // if it's response
        if( (header & USB_PACKET_STATUS_POSITION ) != USB_PACKET_STATUS_OK)
        {
            if( (header & USB_PACKET_STATUS_POSITION ) == USB_PACKET_STATUS_WAIT)
            {
                repeatLastSended();
                return;
            }
            //error status reseived, terminate transmission
            terminateTransmission();
            return;
        }
        handleNewResponse(processingBuffer);
        return;
        
    }        
    
    //device request processing
    if( (header & USB_PACKET_COMMAND_TYPE_POSITION) == USB_PACKET_COMMAND_TYPE_DEVICE)
    {
        // process the device request
        if( (header & USB_PACKET_TYPE_POSITION) == USB_PACKET_TYPE_RESPONSE)
            //error: host can send device request only, not response
            return;
        if( (header & USB_PACKET_STATUS_POSITION ) != USB_PACKET_STATUS_OK)
            //error: device request with non-OK status - it's impossible, 
            //something went wrong, ignore the packet
            return;
        
        handleDeviceRequest(processingBuffer);
        
        uint8_t responseHeader = USB_PACKET_DIRECTION_FROM_DEVICE | USB_PACKET_COMMAND_TYPE_DEVICE | 
                                USB_PACKET_TYPE_RESPONSE | USB_PACKET_STATUS_OK;
        return;
    }
        
    //run request processing
    if( (header & USB_PACKET_COMMAND_TYPE_POSITION) == USB_PACKET_COMMAND_TYPE_RUN)
    {
        // process the run request
        if( (header & USB_PACKET_TYPE_POSITION) == USB_PACKET_TYPE_RESPONSE)
            //error: host can send run request only, not response
            return;
        if( (header & USB_PACKET_STATUS_POSITION ) != USB_PACKET_STATUS_OK)
            //error: run request with non-OK status - it's impossible, 
            //something went wrong, ignore the packet
            return;
        //run run request and send response
        startAll(HAL_GetTick());
        uint8_t responseHeader = USB_PACKET_DIRECTION_FROM_DEVICE | USB_PACKET_COMMAND_TYPE_RUN | 
                                USB_PACKET_TYPE_RESPONSE | USB_PACKET_STATUS_OK;
        return;
    }
    
    //error response processing
    if( (header & USB_PACKET_COMMAND_TYPE_POSITION) == USB_PACKET_COMMAND_TYPE_RUN)
    {
        // process the run request
        if( (header & USB_PACKET_TYPE_POSITION) == USB_PACKET_TYPE_REQUEST)
            //error: host can send error response only, not request
            return;
        if( (header & USB_PACKET_STATUS_POSITION ) != USB_PACKET_STATUS_OK)
        {
            //error: error request with non-OK status - all is very bad
            if( (header & USB_PACKET_STATUS_POSITION ) == USB_PACKET_STATUS_WAIT)
            {
                // TO_DO: invoke error supervior for resend error message
                return;
            }
            else
            {
                //error response to error request. All is very bad
                //TO_DO: CRUSH ALL
                return;
            }
        }
        //error request is received by host - no response is required
        return;
    }
}

void usbBridgeInit(UsbBridge* bridge)
{
    bridge->interruptSideHandler = USB_ReportHandlerFunction;
    bufferInit(&(bridge->bufferUSB_RTX), firstBufferData, MAX_PACKET_LENGTH);
    bufferInit(&(bridge->bufferUSB_RTX_Secondary), secondBufferData, MAX_PACKET_LENGTH);
}

