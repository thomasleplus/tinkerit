#include <DmxSimple.h>

void setup() {
  DmxSimple.maxChannel(4); // Talking to a four channel receiver, so send all four channels.
}

void loop() {
  int brightness;  
  for (brightness = 0; brightness <= 255; brightness++) {
    DmxSimple.write(1, brightness); // Set DMX channel 1 to new value
    delay(10); // Wait 10ms
  }
}
