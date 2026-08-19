# Motif algorithm engineering analysis

## 1. Purpose and scope

Motif is a proposed Generative-bank algorithm that stores an explicit eight-step control-voltage phrase and applies small structural transformations between repetitions.

Its purpose is not to generate an unrelated new random sequence on every cycle. The central musical contract is **identity through variation**: the phrase should remain recognisable while its ordering and, less frequently, one item of its vocabulary changes.

There is no upstream Quinn Freedman implementation and no single external algorithm being reproduced.

## 2. Mathematical representation

Let the stored phrase be

$$
M_n=(m_0,m_1,\ldots,m_7),
$$

where each $m_i$ is a 12-bit value in 0..4095.

A playhead $k$ advances modulo 8 at the Speed-defined step rate and outputs

$$
y=m_k.
$$

When the playhead completes a full phrase cycle, normalized Texture $\tau$ defines the probability of applying one structural edit:

$$
P(edit)=\tau.
$$

If no edit occurs,

$$
M_{n+1}=M_n.
$$

If an edit occurs, one of four project-defined transformations is selected with equal probability.

## 3. Transformation set

### 3.1 Circular rotation

Rotate the complete phrase by one position left or right, selected randomly:

$$
(m_0,m_1,\ldots,m_7)\rightarrow(m_1,m_2,\ldots,m_0)
$$

or the inverse direction.

This preserves every value and every adjacency except the phrase-start reference.

### 3.2 Adjacent swap

Choose one index $i$ and exchange $m_i$ with $m_{i+1}$ modulo 8. This makes the smallest possible ordering edit while preserving the vocabulary exactly.

### 3.3 Three-step reversal

Choose one start index $i$ and reverse the circular span

$$
(m_i,m_{i+1},m_{i+2}).
$$

The operation preserves the complete vocabulary but changes local contour more strongly than an adjacent swap.

### 3.4 Single-value replacement

Choose one index and replace only that value with a new pseudorandom 12-bit code. This is the sole operation that introduces new material.

Because the four operations are equiprobable, three quarters of edit events preserve all eight phrase values, and even the replacement operation preserves seven of eight.

## 4. Relationship to algorithmic composition

Rotation, reversal, permutation and substitution are common transformation families in algorithmic and rule-based composition. The exact eight-step phrase model, operation set, edit timing and probabilities here are project-defined.

For broader context on computational and rule-based composition techniques, see Michael Edwards, *Algorithmic Composition: Computational Thinking in Music*, Communications of the ACM 54(7), 2011: <https://www.pure.ed.ac.uk/ws/files/16205214/algorithmic_composition_AM.pdf>.

## 5. Reference algorithm

At every processing sample:

1. advance the common Speed phase;
2. if no phase wrap occurs, keep the current output;
3. on wrap, advance the playhead modulo 8 and output the new phrase value;
4. if the playhead wrapped from step 7 to step 0, evaluate one Texture-controlled edit decision;
5. if an edit is selected, choose exactly one of the four transformations and apply it once;
6. continue playback from the transformed phrase.

The initial phrase is generated deterministically from the algorithm seed.

## 6. Relationship to existing Drift algorithms

Motif differs from Turing even though both can produce repeating, evolving sequences.

- Turing stores a **binary shift-register state** and mutations enter one bit at a time through feedback.
- Motif stores **eight explicit 12-bit values** and makes phrase-level structural edits only at complete-cycle boundaries.

It differs from Bézier because no interpolation occurs between destinations. It differs from Markov because the complete ordering is explicitly stored rather than generated from a fixed transition rule. It differs from Urn because there are no preference weights.

## 7. Behavioral analysis

At Texture 0 the phrase repeats indefinitely and exactly. At low Texture, many complete repetitions occur between edits. At maximum Texture, there is exactly one edit after every eight-step cycle—still a gradual evolution because only one transformation is permitted per cycle.

This cap is musically important. "Maximum variation" must not mean replacing the complete phrase every cycle; the mode would then lose the identity that distinguishes it from random sequencing.

Rotation produces phase reinterpretation, adjacent swap introduces a small rhythmic/contour displacement, three-step reversal changes local direction, and replacement introduces genuinely new voltage material.

## 8. Findings and classification

- **Musical design choice:** phrase length is fixed at eight steps because the hardware offers no independent length control.
- **Musical requirement:** transformations occur only at complete phrase boundaries.
- **Musical requirement:** at most one edit occurs per phrase, even at maximum Texture.
- **Sound-design choice:** three of four edit types preserve the complete voltage vocabulary.
- **Implementation requirement:** all operations must work correctly across the circular 7→0 boundary.
- **Terminology constraint:** this is a project-defined generative motif transformer, not an implementation of a particular compositional theory.

## 9. Improvement strategy

The first implementation should avoid adding more transformation types. Transposition, inversion around a numeric axis, interpolation or phrase-length changes would introduce additional musical assumptions and may not behave predictably for arbitrary 0..4095 CV values.

If future hardware adds another user control, phrase length would be a strong candidate. On the current hardware, fixed length keeps Texture semantically focused on variation probability.

## 10. Computational cost on ATmega328P

Normal processing samples require only the shared phase update and no sequence mutation work.

Each step event performs:

- one modulo-8 playhead increment;
- one `uint16_t` phrase lookup.

Only every eighth step can perform mutation work. The heaviest operation is a three-value reversal or an eight-value rotation, both trivial compared with the 2.5 kHz sample budget.

Persistent state is eight 12-bit values stored as `uint16_t`, one playhead index, phase accumulator, RNG state and output value.

## 11. Optimization opportunities

- Use an eight-element fixed array and power-of-two index masking.
- Implement left/right rotation with index-offset metadata if measurements show copying eight elements is undesirable; do not complicate the first version prematurely.
- Perform edit probability and operation RNG draws only at phrase boundaries.
- Keep replacement generation 12-bit directly; no floating-point scaling is required.

## 12. Verification and test strategy

Required tests:

- Texture 0 leaves a known phrase bit-for-bit unchanged across arbitrary cycle counts;
- maximum Texture performs exactly one edit per completed phrase, never more;
- no edit is evaluated before step 7→0 wrap;
- circular left/right rotation produces exact expected arrays;
- adjacent swap preserves the multiset of eight values;
- three-step reversal preserves the multiset and handles wraparound spans such as 7,0,1;
- replacement changes at most one position and keeps all values in 0..4095;
- rotation/swap/reversal never create or destroy a value;
- deterministic RNG scripting selects each operation and each boundary case;
- output follows the stored phrase exactly and remains in 0..4095;
- dynamic Texture changes affect future edit decisions but do not retroactively alter the current phrase step.

Statistical tests may verify edit frequency over many cycles, but the exact transformation operations should primarily use deterministic scripted-random tests.

## 13. Musical assessment

**Musical value: very high.**

Motif is probably the most composition-like algorithm in the proposed bank. It creates an immediately audible relationship between repetition and development: a pattern can establish itself, change slightly, repeat again, and gradually become something else without losing continuity.

Strong uses include:

- external-quantizer pitch sequences;
- repeating but evolving filter or waveshaper contours;
- slow macro-parameter changes where structural recurrence is more valuable than smooth noise;
- generative ambient patches in which one phrase can survive for minutes at low Speed/Texture;
- techno or acid modulation where the module is allowed to free-run rather than lock to transport.

Its strongest advantage over Turing is semantic: changes occur at the **phrase level**, so the listener can perceive operations as variations of material rather than bit-level corruption.

Its limitation is equally intentional: at low Speed an eight-step phrase can take a very long time to complete, so mutation may be extremely slow. That is appropriate for a generative/ambient role but should be documented.

## 14. Engineering assessment

Motif has very low computational risk and a strong musical identity. The key design discipline is to resist overloading it with more transformation types. The compact four-operation set is sufficient to demonstrate structural evolution while remaining exhaustively testable on AVR.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
