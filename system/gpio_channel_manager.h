#ifndef GPIO_CHANNEL_MANAGER_H
#define GPIO_CHANNEL_MANAGER_H

#include <main.h>
#include <stdbool.h>

// global GPIO types
typedef enum {GPIO_CHANNEL_1, GPIO_CHANNEL_2, GPIO_CHANNEL_3, GPIO_CHANNEL_4, 
        GPIO_CHANNEL_5, GPIO_CHANNEL_6, GPIO_CHANNEL_7, GPIO_CHANNEL_8} GPIO_Channel;
typedef enum {GPIO_MODE_BUTTON, GPIO_MODE_SERVO, GPIO_MODE_OFF} GPIO_Mode;

// button types
typedef enum {GPIO_BUTTON_PRESSED, GPIO_BUTTON_RELAXED} GPIO_ButtonState;
typedef bool (*GetGPIO_ButtonValue)(void* this);
typedef void (*SetFileName)(void* this, char* fileName);

typedef struct
{
    GPIO_ButtonState currentState;
    GetGPIO_ButtonValue getValue;
    SetFileName setFile;
    char* fileName;
    
    // private value
    uint32_t lastChangeTime;
    bool isReadyForNextChange;
} GPIO_ButtonDescriptor;


// servo types
typedef enum {GPIO_SERVO_ENABLED, GPIO_SERVO_DISABLED} GPIO_ServoState;
typedef enum {SERVO_SUCCESS, SERVO_INTERNAL_ERROR, SERVO_UNREACHEABLE_ANGLE_ERROR} ServoResult;
typedef void (*ServoTimerCallback)(void* this);
typedef void (*ServoTimerCallback)(void* this);
typedef ServoResult (*SetServoAngle)(void* this, float angle);

typedef struct
{
    GPIO_ServoState currentState;
    SetServoAngle setAngle;
    
    uint32_t pulseDuration;
    ServoTimerCallback onCallback;
    ServoTimerCallback offCallback;
} GPIO_ServoDescriptor;


// global GPIO manager
typedef void (*ConfigToMode)(void* this, GPIO_Mode mode);
typedef struct
{
    GPIO_Channel channel;
    GPIO_Mode mode;
    ConfigToMode config;
    
    union
    {
        GPIO_ButtonDescriptor button;
        GPIO_ServoDescriptor servo;
    } modeSpecified;
    
}GPIO_Manager;

void GPIO_Init(GPIO_Manager* manager);

#endif
