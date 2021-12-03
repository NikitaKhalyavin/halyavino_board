#include "led_file_parser.h"

#include <stdbool.h>
#include <string.h>

static uint8_t readBuffer[8];

/*
* led file header structure:
* bytes 0-2: 'l', 'e', 'd' - file signature
* byte 3: number of data points
* other data: points in format:
* 4 bytes - float time
* 4 bytes - float value
*/

FileParsingResult getLedFileHeader(StreamFileBuffer* source, LedFileHeader* dst)
{
		if ( (source->status == STREAM_BUFFER_STATUS_IDLE) || (source->status == STREAM_BUFFER_STATUS_ERROR) )
				// buffer is not linked
				return FILE_PARSING_RESULT_BUFFER_ERROR;
		
		FileSize readed = 0;
		
		// read 4 bytes with waiting
		const FileSize headerSize = 4;
		while(readed < headerSize)
		{
				readed += source->read(source, readBuffer + readed, headerSize - readed);
				if(readed < headerSize)
				{
					if(source->status == STREAM_BUFFER_STATUS_FILE_END)
							// this check is required because the buffer could be updated
							// after reading attempt
							if(source->size == 0)
									return FILE_PARSING_RESULT_ERROR_FILE_ENDED_UNEXPECTED;
					
					if(source->status == STREAM_BUFFER_STATUS_ERROR)
							return FILE_PARSING_RESULT_BUFFER_ERROR;
				}
		}
		
		dst->signature[0] = readBuffer[0];
		dst->signature[1] = readBuffer[1];
		dst->signature[2] = readBuffer[2];
		dst->pointNumber 	= readBuffer[3];
		
		return FILE_PARSING_RESULT_SUCCESS;
}


FileParsingResult getLedFilePoint(StreamFileBuffer* source, LedFilePoint* dst)
{
		if ( (source->status == STREAM_BUFFER_STATUS_IDLE) || (source->status == STREAM_BUFFER_STATUS_ERROR) )
				// buffer is not linked
				return FILE_PARSING_RESULT_BUFFER_ERROR;
		
		if(source->status == STREAM_BUFFER_STATUS_FILE_END)
				if(source->size == 0)
						return FILE_PARSING_RESULT_EOF;
		
		const FileSize pointSize = 8;
		FileSize readed = source->read(source, readBuffer, pointSize);
		if(readed < pointSize)
		{
				// error - points have to be readed instant
				if(source->status == STREAM_BUFFER_STATUS_FILE_END)
						return FILE_PARSING_RESULT_ERROR_FILE_ENDED_UNEXPECTED;
				if(source->status == STREAM_BUFFER_STATUS_IN_PROCESS)
						return FILE_PARSING_RESULT_ERROR_TOO_LATE;
				
				return FILE_PARSING_RESULT_BUFFER_ERROR;
		}
		
		const uint32_t floatSize = 4;
		memcpy(&dst->timeInSeconds, readBuffer, floatSize);
		memcpy(&dst->value, &readBuffer[4], floatSize);
		return FILE_PARSING_RESULT_SUCCESS;
		
}
