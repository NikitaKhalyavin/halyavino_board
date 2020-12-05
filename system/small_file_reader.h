/*
This module is used for reading only one-sector files (with length 512 bytes or less)
For larger files use throw_file_reader module
*/

#ifndef SMALL_FILE_READER_H
#define SMALL_FILE_READER_H

#define SMALL_FILE_BUFFER_SIZE 512

#include "filesystem_status.h"
#include "fatfs.h"

FileSystemResultMessage readSmallFileToBuffer(uint8_t buffer[SMALL_FILE_BUFFER_SIZE], unsigned int * bytesRead, const char* fileName); 

#endif
