
#include "small_file_reader.h"

FileSystemResultMessage readSmallFileToBuffer(uint8_t buffer[SMALL_FILE_BUFFER_SIZE], unsigned int * bytesRead, const char* fileName)
{
    FRESULT res;
    FIL msgFile;
    
    res = f_open(&msgFile, fileName, FA_READ);
    if(res != FR_OK) 
        return FILE_NOT_OPENED;
    
    if(f_size(&msgFile) > SMALL_FILE_BUFFER_SIZE)
        return FILE_SIZE_ERROR;
    
    res = f_read(&msgFile, buffer, SMALL_FILE_BUFFER_SIZE-1, bytesRead);
    if(res != FR_OK) 
        return FILE_READING_ERROR;
    
    if(*bytesRead == SMALL_FILE_BUFFER_SIZE-1)
        return FILE_SIZE_ERROR;
    
    res = f_close(&msgFile); 
    if(res != FR_OK) 
        return FILE_CLOSING_ERROR;
    
    return FILE_SUCCESS;
    
}