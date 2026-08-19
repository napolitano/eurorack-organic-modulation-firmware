# Drift algorithm engineering analyses

These documents analyze each supported Drift algorithm independently. The four **Classic** documents begin with the mathematical model and then work through Quinn Freedman's Rust implementation, numerical behavior, computational cost, improvement options, and verification strategy. The optional **Organic** documents use the same engineering structure but describe project-defined algorithms with no upstream implementation to preserve.

> [!IMPORTANT]
> The analyses are engineering notes, not criticism of the upstream project. A finding is classified according to the available evidence. Unusual behavior is not automatically a defect. Where analysis establishes a defect and a stronger mathematical contract can be stated, the unreleased firmware may correct it directly; the intentional difference is then documented and regression-tested.

## Documents

- [Perlin noise](perlin-noise-analysis.md)
- [Brownian / random walk](brownian-motion-analysis.md)
- [Bézier random segments](bezier-random-walk-analysis.md)
- [LFO](lfo-analysis.md)

### Optional Organic bank

- [Fractal](fractal-analysis.md)
- [Vector](vector-analysis.md)
- [Rain](rain-analysis.md)
- [Attractor / Hénon map](attractor-analysis.md)
- [Organic bank architecture and control contract](../algorithm-banks/organic-bank-design.md)

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

## Mathematical notation

Equations in these analyses use GitHub's native MathJax rendering: inline expressions use `$...$`, while standalone equations use `$$...$$`. This keeps the mathematical notation readable in the Markdown source and rendered directly on GitHub without rasterized formula images.

## Shared implementation context

Three Classic modes — Perlin, Bézier, and LFO — use the common upstream `shared.rs::get_delta_t()` V/oct phase-increment mapping. Brownian intentionally does not. The Organic Fractal, Vector and Attractor modes reuse the corrected C++ equivalent of that Speed mapping; Rain intentionally uses direct Speed-to-decay control instead. Findings in shared paths are repeated only where they materially affect the algorithm being analyzed.

The authoritative upstream references remain:

- <https://github.com/QuinnFreedman/modular/tree/main/modules/Drift/Firmware>
- <https://github.com/QuinnFreedman/modular/blob/main/fm-lib/src/rng.rs>
- <https://quinnfreedman.github.io/fm-artifacts/Drift/drift_manual.pdf>

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
