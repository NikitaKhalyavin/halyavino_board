#ifndef TASK_USB_H_
#define TASK_USB_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "connection_proto_defines.h"
#include "connection_proto_usb_adapter.h"
#include "user_info_handler.h"

typedef struct
{
    UserInfoMessageType type;
    uint16_t textSize;
    const char* text;
} USB_UserInfoMessage;

typedef struct
{
    ConnectionProtoPacketType type;
    union
    {
        USB_UserInfoMessage userInfo;
    }params;
    
} TaskUSB_Msg;

void initTaskUSB(void);

void sendMsgUSB(TaskUSB_Msg msg);

#endif // TASK_USB_H_
