## Summary

Describe what this pull request changes and why.

## Type of change

- [ ] Bug fix / numerical correction
- [ ] New or changed firmware behavior
- [ ] Algorithm or optimization change
- [ ] Test / CI / release engineering
- [ ] Documentation with material user or developer impact
- [ ] Refactoring with no intended behavior change

## Validation

- [ ] `pio test -e native` passes
- [ ] `pio test -e native_sanitized` passes
- [ ] `pio run -e nanoatmega328new` passes
- [ ] `pio run -e nanoatmega328` passes
- [ ] `pio test -e native_organic` passes
- [ ] `pio test -e native_organic_sanitized` passes
- [ ] `pio run -e nanoatmega328new_organic` passes
- [ ] `pio run -e nanoatmega328_organic` passes
- [ ] New or changed mathematical behavior has dedicated reference/property tests
- [ ] Intentional differences from upstream behavior have regression coverage
- [ ] Relevant timing or hardware behavior has been measured, or the remaining qualification requirement is stated below

## Hardware compatibility

- [ ] No unintended PCB, component, pin-assignment or wiring requirement is introduced.
- [ ] The change remains within the Arduino Nano R3 / ATmega328P flash, SRAM and real-time constraints.

## Documentation and changelog

- [ ] Algorithm/developer documentation is updated where the mathematical or behavioral contract changed.
- [ ] `test/requirements-traceability.json` is updated when an acceptance criterion changed or was added.
- [ ] `CHANGELOG.md` contains a release-relevant `Unreleased` entry when appropriate.
- [ ] No versioned release section or release version was invented as part of ordinary development.

## Additional verification notes

List measurements, CI details, known limitations, vectors, screenshots or other evidence that helps review the change. Use `N/A` when there is nothing to add.
