# Humanize algorithm engineering analysis

## 1. Purpose and scope

Humanize is the proposed fourth Percussion-bank mode. It starts from a perfectly regular eighth-note pulse train and introduces bounded random microtiming and pulse-amplitude variation without ever changing the number of events or the long-term tempo.

The purpose is not to imitate a specific drummer. It is to make rigid electronic pulse streams less mechanically identical while preserving a stable underlying grid. There is no Quinn Freedman Humanize mode to preserve.

## 2. Mathematical foundations

The nominal pattern contains eight events per 16-step bar at sixteenth indices

$$
\{0,2,4,6,8,10,12,14\}.
$$

If nominal eighth-note event $n$ occurs at time $t_n^0$, the actual event time is

$$
t_n=t_n^0+\delta_n.
$$

The jitter $\delta_n$ is independently drawn from a bounded symmetric integer interval

$$
\delta_n\in[-J(T),J(T)]
$$

with

$$
J(T)=\left\lfloor\frac{30T}{1023}\right\rfloor\ \text{samples}.
$$

At Drift's 2.5 kHz processing rate, one sample is 0.4 ms and the maximum absolute timing displacement is therefore

$$
30\times0.4\ \text{ms}=12\ \text{ms}.
$$

Pulse amplitude uses nominal DAC code

$$
A_0=3840.
$$

Texture controls amplitude-deviation radius

$$
V(T)=\left\lfloor\frac{255T}{1023}\right\rfloor.
$$

With independent signed amplitude deviation $\epsilon_n\in[-V,V]$,

$$
A_n=\mathrm{clamp}(A_0+\epsilon_n,0,4095).
$$

At maximum Texture this produces codes 3585..4095, approximately 8.75..10 V on the nominal 0..10 V DAC scale before analogue attenuation.

## 3. No-drift timing invariant

The essential correctness condition is that jitter never becomes part of the clock state. Event $n$ is always defined relative to the ideal grid position $t_n^0$.

The implementation must **not** use

$$
t_{n+1}=t_n+T_8+\delta_{n+1},
$$

because that would recursively incorporate the previous event's timing error and create a random walk in tempo.

Instead,

$$
t_n=nT_8+\delta_n
$$

relative to the independent nominal eighth-note phase/grid.

At 240 BPM the eighth-note interval is 125 ms. Even two successive events displaced toward each other by the maximum 12 ms remain 101 ms apart, so event order cannot invert.

### Percussion clock-source contract

In this bank, Speed CV is repurposed as a **0..5 V quarter-note clock input** rather than being summed with the Speed knob. Two valid rising edges acquire external timing; loss for more than 2.5 measured periods returns automatically to the Speed-knob clock. The original hardware is not specified for 10 V trigger inputs, so 10 V clocks are explicitly unsupported until the analogue input stage is revised.

## 4. Reference algorithm

1. derive the nominal grid from the Speed knob at 30..240 BPM or from the locked Speed-CV quarter-note clock;
2. maintain an independent nominal eighth-note phase/count that never contains jitter;
3. before each upcoming event, draw its bounded timing offset and amplitude deviation from the deterministic seeded RNG;
4. schedule the event at its nominal due time plus the signed offset;
5. emit the resulting amplitude for exactly 25 scheduler samples;
6. return to zero until the next scheduled eighth note.

The very first event after startup may be fixed at the nominal origin because an event cannot be emitted before the algorithm starts. All subsequent events can be scheduled early or late because their next nominal time is known in advance.

Humanize does not use the Percussion fill flag and does not alter event count at phrase boundaries.

## 5. Relationship to prior art and upstream Drift

Ableton Live's Groove Pool treats random timing fluctuation and velocity as distinct groove dimensions and explicitly describes low random timing amounts as useful for adding subtle humanization to highly quantized electronic loops: <https://www.ableton.com/en/manual/using-grooves/>.

Drift's fixed eighth-note source, exact jitter bounds, amplitude center/range and single Texture macro are project-defined. The mode does not extract or imitate a recorded performer's groove.

There is no upstream Drift implementation.

## 6. Behavioral analysis

At Texture zero, Humanize is completely deterministic: eight equally spaced eighth-note pulses per bar, each with DAC code 3840 and 10 ms duration.

As Texture rises, each event can move slightly before or after its nominal location and become slightly stronger or weaker. The underlying tempo, bar length and event count remain unchanged.

This makes Humanize particularly different from Shuffle. Shuffle applies a deterministic long-short timing relationship to every subdivision pair; Humanize applies independent bounded deviations around a straight grid.

It is also different from Probability: Humanize never adds or removes an event.

## 7. Findings and classification

- **Timing requirement:** jitter is relative to the nominal grid, never recursively accumulated.
- **Causality requirement:** the next event's offset must be generated early enough to schedule a negative displacement.
- **Ordering requirement:** maximum jitter must remain safely below half the minimum event interval.
- **Musical design choice:** source pattern is fixed to eighth notes because the hardware provides no incoming rhythm to humanize.
- **Amplitude requirement:** even minimum randomized pulse level should remain trigger-useful on typical Eurorack inputs before attenuation.
- **Phrase design choice:** Humanize intentionally ignores fill state and preserves eight events per bar.

## 8. Improvement strategy

The first implementation should not add random event omission, ratchets or fill behavior. Those belong to Probability and Repeat and would make Humanize semantically muddy.

If listening tests show ±12 ms is too strong or too subtle, the endpoint can be revised before implementation. The important invariant is bounded symmetric jitter with no cumulative drift.

A future hardware trigger input would allow Humanize to operate on external events. On current hardware, the internal eighth-note source is the only honest self-contained contract.

## 9. Computational cost on ATmega328P

Humanize performs stochastic work only eight times per bar:

- one bounded timing RNG draw;
- one bounded amplitude RNG draw;
- one future-event schedule update.

Per sample it needs the normal tempo phase, one due-event comparison and the fixed pulse countdown.

No floating-point arithmetic, heap allocation or large tables are required.

## 10. Verification and test strategy

Required deterministic tests:

- Texture zero produces exact straight eighth notes with constant amplitude 3840;
- timing-jitter radius maps exactly from 0 to 30 samples;
- amplitude radius maps exactly from 0 to 255 codes;
- every generated offset lies inside its documented bounds;
- every amplitude lies inside 3585..4095 at maximum Texture and inside 0..4095 generally;
- event count remains exactly eight per bar at every Texture;
- phrase boundaries do not add/remove/force events;
- every pulse lasts exactly 25 samples;
- fixed seed/control sequence reproduces identical timing/amplitude vectors.

Required timing/property tests:

- nominal grid phase after many bars is identical regardless of jitter sequence;
- actual event order never changes at 240 BPM with maximum jitter;
- minimum adjacent-event separation remains greater than pulse width;
- long-run mean signed jitter is statistically compatible with zero;
- long-run mean amplitude remains statistically compatible with the nominal center within a declared tolerance;
- sanitizer/timing-probe builds show no overflow or deadline regression.

## 11. Musical assessment

**Musical value: high.**

Humanize is less spectacular than Euclid or Repeat, but it solves a common problem: perfectly repeated hats, shakers and auxiliary percussion often sound rigid even when the pattern itself is good.

Strong uses include:

- closed hats and shakers;
- claves, rims and hand-percussion-like sounds;
- trigger streams feeding velocity-sensitive or accent-aware drum modules;
- modulation pulses where small amplitude differences should affect downstream envelope depth or timbre;
- layered percussion where one rigid source and one Humanize source create controlled flamming/width.

Its main strength is restraint. It can add feel without changing the pattern's identity or phrase length.

Its limitation is equally important: because current Drift has no trigger input, it humanizes its own fixed eighth-note train rather than an arbitrary external kick/snare pattern.

## 12. Engineering assessment

Humanize is computationally low-risk but has one subtle scheduler requirement: negative jitter must be scheduled causally without contaminating the nominal clock. That should be designed explicitly rather than approximated by delaying-only jitter, which would bias the mean timing and change the musical contract.

With a fixed nominal grid and a maximum ±12 ms window, the mode is straightforward to verify exhaustively for ordering and pulse-separation safety.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
