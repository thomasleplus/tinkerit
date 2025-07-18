#summary DmxSimple.write()

= DmxSimple.write(channel, value) =

= Function =

Updates a DMX channel to a new value.

DmxSimple maintains a buffer of all the DMX channels, and transmits them on the next transmission cycle. DmxSimple.write() is used exactly like analogWrite(). There is no visible delay between writing a new channel and the response of the DMX lamp - the whole process happens in around 20ms.

= channel =
channel is an integer (`int`). Valid values are between 1 and 512 inclusive, which correspond to DMX channels 1 to 512. Writes to invalid channels are ignored.

= value =
value is an integer of 8 bits. Valid values are between 0 and 255 inclusive, which correspond to DMX levels between 0 (off) and 255 (full).

The meaning of particular values for particular channels can change depending on which DMX lamp is being used. Typically 0 means off and 255 means full on, but this can change. Consult your lamp manual.


= Side effects =

DmxSimple.write() automatically initialises the DMX library if that is required. It also adjusts maxChannel if you write to a channel above maxChannel's limit.

For most situations, DmxSimple.write() is the only command you will ever need.