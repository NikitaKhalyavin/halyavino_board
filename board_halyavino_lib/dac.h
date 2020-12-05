#ifndef DAC_H
#define DAC_H
#include <stdint.h>

void setDacValue(uint8_t value);
void disableSpeaker();
void enableSpeaker();

#endif