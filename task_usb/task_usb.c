#include "task_usb.h"
#include "stream_file_buffer.h"

#include "usb_transfer_protocol.h"
#include "proto_defines.h"
#include "tusb.h"

#define QUEUE_LENGTH 10

static uint8_t messageBufferUSB[sizeof(TaskUSB_Msg) * QUEUE_LENGTH];
static StaticQueue_t usbStaticQueue;

#define STACK_SIZE 400
StackType_t usbTaskStack[ STACK_SIZE ];
StaticTask_t usbTaskBuffer;

#define TASK_PERIOD_MS 10

QueueHandle_t usbTaskMessageQueue;

static void usbTaskInitFoo(void* args);
static void handleMsg(TaskUSB_Msg msg);

static void initLowLevelProtocol(void);

typedef struct
{
    bool active;
    TickType_t nextTime;
} TimeOutManageDescriptor;

static TimeOutManageDescriptor usbTimeOutDescs[RECEIVING_BUFFERS_COUNT + 1];

static void timeOutSetterWrapper(uint8_t id, uint32_t time)
{
    if(time == 0)
        usbTimeOutDescs[id].active = false;
    else
    {
        usbTimeOutDescs[id].active = true;
        usbTimeOutDescs[id].nextTime = xTaskGetTickCount() + time;
    }
}

static void usbTimeOutInit()
{
    for(int i = 0; i < RECEIVING_BUFFERS_COUNT + 1; i++)
        usbTimeOutDescs[i].active = false;
}

static void handleTimeOuts()
{
    TickType_t currentTime = xTaskGetTickCount();
    for(int i = 0; i < RECEIVING_BUFFERS_COUNT + 1; i++)
    {
        if(usbTimeOutDescs[i].active)
        {
            int32_t signedDiff = currentTime - usbTimeOutDescs[i].nextTime;
            if(signedDiff > 0)
                handleTimeout(i);
        }
    }
}

static ConnectionProtoPacketHandler packetHandlers[MAX_HANDLERS_COUNT];

void sendMsgUSB(TaskUSB_Msg msg)
{
    xQueueSend(usbTaskMessageQueue, &msg, 0);
}

void initTaskUSB(void)
{
    usbTaskMessageQueue = xQueueCreateStatic(QUEUE_LENGTH, sizeof(TaskUSB_Msg), messageBufferUSB, &usbStaticQueue);
    configASSERT(usbTaskMessageQueue);
    
    TaskHandle_t usbTaskHandle = xTaskCreateStatic(
                  usbTaskInitFoo,
                  "USB_TASK",
                  STACK_SIZE,
                  ( void * ) 1,
                  tskIDLE_PRIORITY,
                  usbTaskStack,
                  &usbTaskBuffer );
    
    userInfoHandlerInit(&packetHandlers[USER_INFO_ID], (UserInfoShowMessage)NULL);
    userInfoHandlerInit(&packetHandlers[USER_INFO_ID], (UserInfoShowMessage)NULL);
                  
    ConnectionProtoUSB_Adapter* adapter = connectionUSB_AdapterLocate();
    connectionProtoUSB_AdapterInit(adapter);
    adapter->registerHandler(adapter, &packetHandlers[USER_INFO_ID], USER_INFO_ID);
    adapter->registerHandler(adapter, &packetHandlers[FILE_TRANSFER_ID], FILE_TRANSFER_ID);
    initLowLevelProtocol();
}


static void usbTaskInitFoo(void* args)
{   
		tusb_init();
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while(1) 
    {
				tud_task();
        // static for avoid stack allocation in thread
        //static uint8_t tempUSB_PacketBuffer[USB_PACKET_SIZE];
        
        //ConnectionProtoUSB_Adapter* adapter = connectionUSB_AdapterLocate();
        //adapter->sendIfCan(adapter);
        
        //TaskUSB_Msg msg;
        
        // 0 means do not wait for messages
        //BaseType_t res = xQueueReceive(usbTaskMessageQueue, &msg, 0);
        //if(res == pdTRUE)
        //    handleMsg(msg);
        //handleTimeOuts();
        //vTaskDelayUntil(&xLastWakeTime, TASK_PERIOD_MS);
    }
}



static void handleMsg(TaskUSB_Msg msg)
{
    switch(msg.type)
    {
        case CONNECTION_PROTO_PACKET_TYPE_USER_INFO:
        {
            USB_UserInfoMessage params = msg.params.userInfo;
            
            ConnectionProtoUSB_Adapter* adapter = connectionUSB_AdapterLocate();
            bool res = userInfoSendMessage(&packetHandlers[USER_INFO_ID], params.type, params.textSize, params.text);
            if(res)
            {
                // cannot process this message right now, let's do it later
                xQueueSend(usbTaskMessageQueue, &msg, 0);
            }
            // error: no free buffers
            // TO-DO: cast error message into server
            return;
        }
    }
}

static void usbSendingWrapper(uint8_t* data)
{
	
}
static void usbHandlingWrapper(uint8_t* data, uint32_t size)
{
    ConnectionProtoUSB_Adapter* adapter = connectionUSB_AdapterLocate();
    adapter->handle(adapter, data, size);
}

static void usbLL_SendingCallbackWrapper(UsbProtocolTransmittingResult res)
{
    ConnectionProtoUSB_Adapter* adapter = connectionUSB_AdapterLocate();
    if(res == USB_PROTOCOL_SUCCESS_TRANSMITTING)
    {
        adapter->callback(adapter, CONNECTION_PROTO_USB_ADAPTER_LOW_LEVEL_TRANSMISSION_RESULT_SUCCESS);
        return;
    }
    if(res == USB_PROTOCOL_NEED_TO_WAIT)
    {
        adapter->callback(adapter, CONNECTION_PROTO_USB_ADAPTER_LOW_LEVEL_TRANSMISSION_RESULT_WAIT);
        return;
    }
    adapter->callback(adapter, CONNECTION_PROTO_USB_ADAPTER_LOW_LEVEL_TRANSMISSION_RESULT_FAIL);
}

static void adapterLL_SendWrapper(uint8_t* data, PacketSizeType size)
{
    transmitMessage(data, size);
}

static void initLowLevelProtocol(void)
{
    usbTimeOutInit();
	
    setUsbSendingFunction(usbSendingWrapper);
    setUsbSendingCallback(usbLL_SendingCallbackWrapper);
    setTimeoutSetter(timeOutSetterWrapper);
    setHighLevelProtocolParser(usbHandlingWrapper);
    
    ConnectionProtoUSB_Adapter* adapter = connectionUSB_AdapterLocate();
    adapter->setSendLL(adapter, adapterLL_SendWrapper);
}

