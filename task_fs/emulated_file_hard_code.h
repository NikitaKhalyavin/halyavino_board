#ifndef EMULATED_FILE_HARD_CODE_H_
#define EMULATED_FILE_HARD_CODE_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct
{
    const uint8_t* data;
    uint32_t size;
    const char* name;
} EmulatedFileStorageDescriptor;
static const uint32_t EmulatedFileStorageDescriptorSize = sizeof(EmulatedFileStorageDescriptor);


static const uint8_t testFile1Data[] = {0, 1, 2, 3, 4}; 
static const uint32_t testFile1Size = sizeof(testFile1Data);
static const char testFile1Name[] = "test_file_1";
static const EmulatedFileStorageDescriptor testFile1 = {.data = testFile1Data, 
				.size = testFile1Size, .name = testFile1Name};

static const uint8_t testFileLedData[] = {
				0x6c, 0x65, 0x64, 0x05, 													// header, 5 points
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		//point 1, time = 0.0, value = 0.0
				0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f,		//point 2, time = 1.0, value = 1.0
				0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00,		//point 3, time = 3.0, value = 0.0
				0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x3f,		//point 4, time = 4.0, value = 0.5
				0x00, 0x00, 0x90, 0x40, 0x00, 0x00, 0x00, 0x00}; 	//point 5, time = 4.5, value = 0.0
static const uint32_t testFileLedSize = sizeof(testFileLedData);
static const char testFileLedName[] = "test_file_led";
static const EmulatedFileStorageDescriptor testFileLed = {.data = testFileLedData, 
				.size = testFileLedSize, .name = testFileLedName};

static const uint32_t emulatedFilesNumber = 2;
static EmulatedFileStorageDescriptor emulatedFiles[emulatedFilesNumber];

bool isEmulatedFilesListInitialized = false;
void initializeList(void)
{
    emulatedFiles[0] = testFile1;
		emulatedFiles[1] = testFileLed;
	
		//---------------------------------------------
		//	led emulated file data
		//---------------------------------------------
		/*
		testFileLedData[0] = 'l';
		testFileLedData[1] = 'e';
		testFileLedData[2] = 'd';
		testFileLedData[3] = 5;
	
		const float firstPointTime = 0.0f;
		const float firstPointValue = 0.0f;
		memcpy(&testFileLedData[4], &firstPointTime, 4);
		memcpy(&testFileLedData[8], &firstPointValue, 4);
	
		const float secondPointTime = 1.0f;
		const float secondPointValue = 1.0f;
		memcpy(&testFileLedData[12], &secondPointTime, 4);
		memcpy(&testFileLedData[16], &secondPointValue, 4);
	
		const float thirdPointTime = 3.0f;
		const float thirdPointValue = 0.0f;
		memcpy(&testFileLedData[20], &thirdPointTime, 4);
		memcpy(&testFileLedData[24], &thirdPointValue, 4);
	
		const float fourthPointTime = 4.0f;
		const float fourthPointValue = 0.5f;
		memcpy(&testFileLedData[28], &fourthPointTime, 4);
		memcpy(&testFileLedData[32], &fourthPointValue, 4);
	
		const float fifthPointTime = 4.5f;
		const float fifthPointValue = 0.0f;
		memcpy(&testFileLedData[36], &fifthPointTime, 4);
		memcpy(&testFileLedData[40], &fifthPointValue, 4);
		*/
		//-------------------------------------------------
		// Led emulated file data end
		//-------------------------------------------------
}

#endif // EMULATED_FILE_HARD_CODE_H_
