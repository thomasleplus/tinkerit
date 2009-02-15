/*
DmxSimple.cpp - DMX simple library
Copyright (c) 2008 Peter Knight, Tinker.it! All right reserved.
*/

#include "DmxSimple.h"
extern "C" {
	#include "DmxInterrupt.h"
}
void DmxSimpleClass::begin(void) {
	dmxBegin(0);
}
void DmxSimpleClass::begin(int maxChannel) {
	dmxBegin(maxChannel);
}
void DmxSimpleClass::write(int address, uint8_t value)
{
	dmxWrite(address, value);
}
DmxSimpleClass DmxSimple;
