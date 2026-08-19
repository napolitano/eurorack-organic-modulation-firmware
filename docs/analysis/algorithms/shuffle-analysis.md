# Shuffle algorithm engineering analysis

## 1. Purpose and scope

Shuffle is the implemented Electronica-bank mode that turns a straight internal subdivision grid into a deterministic long-short timing pattern. It produces a short CV decay at each onset so the timing deformation is directly patchable.

The mode was initially called Groove. **Shuffle** is retained because it specifies the intended behavior precisely and leaves random timing/velocity humanization to a separate Percussion-bank concept. There is no Quinn Freedman Shuffle mode to preserve.

## 2. Mathematical foundations

Consider a pair of nominally equal subdivisions with normalized pair duration 1. Let Texture map to swing ratio

$$
r(\tau)=\frac12+\frac14\tau,
$$

so

$$
\frac12\le r\le\frac34.
$$

The two onset positions are

$$
t_0=0,
$$

$$
t_1=r.
$$

The next pair begins at 1. The interval lengths are therefore

$$
\Delta_1=r,
$$

$$
\Delta_2=1-r,
$$

with invariant total

$$
\Delta_1+\Delta_2=1.
$$

At Texture zero, both intervals are exactly 1/2. At maximum Texture they are 3/4 and 1/4, a 3:1 ratio.

Each onset launches the same fixed decay contour

$$
D(u)=1-S(u),
$$

where $u\in[0,1]$ is local envelope phase and $S(u)=3u^2-2u^3$. The implementation fixes envelope duration to one eighth of the complete pair. At maximum shuffle the shorter interval is one quarter of the pair, so this decay remains safely shorter and cannot overlap an adjacent onset merely because Texture is high.

## 3. Reference algorithm

At each processing sample:

1. map Speed to Electronica tempo;
2. advance a phase over a pair of sixteenth-note subdivisions;
3. map Texture to the second-onset location $r$;
4. detect crossing of phase 0 or $r$ while preserving overshoot;
5. launch/restart the fixed decay envelope at each onset;
6. output its 12-bit value;
7. keep both onset amplitudes identical.

No randomization and no velocity alternation are part of the first contract.

## 4. Relationship to prior art and upstream Drift

Ableton's official Groove documentation describes groove as modification of timing/feel and explicitly separates Timing, Random and Velocity controls: <https://www.ableton.com/en/manual/using-grooves/>. This supports the design decision to make Shuffle a **timing-only deterministic transform** rather than a combined humanizer.

Roland's current TB-303 software instrument also exposes Shuffle as a sequencer parameter, illustrating its relevance to acid/electronic sequencing vocabulary: <https://www.roland.com/global/products/rc_tb-303/>.

There is no upstream Drift implementation.

## 5. Behavioral analysis

Shuffle preserves macro tempo because each long-short pair has constant total duration. It therefore cannot accumulate timing drift merely because Texture changes the ratio inside the pair.

At Texture zero, the output is a straight train of equal-spaced decay contours. Increasing Texture delays every second onset, producing the perceptual long-short feel.

Live Texture modulation may move the second onset while the current pair is already in progress. The implementation must define this carefully to avoid duplicate or missing events. The safest contract is to latch the Texture-derived ratio at pair start and use it for the complete pair.

## 6. Findings and classification

- **Mathematical requirement:** pair duration remains constant for every Texture value.
- **Implementation requirement:** swing ratio should be latched at pair boundaries.
- **Musical design choice:** maximum ratio is 3:1; this is intentionally stronger than mild swing and needs audition.
- **Separation-of-concerns requirement:** no random humanization and no velocity randomization.
- **Hardware limitation:** the internal shuffle grid is free-running and cannot be promised to lock to an external clock.

## 7. Improvement strategy

The implementation keeps identical pulse amplitudes and varies only timing. If later listening suggests a fixed offbeat accent improves usefulness, that must be documented as a second musical dimension rather than slipped into the initial mode.

A future externally clockable hardware revision could make Shuffle substantially more useful without changing its mathematical core.

## 8. Computational cost on ATmega328P

State is minimal:

- one pair-phase accumulator;
- one latched ratio threshold;
- one small decay-envelope state.

The potentially tricky part is event crossing when the phase increment skips over the second-onset threshold at very high tempo. The algorithm must use crossing tests, not equality tests.

## 9. Optimization opportunities

- Convert Texture to a Q0.16 threshold once per pair.
- Use threshold crossing without division.
- Share the fixed smooth decay primitive with other Electronica algorithms if the behavior remains identical.
- Reset the envelope using phase state rather than allocating event objects.

## 10. Verification and test strategy

Required tests:

- Texture zero yields exactly equal intervals;
- maximum Texture yields 3:1 intervals;
- for every Texture, interval lengths sum exactly to one pair in fixed-point representation;
- Texture mapping is monotone;
- ratio is latched for a pair and does not change mid-pair;
- exactly two onsets occur per pair;
- no onset is lost when one processing increment crosses the threshold;
- no duplicate onset occurs at pair wrap;
- onset amplitude is identical for first and second event;
- output remains 0..4095;
- no RNG state is consumed;
- long-run pair boundaries do not drift due to discarded overshoot;
- timing probe remains under deadline.

## 11. Musical assessment

**Musical value: very high.**

Shuffle is valuable because it adds rhythmic feel without adding randomness. Suitable destinations include:

- filter cutoff for shuffled stabs;
- VCA CV for rhythmic chopping;
- percussion decay or timbre parameters;
- delay feedback/send for alternating movement;
- oscillator/wavetable shape for shuffled bass patterns.

The mode should be especially useful in house, acid and broken-techno patches where a straight grid sounds too rigid but random humanization would weaken the machine-like character.

The main limitation is synchronization: free-running swing only locks to the rest of a patch by manual tempo alignment on current hardware.

## 12. Engineering assessment

Shuffle is mathematically simple but has one non-trivial state edge case: moving thresholds during an active pair. Latching Texture at the pair boundary makes the algorithm deterministic, prevents double/missed events and gives the control a clean musical meaning. With that rule, implementation risk is low.
<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
