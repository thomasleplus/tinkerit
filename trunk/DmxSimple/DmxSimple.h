/*
DmxSimple.h - DMX simple library
Copyright (c) 2008 Peter Knight, Tinker.it  All right reserved.
*/

#ifndef DmxSimple_h
#define DmxSimple_h

#include <inttypes.h>

class DmxSimpleClass
{
  public:
    void begin();
    void write(int, uint8_t);
};
extern DmxSimpleClass DmxSimple;

#endif
