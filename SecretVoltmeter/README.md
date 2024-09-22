# Introduction

It turns out the Arduino 168 and 328 can measure their own voltage rail.

# Code

Copy, paste into Arduino and see what it returns. This works on an Arduino 168 or 328\.

```C
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println( readVcc(), DEC );
  delay(1000);
}
```

The voltage is returned in millivolts. So 5000 is 5V, 3300 is 3.3V.

Note the following:

- This works on Arduinos with a **328 or 168 only**. It looks like the same trick might be possible on the Arduino Mega - experiments are ongoing, and will be reported here.

# How it works

The Arduino 328 and 168 have a built in precision voltage reference of 1.1V. This is used sometimes for precision measurement, although for Arduino it usually makes more sense to measure against Vcc, the positive power rail.

The chip has an internal switch that selects which pin the analogue to digital converter reads. That switch has a few leftover connections, so the chip designers wired them up to useful signals. One of those signals is that 1.1V reference.

So if you measure how large the known 1.1V reference is in comparison to Vcc, you can back-calculate what Vcc is with a little algebra. That is how this works.
