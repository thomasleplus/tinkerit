# Introduction

It turns out the Arduino 328 has a built in thermometer. Not the old Mega8 or 168\. Not the Arduino Mega. Just 328 based Arduinos.

# Code

Copy, paste into Arduino and see what it returns. It only works on an Arduino with a 328 chip.

```C
long readTemp() {
  long result;
  // Read temperature sensor against 1.1V reference
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = (result - 125) * 1075;
  return result;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println( readTemp(), DEC );
  delay(1000);
}
```

Temperature is returned in milli-�C. So 25000 is 25�C.

# How it works

The chip has an internal switch that selects which pin the analogue to digital converter reads. That switch has a few leftover connections, so the chip designers wired them up to useful signals. One of those signals is that simple temperature sensor.

If you measure the sensor voltage against the internal precision 1.1V reference, you can calculate approximate temperature. It requires a bit of messing around with the registers, but it can be done. That is how this works.

# Additional notes

The sensor isn't very accurate - the data sheet says �10�C. But once you've worked out the offset and correct for it, accuracy improves.

Note the following:

- This works on Arduinos using CPU's with '8P' in the part number. For standard Arduinos, that means **328 only**.
- If you have an Arduino clone with an ATmega168P or ATmega168PA, it will work there too. It will not work with an ATmega168\. (Thanks @blalor)
- This sensor is pretty useless unless you calibrate it against a known temperature.
- The sensor outputs in approximately 1�C steps.

But hey - it's free!
