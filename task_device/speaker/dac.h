#ifndef DAC_H
#define DAC_H
#include <stdint.h>

void setDacValue(uint8_t value);
void disableSpeaker(void);
void enableSpeaker(void);
void setTimerSampleRate(uint32_t sampleRate);

#endif
