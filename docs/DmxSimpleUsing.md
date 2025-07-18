#summary Using DmxSimple

= Using DmxSimple - FadeUp =

In the Arduino software, view the example "DmxSimple > FadeUp"

{{{
#include <DmxSimple.h>
}}}
All sketches that use DmxSimple must include this line near the top of a sketch. This tells the Arduino system to include the DmxSimple library. This line can be automatically inserted from the Arduino menu using Sketch > Import Library... > DmxSimple.

{{{
void setup() {
  DmxSimple.usePin(3);
}}}

DmxSimple.usePin() tells the DmxSimple library which pin to output DMX signals on. By default, DmxSimple outputs on pin 3 - however, any digital pin can be used.

{{{
  DmxSimple.maxChannel(4);
}
}}}

DmxSimple.maxChannel() is an optional command setting the highest DMX channel that the program will use.

{{{
void loop() {
  int brightness;
  /* Simple loop to ramp up brightness */
  for (brightness = 0; brightness <= 255; brightness++) {
}}}

This code loops brightness from 0 to 255.

{{{    
    /* Update DMX channel 1 to new brightness */
    DmxSimple.write(1, brightness);
}}}

This sets DMX channel 1 to the brightness level
   
{{{
    /* Small delay to slow down the ramping */
    delay(10);
}}}

DmxSimple.write() happens instantaneously (it takes approximately one millionth of a second to execute). For most situations, animations will need to be slowed down to make them visible. This 10ms delay means the entire fade takes about 2.5 seconds.

{{{
  }

}
}}}