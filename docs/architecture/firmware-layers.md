# Firmware layers

`lib/fmd` is portable C++17 and contains no Arduino/AVR APIs. `src/platform/nano_atmega328p` owns ADC interrupts, SPI, MCP4922 chip-select timing, LED PWM, seed acquisition, timer configuration and PROGMEM table storage.

`DriftRuntime` is the application boundary. It advances only when the queued DAC port is ready, snapshots all four analogue controls, executes the real `DriftEngine`, derives LED brightness from the same output sample and queues one 12-bit DAC value.

The AVR DAC adapter preserves the original write/latch concept: foreground code shifts the next MCP4922 command while chip select is low; the 2.5 kHz timer ISR raises chip select to latch that queued sample and releases the next processing slot.

## Domain layout

The portable domain layer mirrors the firmware-bank model in both its public headers and its implementations:

```text
lib/fmd/include/fmd/domain/          lib/fmd/src/domain/
├── classic/                        ├── classic/
│   ├── PerlinAlgorithm.h           │   ├── PerlinAlgorithm.cpp
│   ├── BrownianAlgorithm.h         │   ├── BrownianAlgorithm.cpp
│   ├── BezierAlgorithm.h           │   ├── BezierAlgorithm.cpp
│   └── LfoAlgorithm.h              │   └── LfoAlgorithm.cpp
├── organic/                        ├── organic/
├── generative/                     ├── generative/
├── ambient/                        ├── ambient/
├── DriftEngine.h                   ├── DriftEngine.cpp
├── Types.h                         ├── FixedMath.cpp
├── FixedMath.h                     ├── FrequencyMapping.cpp
├── FrequencyMapping.h              └── ParallelLfsr.cpp
├── ParallelLfsr.h
└── AlgorithmMath.h
```

Bank-specific math helpers live with their bank (`OrganicAlgorithmMath`, `GenerativeAlgorithmMath`, `AmbientAlgorithmMath`). `AlgorithmMath` remains at the shared domain level because several later banks reuse verified phase-advance, gradient and triangular-distribution primitives that originated with the Classic implementation. This placement makes that cross-bank dependency explicit instead of forcing non-Classic code to include a Classic directory.

`DriftEngine` is the only bank-selection/dispatch boundary. Public includes therefore name the bank explicitly, for example `fmd/domain/generative/TuringAlgorithm.h`. PlatformIO discovers sources recursively through `lib/fmd/library.json`, so the subdirectory layout does not require per-bank source lists.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
