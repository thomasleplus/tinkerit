#include "DmxSimple.h"
extern "C" {
	#include "DmxInterrupt.h"
}
void DmxSimpleClass::begin() {
	dmxBegin();
}
void DmxSimpleClass::write(int address, uint8_t value)
{
	dmxWrite(address, value);
}
DmxSimpleClass DmxSimple;
