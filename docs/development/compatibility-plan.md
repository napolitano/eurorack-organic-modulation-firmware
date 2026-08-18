# Reference and correction policy

The filename is retained for existing links, but the project is no longer a defect-preserving compatibility port. The original Drift firmware is the design and behavioral reference; verified implementation defects are corrected when a stronger mathematical or engineering contract can be stated and tested.

## Stage 1 — structural baseline

- PlatformIO C++17 project;
- Quantizer-style ports-and-adapters boundary;
- four algorithm implementations;
- deterministic upstream LFSR recurrence;
- native tests, sanitizers, coverage and traceability;
- AVR Nano old/new bootloader targets.

## Stage 2 — mathematical verification and corrections

For each algorithm:

1. derive the mathematical model independently of the implementation;
2. analyze the upstream Rust code and identify intentional design choices, numerical limitations and defects;
3. define corrected contracts for confirmed problems;
4. test the production mathematical primitives against independent equations, exhaustive domains where practical, and strong invariants;
5. retain regression tests that make intentional differences from upstream explicit.

Current corrections include Brownian Texture/convergence, Bézier ICDF/curve continuity/phase wrap, LFO startup/live skew/endpoints/phase wrap, monotone fixed-point polynomial evaluation and exact-but-cheaper frequency conversion.

## Stage 3 — upstream reference-vector comparison

Generate deterministic vectors from the original Rust implementation for fixed seeds and control streams. Classify every mismatch as one of:

- accidental C++ porting defect;
- expected consequence of a documented correction;
- unresolved lookup-table/reference-data difference.

Reference vectors remain important even though bit-for-bit compatibility is not the goal for corrected defects.

## Stage 4 — hardware qualification

Measure and record:

- 2.5 kHz DAC latch cadence;
- worst-case foreground execution time for each algorithm;
- skipped-latch/deadline diagnostics;
- MCP4922 framing and LDAC timing;
- ADC snapshot behavior and input ordering;
- LED PWM behavior;
- flash and static-SRAM consumption on both Nano bootloader targets.

## Stage 5 — measured optimization

Only optimize further when the relevant mathematical contract is already protected by tests and the AVR build or hardware measurements show a worthwhile gain. Prefer transformations that are provably equivalent; document any accepted numerical approximation and its maximum error.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
