#ifndef FS_API_H_
#define FS_API_H_

#include "stream_file_buffer.h"
#include "common_file_defines.h"

// WARNING!!! All these functions are blocking!!!

typedef enum {FS_API_RESULT_SUCCESS, FS_API_RESULT_NOT_FOUND, FS_API_RESULT_INT_ERR, FS_API_RESULT_REQUEST_ERROR} FS_ApiResult;

FS_ApiResult linkFileToBuffer(StreamFileBuffer* buffer, const char* fileName, FileTransferDirection dir, FileStorageDevice device);

FS_ApiResult unlinkFileFromBuffer(StreamFileBuffer* buffer);

FS_ApiResult runFilesystemOverview(StreamFileBuffer* buffer);

void initTaskFS(void);

#endif // FS_API_H_
