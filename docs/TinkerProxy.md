#summary TinkerProxy Bridge between Arduino and Flash
#labels Featured

= Introduction =

!TinkerProxy is software that allows flash application to access serial ports on a computer. Since Flash applications are running inside a sandbox in order to reduce any security issue with software downloaded from the Internet, they are barred from accessing hardware devices like serial ports. Developers of interactive installation, on the other hand, have the need to access serial ports in order to communicate with interface boards and sensors.

!TinkerProxy solves the problem by connecting on one side to a serial port and on the other side acting as a TCP/IP server. Since Flash applications are allowed to open network connections they can communicate with !TinkerProxy and have access to the required serial port.

= Setup =

[http://tinkerit.googlecode.com/files/tinkerproxy-2_0.zip TinkerProxy] is based on one single executable file and a corresponding configuration file.

Place the executable file serproxy.exe and serproxy.cfg in a suitable folder.

Edit the configuration file depending on your needs.

Let's look at serproxy.cfg

the important parts are :

{{{
# Comm ports used 4 means COM4
comm_ports=4,5,6,7,8
}}}

this line defines which com ports are mapped by the proxy (you can add to the list and also skip numbers i.e. 4,6,9,22,23)

then for each port you have a section like this:

{{{
# Port 7 settings COM7:
net_port7=5334
comm_baud7=9600
comm_databits7=8
comm_stopbits7=1
comm_parity7=none
}}}

These are the values used to map a certain serial port (COM7 in this case) with a network port also specifying the speed of the port.

{{{
# Idle time out in seconds
timeout=300
}}}

means "the proxy will disconnect a port if it is idle for more than 300 secs" put a huge number to disable timeout

This is very important

{{{
# Transform newlines coming from the serial port into nils
# true (e.g. if using Flash 8) or false
newlines_to_nils=true
}}}

this instructs the proxy to replace the newlines coming from the serial port with a byte of value 0. This feature is very important for ActionScript 2.0 because the only type of network connection available (XMLSocket) won't process any data unless it's terminated by a byte 0. Most devices use the newline code to terminate a line of text so the XMLSocket , without this feature, will never process any data.

The configuration can also be edited using [http://tinkerit.googlecode.com/files/TPConfig.zip TinkerProxy Configurator]

= Running =

once the config is ready from the windows command line run serproxy.exe and this will display a welcome banner now you can test it from flash or just by typing {{{telnet localhost 5334}}} (replace 5334 with the network port to test)

= Code examples =

We have put together two simple examples on how to use !TinkerProxy, one for AS2 and one for AS3.

Please note that these are still quite rough at the moment

  * [http://tinkerit.googlecode.com/files/example_as2.zip ActionScript 2.0 Example]
  * [http://tinkerit.googlecode.com/files/example_as3.zip ActionScript 3.0 Example] 