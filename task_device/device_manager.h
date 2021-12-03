#ifndef DEVICE_MANAGER_H_
#define DEVICE_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {DEVICE_STATUS_FREE, DEVICE_STATUS_WAIT, DEVICE_STATUS_RECORDING, 
				DEVICE_STATUS_ERROR} DeviceStatus;
typedef enum {DEVICE_TYPE_SERVO, DEVICE_TYPE_GPIO_OUTPUT, DEVICE_TYPE_LED, DEVICE_TYPE_SPEAKER, 
				DEVICE_TYPE_BUTTON, DEVICE_TYPE_GPIO_DISABLED} DeviceType;

typedef void (*DeviceLinkFileFoo)(void* this, const char* fileName);
typedef void (*DeviceStart)(void* this, int32_t timeMS);
typedef void (*DeviceStop)(void* this);
typedef void (*DeviceHandle)(void* this, int32_t timeMS);
typedef struct
{
	DeviceType type;
  DeviceStatus status;
	bool isEnabled;
	const char* hardwareName;
	
	void* typeSpecifiedDescriptor;
	
	// for creating circular device list
	void* next;
	
	DeviceLinkFileFoo link;
	DeviceStart start;
	DeviceStop stop;
	
	// handle function is calling periodically during recording
	DeviceHandle handle;
}DeviceManagerBase;

#endif // DEVICE_MANAGER_H_
