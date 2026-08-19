# Drift algorithm engineering analyses

These documents analyze each supported or proposed Drift algorithm independently. The four **Classic** documents begin with the mathematical model and then work through Quinn Freedman's Rust implementation, numerical behavior, computational cost, improvement options, and verification strategy. The **Organic**, **Generative**, **Ambient**, **Electronica** and proposed **Percussion** documents use the same engineering structure for project-defined algorithms with no upstream implementation to preserve. Ambient extends the contract into long-form continuous modulation; Electronica specifies tempo/grid-oriented CV behavior for house, acid, techno and adjacent styles; Percussion specifies event-oriented rhythm generation with multi-bar phrase structure. The explicit musical assessment remains part of every new-bank analysis because musical differentiation from the existing banks is a design requirement.

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

### Generative bank

- [Turing / evolving shift register](turing-analysis.md)
- [Markov state grammar](markov-analysis.md)
- [Motif phrase transformation](motif-analysis.md)
- [Urn reinforced preference](urn-analysis.md)
- [Generative bank architecture, duplication audit and musical contract](../algorithm-banks/generative-bank-design.md)

### Ambient bank

- [Current / quasi-periodic long-form motion](current-analysis.md)
- [Anchor / mean-reverting stochastic motion](anchor-analysis.md)
- [Breath / stochastic recurrent swell](breath-analysis.md)
- [Fog / smooth bipolar stochastic cloud](fog-analysis.md)
- [Ambient bank architecture, duplication audit and musical contract](../algorithm-banks/ambient-bank-design.md)

### Electronica bank

- [Pump / sidechain-style recovery contour](pump-analysis.md)
- [Acid / deterministic accent-and-slide contour](acid-analysis.md)
- [Shuffle / deterministic long-short timing](shuffle-analysis.md)
- [Polymeter / dual-cycle accent structure](polymeter-analysis.md)
- [Electronica bank architecture, duplication audit and musical contract](../algorithm-banks/electronica-bank-design.md)

### Percussion bank

- [Euclid / phrase-aware Euclidean rhythm](euclid-analysis.md)
- [Repeat / ratchet and phrase-fill generator](repeat-analysis.md)
- [Probability / metrically weighted stochastic rhythm](probability-analysis.md)
- [Humanize / bounded microtiming and pulse-level variation](humanize-analysis.md)
- [Percussion bank architecture, phrase engine, duplication audit and musical contract](../algorithm-banks/percussion-bank-design.md)

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
9. What is the musical value of the algorithm, where is it strongest, and what limitations or overlaps must be documented?

## Mathematical notation

Equations in these analyses use GitHub's native MathJax rendering: inline expressions use `$...$`, while standalone equations use `$$...$$`. This keeps the mathematical notation readable in the Markdown source and rendered directly on GitHub without rasterized formula images.

## Shared implementation context

Three Classic modes — Perlin, Bézier, and LFO — use the common upstream `shared.rs::get_delta_t()` V/oct phase-increment mapping. Brownian intentionally does not. The Organic Fractal, Vector and Attractor modes reuse the corrected C++ equivalent of that Speed mapping; Rain intentionally uses direct Speed-to-decay control instead. The Ambient bank reuses the common Speed mapping and then applies its documented 1/16 macro-time scale; Anchor additionally uses a generated Speed-compensation table for its bounded triangular innovation. The Electronica bank defines a bank-local nominal 30..240 BPM tempo mapping because its algorithms are explicitly grid-oriented. The proposed Percussion bank reuses that tempo range, adds a fixed 16-step bar and a 4/8/12/16-bar phrase engine for Euclid, Repeat and Probability; Humanize deliberately preserves a fixed event count. Both banks remain free-running on current hardware because Drift has no dedicated external clock/reset input. Findings in shared paths are repeated only where they materially affect the algorithm being analyzed.

The authoritative upstream references remain:

- <https://github.com/QuinnFreedman/modular/tree/main/modules/Drift/Firmware>
- <https://github.com/QuinnFreedman/modular/blob/main/fm-lib/src/rng.rs>
- <https://quinnfreedman.github.io/fm-artifacts/Drift/drift_manual.pdf>

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
