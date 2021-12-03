#ifndef EXECUTING_MANAGER_H_
#define EXECUTING_MANAGER_H_

#include <stdint.h>

typedef enum {EXECUTING_STATUS_IDLE, EXECUTING_STATUS_RECORDING, 
			EXECUTING_STATUS_PREPARING, EXECUTING_STATUS_NOT_CONFIGURED} ExecutingStatus; 

void executingManagerInit(void);

void executingManagerReconfigBoard(int32_t timeMS);

void executingManagerHandle(int32_t timeMS);
			
void executingManagerGetScriptCmd(const char* fileName);
			
// to be defined by user
void executingManagerThreadDelay(void);

#endif // EXECUTING_MANAGER_H_
