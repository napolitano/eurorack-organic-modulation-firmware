# Euclid algorithm engineering analysis

## 1. Purpose and scope

Euclid is the proposed first Percussion-bank mode. It generates a deterministic 16-step onset pattern whose hits are distributed as evenly as possible, then adds a controlled phrase-ending fill after 4, 8, 12 or 16 internally generated bars.

The mode is intended to cover sparse kick/snare-like figures through dense hi-hat/percussion patterns without becoming a general-purpose step sequencer. There is no Quinn Freedman Euclid mode to preserve.

## 2. Mathematical foundations

A Euclidean rhythm is written as

$$
E(k,n),
$$

where $n$ is the number of equal time slots and $k$ is the number of onsets. Godfried Toussaint's 2005 paper relates Bjorklund's even-distribution sequence algorithm to the structure of the Euclidean algorithm and describes the resulting onset patterns as Euclidean rhythms: <https://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf>.

Drift fixes

$$
n=16
$$

and maps the saturated 10-bit Texture value $T$ to

$$
k(T)=2+\left\lfloor\frac{12T}{1024}\right\rfloor.
$$

Because $0\le T\le1023$,

$$
2\le k\le13.
$$

The $k$ onsets are distributed around the 16-step cycle as evenly as possible. Canonical masks are rotated so step 0 is an onset, removing otherwise arbitrary cyclic rotations from the firmware contract.

The phrase length is

$$
N(T)\in\{16,12,8,4\}\ \text{bars}
$$

according to the shared Percussion quartile mapping. Fill level is

$$
F(T)=\left\lfloor\frac{5T}{1024}\right\rfloor.
$$

On the final phrase bar, an additive tail mask is applied:

$$
M_{out}=M_{euclid}\lor M_{fill}(F).
$$

The fill masks add only final-quarter steps and never remove Euclidean onsets.

## 3. Reference algorithm

1. combine Speed knob and CV through the Percussion 30..240 BPM mapping;
2. advance the sixteenth-note phase while preserving overshoot;
3. on a new bar, latch Texture for the bar's Euclidean density and fill strength;
4. at phrase start, latch phrase length from Texture;
5. choose the pre-generated canonical 16-bit mask for $k(T)$;
6. on the final phrase bar, OR the corresponding additive fill-tail mask into the base pattern;
7. on each sixteenth boundary, test the current mask bit;
8. if set, emit a 4095 DAC pulse for exactly 25 processing samples;
9. otherwise keep the output at zero.

The Bjorklund/Euclidean construction is performed offline when reference masks are generated. The AVR hot path only performs table access, counters and bit tests.

## 4. Relationship to prior art and upstream Drift

Toussaint describes rhythms $E(k,n)$ as binary onset patterns produced by Bjorklund's algorithm, with attacks distributed as evenly as possible over the cycle. The same paper gives $E(4,16)$ as the obvious evenly spaced four-onset pattern: <https://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf>.

Drift's fixed 16-step range, Texture-to-$k$ mapping, canonical rotation, multi-bar phrase engine and additive tail-fill transformation are project-defined. They are not taken from a commercial Eurorack implementation.

There is no corresponding mode in upstream Drift.

## 5. Behavioral analysis

At minimum Texture, $E(2,16)$ produces a sparse evenly spaced pulse pattern and no fill. As Texture rises, $k$ increases monotonically and therefore the bar becomes denser.

Texture also shortens phrase length in four large regions:

- lowest region: 16-bar phrase;
- next region: 12-bar phrase;
- next region: 8-bar phrase;
- highest region: 4-bar phrase.

This makes Texture a performance macro from **stable/sparse/long-form** to **busy/dense/fill-active**.

The fill remains recognizably related to the base rhythm because it only adds activity to the final four sixteenth positions. It never substitutes a completely unrelated fill pattern.

Because the mask is deterministic inside a bar, Euclid is suitable for kick-like or percussion ostinatos where stable recurrence matters more than continuous random variation.

## 6. Findings and classification

- **Mathematical requirement:** exactly $k$ base onsets must be present before fill augmentation.
- **Canonicalization requirement:** cyclic rotation must be fixed so golden vectors are unambiguous.
- **Musical design choice:** $k$ is restricted to 2..13 rather than 0..16.
- **Performance requirement:** Euclidean masks should be generated offline and stored in flash.
- **Phrase requirement:** fill only adds hits; a phrase boundary must never remove or relocate base onsets.
- **Hardware limitation:** internal bar/phrase count cannot be externally reset or synchronized.

## 7. Improvement strategy

The first implementation should keep rotation fixed. Adding rotation as another Texture dimension would make the single macro harder to understand and would create discontinuous pattern changes under CV.

Future hardware with a dedicated trigger/clock input could make Euclid dramatically stronger by supporting external clock/reset while preserving the same $E(k,16)$ and fill contracts.

If listening tests show that 2..13 hits is too narrow, the range can be revised before implementation, but the exact mapping must then be frozen in the analysis and golden vectors.

## 8. Computational cost on ATmega328P

Per processing sample, Euclid needs only the common tempo/phase update and pulse countdown. On a sixteenth boundary it performs:

- one step increment;
- one 16-bit mask bit test;
- optionally one pulse start.

Per bar it updates one flash-table pointer/mask and, on fill bars, one mask OR.

No runtime division, heap allocation or RNG is required.

## 9. Optimization opportunities

- Generate all twelve $E(k,16)$ masks offline and store them as `uint16_t` in PROGMEM.
- Store the five fill masks as constants.
- Use a 4-bit step counter with compare/reset rather than modulus 16.
- Latch Texture only on bar boundaries.
- Keep the pulse width as a fixed 25-sample countdown.

## 10. Verification and test strategy

Required tests:

- every canonical mask contains exactly its requested $k$ onsets;
- for each $k$, cyclic inter-onset gaps differ by at most one step wherever mathematically possible for even distribution;
- $E(4,16)$ is rotationally equivalent to `x...x...x...x...` and canonicalization puts an onset at step 0;
- Texture endpoints map exactly to $k=2$ and $k=13$;
- $k(T)$ is monotone non-decreasing over all 1024 Texture codes;
- fill levels map exactly to 0..4;
- every fill mask is a superset of the base mask;
- fill level 4 guarantees steps 12..15 are active;
- fill is applied only on the final phrase bar;
- phrase lengths are exactly 16/12/8/4 bars and are latched at phrase start;
- every emitted pulse lasts exactly 25 scheduler samples;
- output always remains in 0..4095;
- no RNG state is consumed;
- long-run phase counting preserves bar/phrase alignment without discarded overshoot.

Reference-mask generation should have its own deterministic test so changing the generator cannot silently alter musical patterns.

## 11. Musical assessment

**Musical value: very high.**

Euclidean rhythms are especially effective when one output needs to sound structured immediately without requiring a hand-authored sequence. On Drift, the fixed 16-step bar makes the result easy to place in house, techno, electro and generative patches.

Strong uses include:

- sparse kick or low-percussion ostinatos at low Texture;
- snare/rim/percussion figures at medium density;
- hi-hat and metallic-percussion patterns at high density;
- clock-like modulation for envelopes, switches or sample selection;
- self-running phrases where the deterministic final-bar fill provides arrangement-scale punctuation.

The phrase engine is the main musical enhancement over a plain $E(k,16)$ generator. A rhythm can remain stable long enough to establish itself, then become denser at a predictable phrase boundary before returning to its base pattern.

The primary limitation is control economy: density, fill intensity and phrase frequency are intentionally coupled to one Texture macro. The current hardware cannot expose independent $k$, rotation and phrase controls.

## 12. Engineering assessment

Euclid is low-risk computationally and moderate-risk musically. The mathematics and implementation can be made exact with pre-generated masks; the important validation is whether the coupled Texture macro gives a sufficiently useful progression from sparse/long phrases to dense/frequent fills.

The algorithm is an excellent first Percussion implementation target because its expected output is deterministic and therefore supports exhaustive golden-vector testing before any stochastic modes are added.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
