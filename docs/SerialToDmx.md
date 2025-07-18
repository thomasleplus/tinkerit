#summary DmxSimple SerialToDmx example
#labels Arduino,DMX,DmxSimple

= Introduction =

A example showing how to control DmxSimple over a serial port.

= The code =
{{{
#include <DmxSimple.h>

// After installing, switch to Serial Monitor and set the baud rate to 9600.
//
// Type commands in the box and hit 'Send'.
//
// <number>c : Select a DMX channel
// <number>v : Set DMX channel to new value

void setup() {
  Serial.begin(9600);
  Serial.println("SerialToDmx ready");
  Serial.println();
  Serial.println("Syntax:");
  Serial.println(" 123c : use DMX channel 123");
  Serial.println(" 45w  : set current channel to value 45");
}

int value = 0;
int channel;

void loop() {
  int c;

  while(!Serial.available());
  c = Serial.read();
  if ((c>='0') && (c<='9')) {
    value = 10*value + c - '0';
  } else {
    if (c=='c') channel = value;
    else if (c=='w') {
      DmxSimple.write(channel, value);
      Serial.println();
    }
    value = 0;
  }
}
}}}

= Details =

The main loop processes serial input character by character. Digits are read to set the variable `value`. Then, special characters use `value` to do something.

The character 'c' sets the `channel` variable. The character 'w' triggers the DmxSimple write.

= Using !SerialToDmx with the Arduino Serial Monitor =

Set the serial monitor to 9600. You will then see the !SerialToDmx start message.

You can send commands by entering '*<DMX channel>*c*<channel value>*w' and pressing return (or pushing 'send').

= Using !SerialToDmx from Processing =

If you have myPort set up to talk to Arduino at 9600 baud, this function will give you full DMX control within Processing.
{{{
void setDmxChannel(int channel, int value) {
  // Convert the parameters into a message of the form: 123c45w where 123 is the channel and 45 is the value
  // then send to the Arduino
  myPort.write( str(channel) + "c" + str(value) + "w" );
}
}}}

Now you can call setDmxChannel(123,45) to set DMX channel 123 to value 45 within Processing.

Here is a more complicated example to select a colour from an RGB lamp using the mouse.
{{{
/*
Simple example of Processing controlling DMX devices

We are sending data from Processing to the Arduino DMX controller. So you will
need to run the Arduino code at the end of this program to get this to work.

DMX setup:
  A lamp responding to data on DMX channels 1 and 2

Arduino setup:
  Attach Tinker.it! DMX shield
  Load File > Sketchbook > Examples > Library-DmxSimple > SerialToDmx


by Peter Knight
http://www.tinker.it
05 Mar 2009
*/


import processing.serial.*;  // Import Serial library to talk to Arduino 
import processing.opengl.*; //  Import OpenGL to draw a gradient window

int channel1;  // create a variable to hold the data we are sending to the Arduino
int channel2;
Serial myPort; 

// Send new DMX channel value to Arduino
//
void setDmxChannel(int channel, int value) {
  // Convert the parameters into a message of the form: 123c45w where 123 is the channel and 45 is the value
  // then send to the Arduino
  myPort.write( str(channel) + "c" + str(value) + "w" );
}

// Draw gradient window
//
void drawGradient() {
  // Draw a colour gradient
  beginShape(QUADS);
  fill(0,0,255); vertex(0,0); // Top left BLUE
  fill(255,0,0); vertex(width,0); // Top right RED
  fill(255,255,0); vertex(width,height); // Bottom right RED + GREEN
  fill(0,255,255); vertex(0,height); // Bottom left BLUE + GREEN
  endShape(); 
}  

void setup() {
  println(Serial.list()); // shows available serial ports on the system
  // Change 0 to select the appropriate port as required.
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 9600);

  size(256,256,OPENGL);  // Create a window
  drawGradient();
}

void draw() {
  channel1 = (255 * mouseX / width); // Use cursor X position to get channel 1 value
  channel2 = (255 * mouseY / height);// Use cursor Y position to get channel 2 value
  setDmxChannel(1,channel1); // Send new channel values to Arduino
  setDmxChannel(2,channel2);
  setDmxChannel(3,255-channel1);
  
  // You may have to set other channels. Some lamps have a shutter channel that should be set to 255.
  // Set it here:
  setDmxChannel(4,255);
  
  delay(20);  // Short pause before repeating
}
}}}