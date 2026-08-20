# Growl algorithm engineering analysis

## 1. Purpose and scope

> **Implementation status — released in 0.3.0:** Implemented under `domain/dubstep/` as a deterministic CV gesture. The Q0.12 component weights are normalized when Texture changes, sum exactly to unity and avoid per-sample division. Release 0.3.0 retains the public name **Growl**; the implemented mathematical contour is the behavioral contract.


Growl is the second mode of the released Dubstep/Bass bank. Its purpose is to generate a short, beat-synchronised **multi-lobed timbral-motion CV** suitable for destinations such as wavetable position, formant/filter position, FM amount, wavefolding or distortion control.

The name was provisional during analysis; release 0.3.0 retains **Growl** with an explicit semantic limitation. Drift cannot synthesize audio and therefore cannot create a growl bass by itself. If the CV shape does not reliably produce growl-like articulation when patched to suitable voices, a less presumptive name such as **Formant**, **Talk** or **Snarl** would be more accurate.

There is no Quinn Freedman Growl mode to preserve.

## 2. Production basis and semantic limitation

Modern growl basses commonly derive their character from moving timbral parameters rather than from one unique oscillator waveform. MusicRadar describes growling bass via time-varying FM and envelopes/LFOs that introduce harmonic movement:

<https://www.musicradar.com/tuition/tech/how-to-make-a-growling-bass-sound-using-fm-synthesis-606919>

A Serum sound-design example from Bonedo uses LFO modulation of wavetable position and formant-filter parameters to create a talking/growling movement:

<https://www.bonedo.de/artikel/sounddesign-mit-serum-wavetable-in-der-praxis/>

These references support the idea of a structured modulation source. They do not imply that one scalar CV mathematically represents “a growl”. The Drift mode is therefore a **control gesture**, not an audio model.

## 3. Mathematical foundations

Define the unipolar triangle primitive

$$
T(x)=1-\left|2\,\mathrm{frac}(x)-1\right|.
$$

For phase $\phi\in[0,1)$ and normalized Texture $\tau\in[0,1]$, define component weights

$$
a(\tau)=\frac34\tau,
$$

$$
b(\tau)=\frac12\tau^2.
$$

The released ideal contour is

$$
G(\phi,\tau)=
\frac{
T(\phi)+a(\tau)T(2\phi+\tfrac14)+b(\tau)T(3\phi+\tfrac18)
}{1+a(\tau)+b(\tau)}.
$$

Every triangle component lies in $[0,1]$. All weights are non-negative. Therefore the weighted average is bounded by construction:

$$
0\le G(\phi,\tau)\le1.
$$

This avoids output clipping. In the real Q0.12 implementation, the second and third normalized component weights are rounded independently and the remaining unity residual is assigned to the fundamental. The second contribution is monotone; the third can move backward by at most one Q0.12 LSB at an isolated Texture code because the normalization denominator is quantized. Tests explicitly permit only that one-LSB effect.

At Texture zero,

$$
G(\phi,0)=T(\phi).
$$

At higher Texture the second and third components create increasingly complex within-cycle motion.

## 4. Tempo and phase contract

The released algorithm runs one complete Growl gesture per **half note**:

$$
f_g=\frac12 f_q.
$$

At the released bank's 140 BPM centre, one gesture therefore lasts approximately 857 ms. This leaves enough time for multiple internal lobes to be heard as articulation rather than as high-speed buzz.

The phase is transport-relative. Internal mode advances at half the quarter-note phase increment. External acquisition resets the gesture to phase zero; subsequent accepted quarter boundaries snap it alternately to phase 1/2 and phase 0, correcting accumulated sub-quarter quantisation while preserving a half-note gesture period.

A future listening test may still compare half-note and quarter-note base periods, but the half-note period is frozen behavior for 0.3.0.

## 5. Texture mapping

Texture remains continuous for Growl rather than stepped. Saturated 10-bit Texture code maps to a fixed-point approximation of

$$
\tau=\frac{T}{1023}.
$$

The quadratic $b(\tau)$ term deliberately grows more slowly than the second component. This prevents the three-times component from dominating at moderate Texture values.

Texture CV therefore acts as a continuous “complexity/aggression” control.

## 6. Reference algorithm

At each processing sample:

1. obtain the shared bank quarter duration;
2. advance Growl phase at one cycle per half note;
3. map Texture to $a$ and $b$ in fixed point;
4. evaluate the fundamental triangle at $\phi$;
5. evaluate the second triangle at $2\phi+1/4$;
6. evaluate the third triangle at $3\phi+1/8$;
7. form the normalized weighted average;
8. scale to 0..4095.

No RNG is required.

## 7. Why this is not just another LFO

The released mode intentionally does **not** expose waveform choice or arbitrary frequency. Its defining contract is:

- tempo-relative gesture duration;
- fixed phase relationships between one-, two- and three-lobe components;
- one Texture macro that increases higher-order motion;
- deterministic recurrence.

Classic LFO remains the general periodic source. Growl is a single purpose-built compound modulation gesture.

If later listening shows that this distinction is not perceptually strong enough, replacing or renaming Growl would be a future product decision rather than an undocumented 0.3.0 change.

## 8. Relationship to actual growl synthesis

A growl-like result requires the destination patch to convert CV motion into timbral motion. Useful destinations include:

- wavetable position;
- formant-filter frequency/position;
- FM index;
- oscillator wave morph;
- resonant filter cutoff;
- wavefolder depth;
- distortion/waveshaper amount.

Patch results will vary dramatically. A plain VCA may only produce an unusual tremolo. User documentation must therefore avoid claims such as “generates a growl bass”.

## 9. Computational cost on ATmega328P

The algorithm is still modest:

- one phase add;
- three triangle evaluations;
- Texture weight calculations;
- three multiply-accumulate terms;
- one normalization.

The released implementation recalculates normalized Q0.12 component weights only when the saturated Texture code changes. The integer divisions needed for normalization therefore sit in `updateTexture()`, not in the steady-state sample hot path. Per-sample rendering is three triangle evaluations plus three multiply-accumulate terms and one Q-format downshift; no cached reciprocal is used.

The quadratic $\tau^2$ term can be evaluated in Q-format with one multiply when Texture is updated.

## 10. Optimization opportunities

- Keep the current cache of fully normalized Q0.12 weights and avoid recomputing them when Texture is unchanged.
- Derive $2\phi$ and $3\phi$ with adds/shifts rather than general multiplication.
- Implement the phase offsets as unsigned fixed-point constants.
- Reuse one branch-light triangle primitive.
- Avoid sine tables and floating point entirely.

## 11. Verification and test strategy

Required tests:

- dense sweep proves output is always 0..4095;
- Texture zero exactly reduces to the fundamental triangle;
- normalized second-component weight is monotone over all 1024 Texture codes and the third component never reverses by more than one Q0.12 LSB;
- the unnormalized third component follows the documented quadratic Texture relation before fixed-point normalization;
- fixed-point contour matches a double-precision reference within an explicit code tolerance;
- phase wrap is continuous to the expected triangle quantisation;
- second and third component phase offsets are exact;
- period is exactly one half note relative to the selected quarter clock;
- no RNG state is consumed;
- identical phase/Texture sequences are bit-for-bit deterministic;
- output never relies on post-hoc clipping for correctness;
- external lock/re-lock produces deterministic gesture phase;
- AVR timing remains below the 400 microsecond processing deadline.

The unit suite already verifies the exact Texture-zero reduction, unity-sum weights over all 1024 Texture codes, bounded dense phase/Texture output, a real high-Texture shape change, deterministic runtime and external re-lock origin. Floating-reference golden vectors at representative Texture values remain a useful future precision test.

## 12. Musical assessment

**Musical value: potentially high, design confidence: medium.**

The mode could be unusually useful because one CV can create articulated movement without requiring a multi-segment envelope module. It should work particularly well with digital oscillators and complex filters.

Its weakness is not computation but semantics. “Growl” in contemporary production usually describes the resulting **audio timbre**, which depends on synthesis and processing beyond Drift's control. A weak destination patch may not sound remotely growl-like.

This algorithm should therefore receive an explicit A/B listening test against a simple tempo-synchronised LFO. If users cannot reliably perceive additional value, the slot should be reconsidered.

## 13. Engineering assessment

The weighted-triangle model is bounded, deterministic, cheap and frozen as the 0.3.0 behavior. Hardware listening still matters for the **product question** of whether the name Growl communicates the result well across different destination patches; it no longer blocks the engineering contract.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
