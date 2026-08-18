# Drift algorithm engineering analyses

These documents analyze each original Drift algorithm independently, beginning with the mathematical model and then working through Quinn Freedman's Rust implementation, numerical behavior, computational cost, improvement options, and verification strategy.

> [!IMPORTANT]
> The analyses are engineering notes, not criticism of the upstream project. A finding is classified according to the available evidence. Unusual behavior is not automatically a defect. Where analysis establishes a defect and a stronger mathematical contract can be stated, the unreleased firmware may correct it directly; the intentional difference is then documented and regression-tested.

## Documents

- [Perlin noise](perlin-noise-analysis.md)
- [Brownian / random walk](brownian-motion-analysis.md)
- [Bézier random segments](bezier-random-walk-analysis.md)
- [LFO](lfo-analysis.md)

## Common analysis contract

Each document addresses the same questions:

1. What mathematical process is the mode based on?
2. What does the upstream firmware actually implement?
3. Which behavior is intentional instrument character?
4. Which behavior is a numerical limitation, documentation discrepancy, performance concern, probable defect, or verified defect?
5. What compatibility constraints follow from the original implementation?
6. What improvement strategies are technically defensible?
7. What is the source-level computational hot path on ATmega328P?
8. Which host tests, golden vectors, statistical tests, and hardware timing measurements are required to prove correctness?

## Shared implementation context

Three modes — Perlin, Bézier, and LFO — use the common `shared.rs::get_delta_t()` V/oct phase-increment mapping. Brownian intentionally does not. Findings in the shared path are repeated only where they materially affect the algorithm being analyzed.

The authoritative upstream references remain:

- <https://github.com/QuinnFreedman/modular/tree/main/modules/Drift/Firmware>
- <https://github.com/QuinnFreedman/modular/blob/main/fm-lib/src/rng.rs>
- <https://quinnfreedman.github.io/fm-artifacts/Drift/drift_manual.pdf>

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
