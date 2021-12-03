#include "wav_file_parser.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

static uint8_t readingBuffer[8];
#define READ_WITH_ASSERTION(src, size) do{if(blockingRead(src, readingBuffer, size))\
				return FILE_PARSING_RESULT_ERROR_FILE_ENDED_UNEXPECTED;}while(0)

static bool blockingRead(StreamFileBuffer* src, uint8_t* dst, FileSize size);


FileParsingResult getWavFileHeader(StreamFileBuffer* source, WavFileHeader* dst)
{
		/*
    * wav file header structure:
    * bytes 0-3: "RIFF"
    * bytes 4-7: fileSize;
    * bytes 8-11: "WAVE"
    * bytes 12-15: "fmt\0"
    * bytes 16-19: 16 (formatDataLength)
    * bytes 20-21: formatType (1-PCM)
    * bytes 22-23: numberOfChannels 
    * bytes 24-27: SampleRate
    * bytes 28-31: ByteRate
    * bytes 32-33: BytesPerFrame
    * bytes 34-35: BitsPerSample
    * bytes 36-39: "data"
    * bytes 40-43: dataLength (bytes)
    */
		READ_WITH_ASSERTION(source, 4);
		if( (readingBuffer[0] != 'R') || (readingBuffer[1] != 'I') || 
					(readingBuffer[2] != 'F') || (readingBuffer[3] != 'F') )
					return FILE_PARSING_RESULT_ERROR_DATA;
		
		READ_WITH_ASSERTION(source, 4);
		uint32_t fileSize = *((uint32_t*)readingBuffer);
		
		READ_WITH_ASSERTION(source, 4);
		if( (readingBuffer[0] != 'W') || (readingBuffer[1] != 'A') || 
					(readingBuffer[2] != 'V') || (readingBuffer[3] != 'E') )
					return FILE_PARSING_RESULT_ERROR_DATA;
		
		READ_WITH_ASSERTION(source, 4);
		if( (readingBuffer[0] != 'f') || (readingBuffer[1] != 'm') || 
					(readingBuffer[2] != 't') )
					return FILE_PARSING_RESULT_ERROR_DATA;
		
		READ_WITH_ASSERTION(source, 4);
		uint32_t formatDataLength = *((uint32_t*)readingBuffer);
		if(formatDataLength != 16)
					return FILE_PARSING_RESULT_ERROR_DATA;
		
		READ_WITH_ASSERTION(source, 2);
		uint16_t formatType = *((uint16_t*)readingBuffer);
		if(formatType != 1)
					return FILE_PARSING_RESULT_ERROR_DATA;
		
		READ_WITH_ASSERTION(source, 2);
		uint16_t numberOfChannels = *((uint16_t*)readingBuffer);
		
		READ_WITH_ASSERTION(source, 4);
		uint32_t sampleRate = *((uint32_t*)readingBuffer);
		READ_WITH_ASSERTION(source, 4);
		uint32_t byteRate = *((uint32_t*)readingBuffer);
		
		READ_WITH_ASSERTION(source, 2);
		uint16_t bytesPerFrame = *((uint16_t*)readingBuffer);
		READ_WITH_ASSERTION(source, 2);
		uint16_t bitsPerSample = *((uint16_t*)readingBuffer);
		
		// check format
		if(bytesPerFrame != bitsPerSample * numberOfChannels / 8)
				return FILE_PARSING_RESULT_ERROR_DATA;
		if(byteRate != bytesPerFrame * sampleRate)
				return FILE_PARSING_RESULT_ERROR_DATA;
		
		READ_WITH_ASSERTION(source, 4);
		if( (readingBuffer[0] != 'd') || (readingBuffer[1] != 'a') || 
					(readingBuffer[2] != 't') || (readingBuffer[3] != 'a') )
					return FILE_PARSING_RESULT_ERROR_DATA;
		
		READ_WITH_ASSERTION(source, 4);
		uint32_t dataLength = *((uint32_t*)readingBuffer);
		uint32_t dataFrameNumber = dataLength / bytesPerFrame;
		
		dst->bitsPerSample = bitsPerSample;
		dst->bytesPerFrame = bytesPerFrame;
		dst->channelNumber = numberOfChannels;
		dst->sampleRate = sampleRate;
		dst->numberOfSamples = dataFrameNumber;
		return FILE_PARSING_RESULT_SUCCESS;
		
}


FileParsingResult getWavFileData(StreamFileBuffer* source, uint8_t* dst, WavFileHeader header)
{
		if(header.bytesPerFrame > 8)
				return FILE_PARSING_RESULT_ERROR_DATA;
		FileSize readed = source->read(source, readingBuffer, header.bytesPerFrame);
		if(readed < header.bytesPerFrame)
		{
				if(source->status == STREAM_BUFFER_STATUS_FILE_END)
						return FILE_PARSING_RESULT_ERROR_FILE_ENDED_UNEXPECTED;
				else
					return FILE_PARSING_RESULT_ERROR_TOO_LATE;
		}
		// read only first channel
		switch(header.bitsPerSample)
		{
				case 8:
						*dst = readingBuffer[0];
						break;
				case 16:
						*dst = (*(uint16_t*)readingBuffer >> 8);
						break;
				case 32:
						*dst = (*(uint32_t*)readingBuffer >> 24);
						break;
				default:
						return FILE_PARSING_RESULT_ERROR_DATA;
		}
		return FILE_PARSING_RESULT_SUCCESS;
}

static bool blockingRead(StreamFileBuffer* src, uint8_t* dst, FileSize size)
{
	FileSize readed = 0;
	while(readed < size)
		{
				readed += src->read(src, dst + readed, size - readed);
				if(readed < size)
				{
					if(src->status == STREAM_BUFFER_STATUS_FILE_END)
							// this check is required because the buffer could be updated
							// after reading attempt
							if(src->size == 0)
									return true;
					
					if(src->status == STREAM_BUFFER_STATUS_ERROR)
							return true;
				}
		}
		return false;
}
