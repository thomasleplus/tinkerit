#summary Building a DmxSimple DMX interface

= Building a DMX interface =

DmxSimple has two modes.

== External interface mode ==

This is the default mode. An external circuit converts the digital signals into a level suitable for driving the DMX signal. It uses one small integrated circuit - the SN75176A differential transceiver chip.
http://www.arduino.cc/playground/uploads/DMX/send_sn75276a.jpg

This design and others are [http://www.arduino.cc/playground/DMX/DMXShield listed on the Arduino Playground.]

== Direct drive mode ==

In this mode, DMX signals are sent directly from Arduino without any extra interface circuitry. This system great for small or test installations, or just getting started quickly. However, it is not so good at driving long cables or large numbers of DMX lamps at once.

= [DmxSimpleUsing Using DmxSimple...] =