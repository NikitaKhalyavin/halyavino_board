#ifndef WAV_MANAGER_H
#define WAV_MANAGER_H

#include "device_manager.h"
#include "stream_file_buffer.h"
#include "wav_file_parser.h"

#define SPEAKER_BUFFER_SIZE 512

typedef enum {SPEAKER_STATUS_ISR_PROCESSING, SPEAKER_STATUS_ISR_IDLE, SPEAKER_STATUS_ISR_ERROR} SpeakerStatusISR;

typedef struct
{
    DeviceManagerBase base;
    
    StreamFileBuffer buffer;
    uint8_t bufferData[SPEAKER_BUFFER_SIZE];
    
		WavFileHeader header;	
    uint32_t samplesReaded;
		bool isFileLinked;
		SpeakerStatusISR statusISR;
	
} SpeakerManager;

void speakerManagerInit(SpeakerManager* this);
void speakerHandleFromISR(SpeakerManager* this);

#endif
