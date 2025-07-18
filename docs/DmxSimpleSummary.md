# Introduction

Control spot lamps, floods, wall washers, lasers and smoke machines
safely and easily from Arduino using DMX. High power low cost
controllable lighting with safe low voltage control.

# DMX is ? {#dmx_is}

DMX is a standardised system for communicating with lamps. In the same
way that most electronic musical instruments have a MIDI port, many
lamps have a DMX port. DMX controlled lights and visual effects are
available from most DJ and stage/studio supply shops. They are also
easily available by mail order.

# Arduino is ? {#arduino_is}

Arduino is a low cost, open source electronics / computing prototyping
platform with lots of helpful libraries and a strong support community.

# DmxSimple is ? {#dmxsimple_is}

DmxSimple is a library for Arduino. It handles all the tricky parts of
the DMX protocol internally, and provides a simple yet powerful
interface to your own sketches.

# How powerful is DmxSimple? {#how_powerful_is_dmxsimple}

Some projects using DmxSimple:

` * `[`Kohtausjone`](http://www.tasankokaiku.com/jarse/?p=268)` - a strobe pedal for live performance`\
` * `[`Forcefield`](http://www.tinker.it/en/Projects/Forcefield)` - an interactive RFID / colour sensing installation`\
` * `[`Hop`](http://www.tinker.it/en/Projects/Hop)` - Hop Scotch reimagined for the new millenium`\
` * `[`Time Table`](http://vimeo.com/6944430)\
` * `[`Music activated DMX light`](http://www.youtube.com/watch?v=GJfbM0zuRVk)

# How simple is DmxSimple? {#how_simple_is_dmxsimple}

`{{{
#include <DmxSimple.h>

int brightness;

void setup() {
}

void loop() {
  for (brightness = 0; brightness <= 255; brightness++) {
    DmxSimple.write(1, brightness);
    delay(10);
  }
}
}}}`{=mediawiki}

Very simple. Forget about sending the DMX frame in the appropriate
sequence. It is all handled for you. \`DmxSimple.write()\` behaves in an
almost identical way to \`analogWrite()\`. The only difference is that
the DmxSimple version uses DMX channels, not Arduino pins.

# \[DmxSimpleInstallation Installing DmxSimple\...\] {#dmxsimpleinstallation_installing_dmxsimple...}
