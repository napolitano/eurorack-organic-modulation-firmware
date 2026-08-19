# Repeat algorithm engineering analysis

## 1. Purpose and scope

Repeat is the proposed second Percussion-bank mode. It generates guaranteed quarter-note anchor events and allows those anchors to sprout short repeated clusters: doubles, triples and four-pulse ratchets. The same mechanism is intensified on the final bar of a 4/8/12/16-bar phrase to create legible rolls and fills.

The design intentionally separates **event multiplication** from **event probability**. An anchor never disappears because of Repeat's randomness; randomness only decides whether additional pulses are inserted after it. There is no Quinn Freedman Repeat mode to preserve.

## 2. Mathematical foundations

Let normalized Texture be $\tau\in[0,1]$. Every quarter-note boundary produces a primary pulse.

For a normal, non-forced anchor, a repeat cluster is enabled with Bernoulli probability

$$
p_r=\frac34\tau.
$$

Therefore even maximum Texture retains some stochastic variation rather than forcing every anchor into the same ratchet.

If no repeat is selected, pulse count is one. If selected, saturated Texture code $T\in[0,1023]$ maps to

$$
r(T)=2+\left\lfloor\frac{3T}{1024}\right\rfloor,
$$

which yields

$$
r\in\{2,3,4\}.
$$

Let quarter-note duration be $Q$ and the cluster window be

$$
W=\frac{Q}{2}.
$$

For a cluster containing $r$ pulses, pulse $j$ occurs at

$$
t_j=\frac{jW}{r},\qquad j=0,\ldots,r-1.
$$

The first pulse is therefore always exactly on the quarter-note anchor. Subsequent pulses occupy only the first half of the beat, leaving a recovery gap before the next anchor.

## 3. Phrase-fill contract

Repeat uses the shared phrase length and fill level. On the final phrase bar:

- $F=0$: normal probabilistic behavior;
- $F=1$: final quarter forced to at least two pulses;
- $F=2$: final quarter forced to at least three pulses;
- $F=3$: final quarter forced to four pulses;
- $F=4$: third and fourth quarters both forced to four pulses.

A forced fill never cancels the primary anchor; it only raises the minimum cluster size. Normal probabilistic ratchets remain possible on the other beats of the fill bar.

This creates two distinct time scales:

- intermittent repeats can appear **inside ordinary bars**;
- the phrase end produces a guaranteed stronger repeat gesture when $F>0$.

## 4. Reference algorithm

At every processing sample:

1. map Speed to the Percussion 30..240 BPM quarter-note phase;
2. advance the nominal beat/phrase counters while preserving phase overshoot;
3. run the fixed 25-sample pulse countdown;
4. on each quarter boundary, determine whether the anchor is forced by phrase-fill state;
5. if not forced, draw one Bernoulli repeat decision using threshold $p_r$;
6. determine cluster count $r$ from latched Texture when a repeat is active;
7. precompute/schedule the remaining cluster pulse offsets inside $W=Q/2$;
8. emit each scheduled pulse at full 12-bit scale for 25 samples.

Texture should be latched at the start of each bar so a CV change cannot alter the repeat count halfway through an already scheduled cluster.

## 5. Relationship to prior art and upstream Drift

Ratchets, flams, retriggers and rolls are established sequencing/percussion techniques, but this exact anchor/probability/cluster/phrase contract is project-defined. The design deliberately avoids cloning a specific sequencer's ratchet interface or preset patterns.

There is no equivalent upstream Drift mode. The inherited contracts are the scheduler, DAC path, seeded RNG and combined Speed/Texture control handling.

## 6. Behavioral analysis

At Texture zero, Repeat becomes a simple quarter-note pulse train with no stochastic repeats and no phrase fill.

At low Texture, occasional doubles appear while the underlying quarter-note skeleton remains completely stable. At medium Texture, repeat probability and depth increase. Near maximum Texture, normal bars contain frequent four-pulse clusters, but because $p_r$ tops out at 0.75 there is still room for contrast when the phrase engine forces the tail fill.

The half-beat cluster window makes the output read as a local embellishment of the anchor rather than as a new continuous clock. At maximum tempo, a four-pulse cluster still has about 31.25 ms between adjacent pulses, leaving ample separation around the fixed 10 ms trigger pulse.

## 7. Findings and classification

- **Musical requirement:** the quarter-note anchor must never be probabilistically removed.
- **Duplication requirement:** probability applies only to repeat multiplication, not to existence of the primary hit.
- **Timing requirement:** repeat positions are derived from the nominal quarter duration and must not recursively chain from the previous repeated pulse.
- **Pulse requirement:** repeated pulses must not overlap at the 240 BPM endpoint.
- **Phrase requirement:** forced fill escalation must be stronger than normal-bar repeat behavior.
- **Hardware limitation:** Repeat cannot retrigger an *external incoming* drum hit because current Drift hardware has no trigger input.

## 8. Improvement strategy

The first version should keep quarter-note anchors fixed. Allowing Texture to change both anchor rate and repeat depth would make the mode less predictable and would overlap Euclid/Probability.

A future hardware revision with a dedicated trigger input could reinterpret Repeat as a true event repeater driven by external hits. That would be a different input contract and should not be silently simulated on current hardware.

The half-beat cluster window should be listening-tested. A shorter fixed fraction could create tighter flams, while a longer one would move toward continuous subdivision patterns. Any change must preserve pulse-separation safety at 240 BPM.

## 9. Computational cost on ATmega328P

The realtime workload is small:

- common quarter-phase update;
- one pulse countdown;
- at most one Bernoulli draw per quarter anchor;
- one small integer repeat-count mapping;
- up to three future sub-event offsets scheduled per anchor.

No heap allocation or dynamic list is required. A fixed four-entry event schedule is sufficient.

General division by 3 when scheduling triples should be avoided in the per-sample path. Cluster offsets can be precomputed from the current quarter duration once per anchor, or implemented using fixed rational multiplies.

## 10. Verification and test strategy

Required tests:

- Texture zero produces exactly one pulse per quarter and consumes no repeat RNG decision beyond what the implementation contract requires;
- repeat threshold is monotone and reaches exactly $3/4$ at maximum normalized Texture representation;
- repeat-count mapping produces only 2, 3 or 4;
- a repeated cluster always contains a pulse exactly at the nominal quarter boundary;
- all subsequent cluster pulses fall inside the first half of the quarter;
- pulse offsets are strictly increasing;
- at 240 BPM, 4-pulse clusters never overlap 25-sample output pulses;
- no-repeat anchors remain exactly quarter-note aligned;
- phrase fill levels force the exact documented minimum cluster counts;
- $F=4$ forces both third and fourth quarter anchors to four pulses;
- normal stochastic repeats remain possible outside the forced tail;
- fixed seed/control sequence gives bit-for-bit repeat decisions;
- output always remains in 0..4095;
- long-run nominal beat timing has no accumulated repeat-induced drift.

Statistical tests should validate repeat-decision probability separately from the deterministic forced-fill cases.

## 11. Musical assessment

**Musical value: very high.**

Repeat gives Drift something none of the existing banks currently provide: **sub-event structure**. Rather than merely deciding where a hit occurs, it can decorate a stable pulse with a flam, short roll or ratchet.

Strong destinations include:

- closed/open hi-hats for intermittent ratchets;
- snares and claps for phrase-ending rolls;
- metallic percussion and FM drum voices;
- sample retrigger inputs;
- envelope or switch triggers where clustered events create timbral bursts.

The strongest aspect is the combination of local unpredictability and global structure. Repeats can appear during ordinary bars, while the phrase counter guarantees an intelligible stronger gesture at the end of the current 4/8/12/16-bar form.

The main limitation is fundamental: with no trigger input, Repeat generates its own anchor pulses instead of embellishing an external drum pattern.

## 12. Engineering assessment

Repeat is computationally low-risk but scheduler-sensitive. The core arithmetic is trivial; correctness depends on not losing, merging or delaying scheduled sub-events when pulses occur close together.

A dedicated deterministic event-scheduler test should therefore be considered as important as the algorithm's statistical tests. If that scheduler remains correct at 240 BPM with four-pulse clusters, the mode should fit comfortably within ATmega328P limits.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
