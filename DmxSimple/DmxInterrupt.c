#include <avr/interrupt.h>
#include <util/delay.h>
#include "DmxInterrupt.h"

#define DMX_TX 3 // DMX transmit on pin 4
#define DMX_PORT PORTD
#define DMX_BIT 3
#define DMX_DDR DDRD

#define DMX_SIZE 128
static uint8_t DMX_BUFFER[DMX_SIZE];

void dmxBegin()
{
	// Set DMX pin to output
	DMX_DDR |= _BV(DMX_BIT);
	
	// Initialise DMX frame interrupt
	TCCR1A = 0;
	TCCR1B = _BV(WGM13) | _BV(CS12);
	ICR1 = 1250;
	TCNT1 = 0;
	TIMSK1 = _BV(TOIE1);
}

void dmxSendByte(volatile uint8_t value)
{
  uint8_t bitCount, delCount;
  __asm__ volatile (
    "cbi %[outPort],%[outBit]\n"
    "nop\n nop\n nop\n nop\n" // Timing tweak
    "ldi %[bitCount],11\n" // 11 bit intervals per transmitted byte
  "bitLoop%=:\n"
    "ldi %[delCount],18\n" // 18 loops to hit exact 4us bit time on 16MHz clock
  "delLoop%=:\n"
    "dec %[delCount]\n"
    "brne delLoop%=\n"
    "sbrc %[value],0\n"
    "sbi %[outPort],%[outBit]\n"
    "sbrs %[value],0\n"
    "cbi %[outPort],%[outBit]\n"
    "lsr %[value]\n"
    "ori %[value],128\n" // After sending out 8 data bits, send out high state
    "dec %[bitCount]\n"
    "brne bitLoop%=\n"
    :[bitCount] "=&d" (bitCount), [delCount] "=&d" (delCount)
    :[outPort] "I" (_SFR_IO_ADDR(DMX_PORT)), [outBit] "I" (DMX_BIT), [value] "r" (value)
  );
}

SIGNAL(TIMER1_OVF_vect) {
	uint8_t i;
	DMX_PORT &= ~_BV(DMX_BIT);
	for (i=0; i<11; i++) _delay_us(8);
	DMX_PORT |= _BV(DMX_BIT);
	_delay_us(8); // MAB 8us
	dmxSendByte(0); // Send start code
	for (i=0; i<DMX_SIZE; i++) dmxSendByte(DMX_BUFFER[i]);
}
void dmxWrite(int channel, uint8_t value) {
	if ((channel > 0) && (channel <=128)) {
		if (value<0) value=0;
		if (value>255) value=255;
		DMX_BUFFER[channel-1] = value;
	}
}
