#include "script_parser.h"
#include <fatfs.h>
#include <string.h>

#include "global_device_manager.h"

typedef enum {SCRIPT_PARSER_READING_DEVICE_NAME, SCRIPT_PARSER_READING_FILE_NAME} ScriptParserState;

static void applyCommand(char* device, char* file)
{
    
    char* speakerName = "SPEAKER";
    
    char* led1Name = "LED1";
    char* led2Name = "LED2";
    char* led3Name = "LED3";
    char* led4Name = "LED4";
    char* led5Name = "LED5";
    char* led6Name = "LED6";
    
    uint32_t diff;
    diff = strcmp(device, speakerName);
    if(diff == 0)
        setWavFile(file);
    
    diff = strcmp(device, led1Name);
    if(diff == 0)
        setLedFile(LED_CHANNEL_1, file);
    
    diff = strcmp(device, led2Name);
    if(diff == 0)
        setLedFile(LED_CHANNEL_2, file);
    
    diff = strcmp(device, led3Name);
    if(diff == 0)
        setLedFile(LED_CHANNEL_3, file);
    
    diff = strcmp(device, led4Name);
    if(diff == 0)
        setLedFile(LED_CHANNEL_4, file);
    
    diff = strcmp(device, led5Name);
    if(diff == 0)
        setLedFile(LED_CHANNEL_5, file);
    
    diff = strcmp(device, led6Name);
    if(diff == 0)
        setLedFile(LED_CHANNEL_6, file);
    
}

static void parseFile(char* file, uint32_t size)
{
    //first separator is " ", second - '\n' (new line), '\r' is ignoring
    int cursor = 0;
    
    ScriptParserState state = SCRIPT_PARSER_READING_DEVICE_NAME;
    
    char deviceName[10];
    uint32_t deviceNameIndex = 0;
    
    char fileName[50];
    uint32_t fileNameIndex = 0;
    
    for(;cursor < size; cursor++)
    {
        char symbol = file[cursor];
        if(symbol == '\r')
            continue;
        switch (state)
        {
            case SCRIPT_PARSER_READING_DEVICE_NAME:
            {
                if(symbol == ' ')
                {
                    state = SCRIPT_PARSER_READING_FILE_NAME;
                    deviceName[deviceNameIndex++] = '\0';
                    fileNameIndex = 0;
                    continue;
                }
                if(symbol == '\n')
                {
                    //incorrect string - return
                    return;
                }
                deviceName[deviceNameIndex++] = symbol;
                break;
            }
            case SCRIPT_PARSER_READING_FILE_NAME:
            {
                if(symbol == '\n')
                {
                    state = SCRIPT_PARSER_READING_FILE_NAME;
                    deviceName[deviceNameIndex++] = '\0';
                    deviceNameIndex = 0;
                    applyCommand(deviceName, fileName);
                }
                fileName[fileNameIndex++] = symbol;    
                break;
            }
        }
    }
    startAll(HAL_GetTick());
}


void executeScriptFile(char* fileName)
{
    FIL file;
    FRESULT res;
    res = f_open(&file, fileName, FA_READ);
    if(res != FR_OK)
        return;
    
    uint32_t size = f_size(&file);
    
    const uint32_t MAX_SCRIPT_SIZE = 512;
    if(size > MAX_SCRIPT_SIZE)
        return;
    
    char readBuffer[MAX_SCRIPT_SIZE];
    uint32_t readed = 0;
    res = f_read(&file, readBuffer, size, &readed);
    
    if(res != FR_OK)
        return;
    if(readed < size)
        return;
    
    parseFile(readBuffer, readed);
    
}

