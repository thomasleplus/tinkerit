/*
DmxInterrupt.c - DMX simple library
Copyright (c) 2008 Peter Knight, Tinker.it! All right reserved.
*/

#include <avr/interrupt.h>
#include <util/delay.h>
#include "DmxInterrupt.h"

#define DMX_TX 3 // DMX transmit on pin 3
#define DMX_PORT PORTD
#define DMX_BIT 3
#define DMX_DDR DDRD

#if RAMEND <= 0x4FF
#define DMX_SIZE 128
#else
#define DMX_SIZE 512
#endif

static uint8_t DMX_BUFFER[DMX_SIZE];
static uint16_t DMX_MAX=0;
static uint8_t DMX_STARTED=0;

void dmxBegin(int maxChannel)
{
  // Set DMX pin to output
  DMX_DDR |= _BV(DMX_BIT);
  
  // Initialise DMX frame interrupt
  TCCR1A = 0;
  TCCR1B = _BV(WGM13) | _BV(CS12);
  ICR1 = F_CPU / 12800;
  TCNT1 = 0;
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)
  TIMSK1 = _BV(TOIE1);
#elif defined(__AVR_ATmega8__)
  TIMSK = _BV(TOIE1);
#else
#warning "DmxSimple does not support this CPU"
#endif
  DMX_STARTED = 1;
  if ((maxChannel > 0) && (maxChannel <= 512)) DMX_MAX = maxChannel;
}

void dmxSendByte(volatile uint8_t value)
{
  uint8_t bitCount, delCount;
  __asm__ volatile (
    "cbi %[outPort],%[outBit]\n"
    "nop\nnop\nnop\nnop\n"
  //  "rjmp branch1%=\n"  // Delay 2 t-states (more code efficient than two nops)
  //"branch1%=:\n"
  //  "rjmp branch2%=\n" // And another 2. This is to align the start bit to later transitions.
  //"branch2%=:\n"
    "ldi %[bitCount],11\n" // 11 bit intervals per transmitted byte
  "bitLoop%=:\n"
#if F_CPU == 8000000
    "nop\n"
    "ldi %[delCount],7\n" // 7 loops to hit exact 4us bit time on 8MHz clock
#elif F_CPU == 16000000
    "ldi %[delCount],18\n" // 18 loops to hit exact 4us bit time on 16MHz clock
#elif F_CPU == 20000000
    "nop\n"
    "ldi %[delCount],23\n" // 23 loops to hit exact 4us bit time on 20MHz clock
#else
    #warning "DmxSimple does not support this clock speed"
#endif
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
  uint16_t j;
  DMX_PORT &= ~_BV(DMX_BIT);
  for (i=0; i<11; i++) _delay_us(8);
  DMX_PORT |= _BV(DMX_BIT);
  _delay_us(8); // MAB 8us
  dmxSendByte(0); // Send start code
  for (j=0; j<DMX_MAX; j++) dmxSendByte(DMX_BUFFER[j]);
}

void dmxWrite(int channel, uint8_t value) {
  if (!DMX_STARTED) dmxBegin(0);
  if ((channel > 0) && (channel <= DMX_SIZE)) {
    if (value<0) value=0;
    if (value>255) value=255;
    DMX_MAX = (channel > DMX_MAX) ? channel : DMX_MAX;
    DMX_BUFFER[channel-1] = value;
  }
}
