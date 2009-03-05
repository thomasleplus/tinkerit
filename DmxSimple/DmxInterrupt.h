/*
DmxInterrupt.h - DMX simple library
Copyright (c) 2008 Peter Knight, Tinker.it! All right reserved.
*/

#include <inttypes.h>

#if RAMEND <= 0x4FF
#define DMX_SIZE 128
#else
#define DMX_SIZE 512
#endif

extern volatile uint8_t dmxBuffer[DMX_SIZE];

void dmxWrite(int,uint8_t);
void dmxMaxChannel(int);