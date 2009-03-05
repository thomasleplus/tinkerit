/*
DmxSimple.cpp - DMX simple library
Copyright (c) 2008 Peter Knight, Tinker.it! All right reserved.
*/
/**
 * DmxSimple - A simple interface to DMX.
 */

#include "DmxSimple.h"
extern "C" {
	#include "DmxInterrupt.h"
}

/** Set DMX maximum channel
 * @param channel The highest DMX channel to use
 */
void DmxSimpleClass::maxChannel(int channel) {
  dmxMaxChannel(channel);
}

/** Write to a DMX channel
 * @param address DMX address in the range 1 - 512
 */
void DmxSimpleClass::write(int address, uint8_t value)
{
	dmxWrite(address, value);
}
DmxSimpleClass DmxSimple;
