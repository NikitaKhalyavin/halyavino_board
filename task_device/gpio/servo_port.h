#ifndef SERVO_PORT_H_
#define SERVO_PORT_H_

#include <stdint.h>
#include <stdbool.h>

bool servoSetTimer(uint32_t timeMCS);
void servoResetTimer(void);

#endif // SERVO_PORT_H_
