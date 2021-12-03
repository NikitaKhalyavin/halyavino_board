#ifndef LED_FILE_PARSER_H_
#define LED_FILE_PARSER_H_

#include "stdint.h"
#include "stream_file_buffer.h"
#include "file_parsing_result.h"

typedef struct
{
    float timeInSeconds;
    float value;
} LedFilePoint;

    
typedef struct
{
		//must be "LED"
		char signature[3];
    uint32_t pointNumber;
} LedFileHeader;

// this one is blocking
FileParsingResult getLedFileHeader(StreamFileBuffer* source, LedFileHeader* dst);

//this one is non-blocking, if cannot read yet - error result
FileParsingResult getLedFilePoint(StreamFileBuffer* source, LedFilePoint* dst);


#endif // LED_FILE_PARSER_H_
