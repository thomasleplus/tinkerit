#include <DmxInterrupt.h>
#include <DmxSimple.h>

int dmxChannel = 1;
int dmxTotalChannels = 4;
void setup() {
  // If DMX needs to send a minimum number of channels, set it here.
  DmxSimple.begin(dmxTotalChannels); // Talking to a four channel receiver, so send all four channels.
  // If the begin() is omitted, DmxSimple works out the maximum channel automatically.
}

void loop() {
  int brightness;  
  for (brightness = 0; brightness <= 255; brightness++) {
    DmxSimple.write(dmxChannel, brightness); // Set DMX channel 1 to new value
    delay(10); // Wait 10ms
  }
}
