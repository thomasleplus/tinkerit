#summary DmxSimple FadeUp example
#labels Arduino,DMX,DmxSimple

= Introduction =

A simple example showing how to use DmxSimple on Arduino.

= The code =
{{{
#include <DmxSimple.h>

void setup() {
}

void loop() {
  int brightness;  
  for (brightness = 0; brightness <= 255; brightness++) {
    DmxSimple.write(1, brightness); // Set DMX channel 1 to new value
    delay(10); // Wait 10ms
  }
}
}}}

= Details =

This code fades DMX channel 1 from off to full on, then snaps back to off again and repeats.

`brightness` increments in a `for` loop. Each time around, the `DmxSimple.write()` command updates the DMX network.

The `delay` slows down the fading so it is visible.