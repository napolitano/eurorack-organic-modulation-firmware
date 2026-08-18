# Firmware layers

`lib/fmd` is portable C++17 and contains no Arduino/AVR APIs. `src/platform/nano_atmega328p` owns ADC interrupts, SPI, MCP4922 chip-select timing, LED PWM, seed acquisition, timer configuration and PROGMEM table storage.

`DriftRuntime` is the application boundary. It advances only when the queued DAC port is ready, snapshots all four analogue controls, executes the real `DriftEngine`, derives LED brightness from the same output sample and queues one 12-bit DAC value.

The AVR DAC adapter preserves the original write/latch concept: foreground code shifts the next MCP4922 command while chip select is low; the 2.5 kHz timer ISR raises chip select to latch that queued sample and releases the next processing slot.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
