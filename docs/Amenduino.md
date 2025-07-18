# Introduction

Amenduino plays an ever-changing percussion track. It requires no
external hardware other than an audio amplifier and speaker. Amenduino
is an interesting introduction to the world of algorithmically generated
music.

# Compatibility

This project only works on the Arduino Duemilanove 328 and equivalents.
168 based Arduinos are not supported due to lack of Flash space. The
Mega is not supported yet.

# Download

Coming soon\...

# Schematic

The circuit is simple. Connect an audio amplifier to Digital pin 3 of
the Arduino. Connect ground on the amplifier to GND on the Arduino.

Optionally, LEDs can be connected to pins 6, 8, 10 and 12 for a simple
light show. Pins 7, 9, 11 and 13 are grounded, so LEDs can be slotted
with anode to pin 6, cathode to pin 7, and similarly 8/9, 10/11 and
12/13.

# Background

The [Amen break](https://en.wikipedia.org/wiki/Amen_break) is a short
sample of a 1969 B-side, \"Amen, Brother\" by the Winstons. Due to its
clear recording and consistent timing, it was regularly sampled in the
80\'s as a percussion loop in contemporary music - notably Hip-hop,
jungle and drum & bass. As sampling methods improved, the sample was cut
and spliced to generate new rhythms - but the sample remains in use to
this day.

This project repurposes the Arduino to cut and splice Amen into an
infinite stream of customisable ever-changing percussion. The code is
tweakable, so you can develop your own cut and splicing algorithms.

# Tweaking guide {#tweaking_guide}

## amenStart\[\]

At the start of the code is a huge array of random data, labelled
\'amenStart\[\]\'. This is the compressed audio sample. Leave that
alone - you won\'t need to touch anything there. Scroll right past.

## scaleTable\[\]\[\] and dacDelta\[\] {#scaletable_and_dacdelta}

These are data tables that are part of the ADPCM decoder. Don\'t touch.

## lightLed()

This controls lights on pins 6, 8, 10 and 12. LEDs on 6,8 and 10 form a
simple VU meter. The LED on pin 12 flashes at the start of the pattern,
when the cymbal ride is playing.

## swapSlices()

This swaps two slices in the playback list.

## adpcmDecodeBlock()

This code swaps the beat slices algorithmically, then decodes a 128
sample block of music. This is a good place to tweak beat slicing
systems.

## SIGNAL(TIMER2_OVF_vect)

This implements the resampling. This code is run 62500 times a second,
so if you are modifying here, keep things simple and fast.

## setup()

This sets up the PWM, beat slice table and LED pins. It then decodes 2
sample blocks, filling the buffer and starts the playback process.

## loop()

The loop routine watches the playback pointer. When it completes playing
a block, it runs the decompression code to have the next block ready for
playback. This is a good place to put tweak code.

# Overview of some techniques {#overview_of_some_techniques}

## Sound reproduction {#sound_reproduction}

Timer 2 on the ATmega328 processor is responsible for generating the PWM
on pin 3. Amenduino reprograms the timer to run the PWM much faster. It
runs at 62.5kHz. This is well above the range of human hearing, so there
is no audible whistling. Timer 2 also generates an interrupt 62500 times
a second, which runs an interrupt routine. This makes sure the sample is
updated in good time.

## Altering sample rate {#altering_sample_rate}

The interrupt routine adds a number to an 8 bit variable. That variable
can only store numbers between 0 and 255. Eventually it will cycle over
from 255 back to 0 again. When this happens, a new sample from the
buffer is stored in the PWM output register. This technique allows the
sample rate to be adjusted easily by changing the increment number.

## Double buffering {#double_buffering}

The next 256 samples are calculated ahead of time. Samples are
decompressed in blocks of 128. The decompression code makes sure that it
only decompresses the second block when the first block is playing, and
vice versa. That way it is impossible to hear the buffer while
decompression is happening.

## ADPCM decompression {#adpcm_decompression}

The Amen break is a long sample, and a recording of it is too large to
fit into the Arduino 328. [ADPCM](https://en.wikipedia.org/wiki/ADPCM) is
a simple audio compression technique that gives better audio quality for
a small number of bytes. Adaptive Delta Pulse Code Modulation works by
storing the difference between samples (delta) rather than the sample
itself. That way quantisation errors are less noticeable. Two bits are
stored per sample, so this gives a 4:1 compression for an 8 bit sample.
Each 128 sample block also has a global volume level, which is set at
compression time to give the best fidelity.

## The sample {#the_sample}

The sample was resampled to 98304 samples (128 samples \* 24 blocks per
beat \* 32 beats in the pattern) using
[SoX](https://sox.sourceforge.net). Each block was then compressed with
the ADPCM algorithm at different volume levels. The error rate was
measured, and the best match block was used.
