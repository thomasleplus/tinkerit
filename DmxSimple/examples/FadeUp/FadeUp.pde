#include <DmxInterrupt.h>
#include <DmxSimple.h>

void setup() {
  DmxSimple.begin(); // Set up DMX system
}
void loop() {
  int a;
  for (a=0; a<255; a++) {
    DmxSimple.write(1, a); // Set DMX channel 1 to new value
    delay(10); // Wait 10ms
  }
}
