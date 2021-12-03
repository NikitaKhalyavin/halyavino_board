#ifndef WAV_FILE_PARSER_H_
#define WAV_FILE_PARSER_H_

#include "stdint.h"
#include "stream_file_buffer.h"
#include "file_parsing_result.h"


typedef struct
{
    uint16_t channelNumber;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
    uint16_t bytesPerFrame;
    
    int32_t numberOfSamples;
    
} WavFileHeader;

// this one is blocking
FileParsingResult getWavFileHeader(StreamFileBuffer* source, WavFileHeader* dst);

//this one is non-blocking, if cannot read yet - error result
//for using in ISR only
FileParsingResult getWavFileData(StreamFileBuffer* source, uint8_t* dst, WavFileHeader header);

#endif // WAV_FILE_PARSER_H_
