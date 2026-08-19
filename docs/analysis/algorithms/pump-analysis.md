# Pump algorithm engineering analysis

## 1. Purpose and scope

Pump is the implemented first Electronica-bank mode. It generates the familiar **duck then recover** modulation contour associated with sidechain-heavy house and techno production.

The mode does not perform dynamics processing. Drift has no audio detector or external sidechain input, so Pump is a self-running control-voltage generator only. There is no Quinn Freedman Pump mode to preserve.

## 2. Mathematical foundations

Let $p\in[0,1)$ be normalized quarter-note beat phase and $e$ the recovery endpoint. Define the cubic smoothstep

$$
S(x)=3x^2-2x^3.
$$

The normalized output is

$$
y(p;e)=S\left(\operatorname{clamp}\left(\frac{p}{e},0,1\right)\right).
$$

The required range is

$$
\frac14\le e\le\frac{15}{16}.
$$

Therefore:

- $y(0)=0$ immediately after a beat boundary;
- $y(e)=1$;
- $y(p)=1$ for all $p\ge e$;
- the recovery segment has zero slope at both endpoints.

Texture $\tau\in[0,1]$ maps linearly to recovery endpoint

$$
e(\tau)=\frac14+\frac{11}{16}\tau.
$$

The next beat intentionally resets the output discontinuously from its held high value to zero.

## 3. Reference algorithm

At every 2.5 kHz processing sample:

1. map Speed knob + CV to the Electronica nominal tempo;
2. advance a quarter-note phase accumulator while preserving overshoot;
3. map Texture to recovery endpoint $e$;
4. normalize phase inside the recovery interval;
5. if phase is beyond $e$, output full scale;
6. otherwise evaluate smoothstep and map to 0..4095;
7. on beat wrap, the phase returns to the beginning of the duck contour.

No random generator is required.

## 4. Relationship to prior art and upstream Drift

Ableton's Compressor documentation describes ducking as reducing one signal in response to another and calls sidechain/ducking a common dance-music technique for making basslines or mixes make room for a kick: <https://www.ableton.com/en/live-manual/11/live-audio-effect-reference/#sidechaining-in-dance-music>.

Pump reproduces only the resulting envelope gesture. It does not detect audio, estimate gain reduction, implement a compressor transfer curve or claim sidechain synchronization.

There is no upstream Drift implementation. The relevant inherited contracts are the 2.5 kHz scheduler, 12-bit DAC path and combined knob/CV control handling.

## 5. Behavioral analysis

Pump has one intentional discontinuity per beat. That discontinuity is not a numerical defect: it represents the instant a hypothetical kick would force gain reduction.

Texture changes how much of the beat is spent recovering:

- low Texture: fast release, subtle breathing;
- medium Texture: classic quarter-note pump;
- high Texture: long suppression and late recovery.

Because Attenuation remains analogue after the DAC, the user can independently choose modulation depth without consuming Texture for depth control.

## 6. Findings and classification

- **Musical design choice:** quarter-note reset is the defining event.
- **Mathematical requirement:** the recovery endpoint must never be zero, avoiding division by zero.
- **Implementation requirement:** phase overshoot must be preserved so long-running tempo does not accumulate sample-grid bias.
- **Documentation requirement:** user-facing text must say *sidechain-style* or *ducking-style*, not actual sidechain compression.
- **Hardware limitation:** no external clock/trigger means no promised lock to a kick module.

## 7. Improvement strategy

The implementation keeps one curve family and one Texture macro. Adding attack, hold, depth or multi-beat patterns would either consume unavailable controls or turn Pump into a preset-heavy envelope generator.

If listening tests show cubic smoothstep is too soft, a fixed integer power-curve family may be evaluated later, but that would be a musical revision and requires regression/golden-vector updates.

## 8. Computational cost on ATmega328P

Per sample:

- one tempo/phase update;
- one recovery-range comparison;
- one normalized fixed-point ratio while inside recovery;
- one cubic smoothstep evaluation;
- one 12-bit scaling operation.

The division implied by $p/e$ should not be performed as a general AVR division on every sample. The implementation caches the reciprocal of $e$ as a Q28 value whenever the saturated Texture control changes, avoiding a general phase/end-point division in the per-sample path.

## 9. Optimization opportunities

- Cache recovery endpoint and reciprocal from Texture.
- Use Q0.16 smoothstep with the already established fixed-point conventions.
- Short-circuit to 4095 after the recovery endpoint.
- Preserve phase overshoot with the shared helper rather than modulus/division.
- Avoid all RNG work.

## 10. Verification and test strategy

Required tests:

- $y(0)=0$ for every Texture value;
- output reaches exactly 4095 at/after recovery endpoint;
- recovery is monotone non-decreasing;
- cubic segment has no one-code reversal over the dense fixed-point domain;
- Texture monotonically lengthens the recovery interval;
- zero and maximum Texture map to exactly 1/4 and 15/16 beat;
- output always remains in 0..4095;
- phase overshoot is preserved across beat wrap;
- identical control sequence is bit-for-bit deterministic;
- no RNG state is consumed;
- timing probe remains below the 400 µs processing deadline.

A system test should run multiple minutes of nominal tempo and confirm there is no cumulative phase drift caused by discarding wrap overshoot.

## 11. Musical assessment

**Musical value: very high.**

Pump is probably the most immediately useful Electronica mode because a single CV can create a large amount of perceived rhythmic motion. Strong destinations include:

- VCA level for pad/bass ducking;
- filter cutoff or resonance for breathing grooves;
- reverb/delay send depth;
- wavetable or wavefolder position;
- stereo/spatial parameters on effects that accept CV.

Its greatest strength is legibility: the listener understands the pulse almost immediately.

Its main weakness is equally clear: without a clock input, Pump can only be manually tempo-matched. It should be marketed as a rhythmic modulation source, not as a replacement for a clocked sidechain envelope.

## 12. Engineering assessment

Pump is low-risk mathematically and computationally. The critical correctness points are semantic rather than numerical: preserve the intentional reset, keep recovery monotone and avoid overstating synchronization. Its simple state also makes it a strong first implementation target for the Electronica bank.
<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
