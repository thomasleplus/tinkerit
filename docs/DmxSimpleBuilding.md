# Building a DMX interface {#building_a_dmx_interface}

DmxSimple has two modes.

## External interface mode {#external_interface_mode}

This is the default mode. An external circuit converts the digital
signals into a level suitable for driving the DMX signal. It uses one
small integrated circuit - the SN75176A differential transceiver chip.
<http://www.arduino.cc/playground/uploads/DMX/send_sn75276a.jpg>

This design and others are [listed on the Arduino
Playground.](http://www.arduino.cc/playground/DMX/DMXShield)

## Direct drive mode {#direct_drive_mode}

In this mode, DMX signals are sent directly from Arduino without any
extra interface circuitry. This system great for small or test
installations, or just getting started quickly. However, it is not so
good at driving long cables or large numbers of DMX lamps at once.

# \[DmxSimpleUsing Using DmxSimple\...\] {#dmxsimpleusing_using_dmxsimple...}
