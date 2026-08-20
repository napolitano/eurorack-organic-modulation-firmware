# Code documentation conventions

The firmware follows the same source-documentation style used by the companion Quantizer project. Documentation is part of the engineering contract: names should explain intent, Doxygen comments should define public and non-obvious behaviour, and inline comments should explain *why* an implementation is unusual rather than repeat what the code already says.

## File headers

Every C++ header, implementation file and native test source begins with a Doxygen file header containing:

- `@file` with the exact filename;
- a one-line responsibility statement;
- `@author Axel Napolitano`;
- acknowledgement of Quinn Freedman's original Free Modular Drift concept and Rust firmware;
- copyright and `GPL-3.0-or-later` licence metadata;
- the SPDX licence identifier.

Generated lookup-table fragments are intentionally different: they carry generator provenance and a mathematical description instead of hand-maintained Doxygen metadata.

## API documentation

Public classes, ports and functions use Doxygen comments to document behaviour, units, fixed-point formats, valid ranges, state effects and return values. Algorithm classes additionally document constructor dependencies, `step()` semantics, latching/reset rules and persistent state members. Private helpers are documented when their contract is not obvious from the name alone.

Examples of information that belongs in Doxygen documentation include:

- whether a value is an ADC code, DAC code, phase accumulator or fixed-point quantity;
- whether a function clamps out-of-range input;
- whether a method advances algorithm state;
- whether a hardware method is safe only from foreground code or an ISR;
- ownership and lifetime expectations of referenced ports;
- the meaning of algorithm-specific state held across samples.

## Naming

Names favour domain meaning over implementation shorthand. Examples include `phaseIncrementFromControls`, `segmentSpeedOffset_`, `phaseAccumulator_`, `resultChannelIndex_` and `missedLatchCount_` rather than generic names such as `dt`, `time`, `idx` or `n` where the role is not immediately clear.

Short mathematical symbols remain appropriate only inside compact derivations where the surrounding formula makes their meaning unambiguous. Production state, hardware resources and control values use descriptive names.

## Inline comments

Inline comments are reserved for implementation decisions that are easy to misunderstand, such as:

- single-rounding integer forms used to preserve monotonicity;
- reciprocal multiplication replacing AVR 64-bit division;
- fractional residual accumulation in Brownian smoothing;
- phase remapping during live LFO Texture changes;
- ADC multiplexer look-ahead in free-running mode;
- deliberate split-phase MCP4922 SPI preparation and timer-edge latching;
- direct AVR register access inside timing-critical ISRs.

Comments should explain the invariant, hardware constraint or numerical reason, not restate the next C++ statement.

## Automated guardrail

Run:

```bash
python scripts/check_code_documentation.py
```

The check verifies standard file headers, generated-table provenance and a 140-character source-line limit. It also enforces a minimum semantic API contract for every algorithm header, requires parameter/return documentation in all bank math headers, and protects the shared `ClockSource` safety/timing contract.

A project `Doxyfile` is included for local API-documentation generation:

```bash
doxygen Doxyfile
```

Generated Doxygen output is local-only under `.doxygen/` and is not committed or packaged as a release artifact. The maintained architectural map and representation/state invariants are documented separately in [Source-code reference](source-code-reference.md).

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
