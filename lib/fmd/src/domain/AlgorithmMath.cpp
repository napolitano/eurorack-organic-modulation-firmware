#include "fmd/domain/AlgorithmMath.h"

#include "fmd/domain/FixedMath.h"

#include <stdint.h>

namespace fmd::perlinmath {
namespace {
int16_t toSigned(uint16_t x) { return static_cast<int16_t>(x >> 1U); }
}

int16_t gradientFromRandom(uint16_t random) {
  const uint16_t h = static_cast<uint16_t>(random & 15U);
  const uint16_t magnitude = static_cast<uint16_t>(1U + (h & 7U));
  const int16_t gradient = static_cast<int16_t>(magnitude << 11U);
  return (h & 8U) != 0U ? static_cast<int16_t>(-gradient) : gradient;
}

uint16_t fadeQ0F16(uint16_t phase) {
  // The upstream implementation ultimately quantises the fade weight to
  // 12 effective bits.  Evaluate the canonical quintic exactly on that Q0.12
  // domain and round once:
  //   f(x)=6x^5-15x^4+10x^3
  //       = x^3(6x^2-15Sx+10S^2) / S^5, S=4096.
  // Returning Q0.16 therefore requires division by S^4 followed by <<4.
  // The integer form is monotone by construction and avoids the local
  // one-code reversals caused by repeated truncated fixed-point products.
  constexpr uint64_t kHalfQ48 = UINT64_C(1) << 47U;
  constexpr uint64_t kScale = 4096U;
  const uint64_t x = static_cast<uint64_t>(phase >> 4U);
  const uint64_t x2 = x * x;
  const uint64_t polynomial = 6U * x2
                            - 15U * kScale * x
                            + 10U * kScale * kScale;
  const uint64_t numerator = x2 * x * polynomial;
  uint64_t q12 = (numerator + kHalfQ48) >> 48U;
  if (q12 > 4095U) {
    q12 = 4095U;
  }
  return static_cast<uint16_t>(q12 << 4U);
}

int16_t segmentQ1F15(uint16_t phase, int16_t lastGradient, int16_t nextGradient) {
  constexpr int16_t kOneQ1F15 = 0x7FFF;
  const uint16_t u = fadeQ0F16(phase);
  const int16_t x = toSigned(phase);
  const int16_t a = fixedmath::mulI1F15(lastGradient, x);
  const int16_t b = fixedmath::mulI1F15(nextGradient, static_cast<int16_t>(x - kOneQ1F15));
  return fixedmath::lerpI1F15(toSigned(u), a, b);
}

uint32_t advancePhase(uint32_t phase, uint32_t delta, bool& rollover) {
  const uint32_t next = static_cast<uint32_t>(phase + delta);
  rollover = next < phase;
  return next;
}
}  // namespace fmd::perlinmath

namespace fmd::brownianmath {
uint16_t stepSize(uint16_t speed) {
  return static_cast<uint16_t>((256U + speed) >> 1U);
}

uint16_t eventCutoff(uint16_t speed) {
  return static_cast<uint16_t>(speed << 6U);
}

uint16_t directionCutoff(uint16_t target, uint16_t cutoff) {
  if (target < static_cast<uint16_t>(0xFFFFU / kCenteringMargin)) {
    return static_cast<uint16_t>((cutoff / 2U) - (cutoff / kCenteringStrength));
  }
  if (target > static_cast<uint16_t>(0xFFFFU - (0xFFFFU / kCenteringMargin))) {
    return static_cast<uint16_t>((cutoff / 2U) + (cutoff / kCenteringStrength));
  }
  return static_cast<uint16_t>(cutoff / 2U);
}

uint16_t nextTarget(uint16_t target, uint16_t random, uint16_t speed) {
  const uint16_t cutoff = eventCutoff(speed);
  if (random >= cutoff) {
    return target;
  }

  const uint16_t amount = stepSize(speed);
  if (random >= directionCutoff(target, cutoff)) {
    const uint32_t next = static_cast<uint32_t>(target) + static_cast<uint32_t>(amount);
    return next > 0xFFFFU ? 0xFFFFU : static_cast<uint16_t>(next);
  }
  return amount > target ? 0U : static_cast<uint16_t>(target - amount);
}

uint16_t textureAlphaQ0F16(uint16_t texture) {
  if (texture > 1023U) {
    texture = 1023U;
  }
  constexpr uint32_t kRange = static_cast<uint32_t>(kMaxAlphaQ0F16) - kMinAlphaQ0F16;
  const uint32_t scaled = static_cast<uint32_t>(texture) * kRange;
  return static_cast<uint16_t>(static_cast<uint32_t>(kMinAlphaQ0F16)
      + ((scaled + 511U) / 1023U));
}

void smoothToward(uint16_t target,
                  uint16_t alphaQ0F16,
                  uint16_t& current,
                  uint16_t& residualQ0F16,
                  int8_t& residualDirection) {
  if (current == target) {
    residualQ0F16 = 0U;
    residualDirection = 0;
    return;
  }

  const int8_t direction = current < target ? static_cast<int8_t>(1) : static_cast<int8_t>(-1);
  if (direction != residualDirection) {
    residualQ0F16 = 0U;
    residualDirection = direction;
  }

  const uint16_t delta = current < target
      ? static_cast<uint16_t>(target - current)
      : static_cast<uint16_t>(current - target);
  const uint32_t product = static_cast<uint32_t>(alphaQ0F16) * static_cast<uint32_t>(delta)
                         + static_cast<uint32_t>(residualQ0F16);
  uint16_t move = static_cast<uint16_t>(product >> 16U);
  residualQ0F16 = static_cast<uint16_t>(product & 0xFFFFU);

  current = direction > 0
      ? static_cast<uint16_t>(current + move)
      : static_cast<uint16_t>(current - move);

  if (current == target) {
    residualQ0F16 = 0U;
    residualDirection = 0;
  }
}
}  // namespace fmd::brownianmath

namespace fmd::beziermath {
uint16_t speedVariationScaleQ1F15(uint16_t knob, uint16_t cv) {
  constexpr uint16_t kAdcMax = 1023U;
  constexpr uint16_t kHalf = kAdcMax / 2U;
  constexpr uint16_t kDeadZone = 128U;
  constexpr uint16_t kRange = kHalf - kDeadZone;

  if (knob > kAdcMax) {
    knob = kAdcMax;
  }
  if (cv > kAdcMax) {
    cv = kAdcMax;
  }

  uint16_t magnitude = knob > kHalf
      ? static_cast<uint16_t>(knob - kHalf)
      : static_cast<uint16_t>(kHalf - knob);
  magnitude = magnitude > kDeadZone ? static_cast<uint16_t>(magnitude - kDeadZone) : 0U;

  uint16_t sum = static_cast<uint16_t>(magnitude + cv / 2U);
  if (sum > kRange) {
    sum = kRange;
  }
  return static_cast<uint16_t>((static_cast<uint32_t>(sum) * 0x7FFFU) / kRange);
}

uint16_t smoothCurveQ4F12(uint16_t phaseQ4F12) {
  // y = 3x^2 - 2x^3.  Evaluate the integer rational form directly:
  //   y_q12 = round(x_q12^2 * (3*S - 2*x_q12) / S^2), S=4096.
  // This avoids the one-code local reversals produced by repeatedly truncating
  // intermediate Q4.12 products while keeping the denominator a power-of-two.
  constexpr uint64_t kHalfQ24 = UINT64_C(1) << 23U;
  const uint64_t x = phaseQ4F12 > 4096U ? 4096U : phaseQ4F12;
  const uint64_t numerator = x * x * (UINT64_C(12288) - 2U * x);
  return static_cast<uint16_t>((numerator + kHalfQ24) >> 24U);
}

uint16_t reverseCurveQ4F12(uint16_t phaseQ4F12) {
  // y = 2x^3 - 3x^2 + 2x.  As above, evaluate the exact rational
  // polynomial and round only once.  floor/round of a monotone continuous
  // function remains monotone over the ordered integer phase domain.
  constexpr uint64_t kHalfQ24 = UINT64_C(1) << 23U;
  const uint64_t x = phaseQ4F12 > 4096U ? 4096U : phaseQ4F12;
  const uint64_t x2 = x * x;
  const uint64_t numerator = 2U * x2 * x
                           - UINT64_C(12288) * x2
                           + UINT64_C(33554432) * x;
  return static_cast<uint16_t>((numerator + kHalfQ24) >> 24U);
}

uint16_t textureBlendQ0F16(uint16_t texture) {
  if (texture > 1023U) {
    texture = 1023U;
  }
  return static_cast<uint16_t>((static_cast<uint32_t>(texture) * 0xFFFFU + 511U) / 1023U);
}

uint16_t morphCurveQ4F12(uint16_t phaseQ4F12, uint16_t texture) {
  const uint16_t reverse = reverseCurveQ4F12(phaseQ4F12);
  const uint16_t smooth = smoothCurveQ4F12(phaseQ4F12);
  if (texture == 0U) {
    return reverse;
  }
  if (texture >= 1023U) {
    return smooth;
  }
  return fixedmath::lerpU0F16(textureBlendQ0F16(texture), reverse, smooth);
}

uint16_t interpolateQ4F12(uint16_t phaseQ4F12,
                          uint16_t aQ4F12,
                          uint16_t bQ4F12,
                          uint16_t texture) {
  return fixedmath::lerpU4F12(morphCurveQ4F12(phaseQ4F12, texture), aQ4F12, bQ4F12);
}

int16_t triangularIcdfQ1F15(uint16_t uniform15, const IReferenceTables& tables) {
  uniform15 = static_cast<uint16_t>(uniform15 & 0x7FFFU);
  const uint16_t low = static_cast<uint16_t>(uniform15 >> 7U);
  const uint16_t high = static_cast<uint16_t>(low + 1U);
  const int16_t remainderQ1F15 = static_cast<int16_t>((static_cast<uint16_t>(uniform15 << 8U)) & 0x7FFFU);
  return fixedmath::lerpI1F15(remainderQ1F15,
                              tables.triangularIcdfQ1_15(low),
                              tables.triangularIcdfQ1_15(high));
}

uint32_t advancePhase(uint32_t phase, uint32_t delta, bool& rollover) {
  const uint32_t next = static_cast<uint32_t>(phase + delta);
  rollover = next < phase;
  return next;
}
}  // namespace fmd::beziermath

namespace fmd::lfomath {
uint16_t apexFromTexture(uint16_t texture) {
  if (texture > 1023U) {
    texture = 1023U;
  }
  return static_cast<uint16_t>((static_cast<uint32_t>(texture) * 0xFFFFU + 511U) / 1023U);
}

uint16_t waveformQ0F16(uint16_t phase, uint16_t apex) {
  if (apex == 0U) {
    return static_cast<uint16_t>(0xFFFFU - phase);
  }
  if (apex == 0xFFFFU) {
    return phase;
  }

  if (phase <= apex) {
    const uint32_t numerator = static_cast<uint32_t>(phase) * 0xFFFFU;
    return static_cast<uint16_t>(numerator / apex);
  }

  const uint16_t phaseRemaining = static_cast<uint16_t>(0xFFFFU - phase);
  const uint16_t fallLength = static_cast<uint16_t>(0xFFFFU - apex);
  const uint32_t numerator = static_cast<uint32_t>(phaseRemaining) * 0xFFFFU;
  return static_cast<uint16_t>(numerator / fallLength);
}

uint16_t waveform12(uint16_t phase, uint16_t apex) {
  return static_cast<uint16_t>(waveformQ0F16(phase, apex) >> 4U);
}

uint16_t remapPhasePreservingOutput(uint16_t phase,
                                    uint16_t oldApex,
                                    uint16_t newApex) {
  const uint16_t y = waveformQ0F16(phase, oldApex);
  const bool preferredRising = oldApex == 0xFFFFU || (oldApex != 0U && phase <= oldApex);

  // Evaluate both inverse branches of the new waveform.  Near the sawtooth
  // endpoints one branch can be only one phase code long and therefore cannot
  // represent an arbitrary output level; selecting the candidate with the
  // smaller forward-evaluation error keeps live Texture changes continuous.
  const uint32_t risingProduct = static_cast<uint32_t>(y) * static_cast<uint32_t>(newApex);
  const uint16_t risingPhase = static_cast<uint16_t>((risingProduct + 0x7FFFU) / 0xFFFFU);

  const uint16_t oneMinusY = static_cast<uint16_t>(0xFFFFU - y);
  const uint16_t fallLength = static_cast<uint16_t>(0xFFFFU - newApex);
  const uint32_t fallingProduct = static_cast<uint32_t>(oneMinusY) * static_cast<uint32_t>(fallLength);
  const uint16_t fallingTail = static_cast<uint16_t>((fallingProduct + 0x7FFFU) / 0xFFFFU);
  const uint16_t fallingPhase = static_cast<uint16_t>(static_cast<uint32_t>(newApex) + fallingTail);

  const uint16_t risingY = waveformQ0F16(risingPhase, newApex);
  const uint16_t fallingY = waveformQ0F16(fallingPhase, newApex);
  const uint16_t risingError = risingY > y
      ? static_cast<uint16_t>(risingY - y)
      : static_cast<uint16_t>(y - risingY);
  const uint16_t fallingError = fallingY > y
      ? static_cast<uint16_t>(fallingY - y)
      : static_cast<uint16_t>(y - fallingY);

  if (risingError < fallingError) {
    return risingPhase;
  }
  if (fallingError < risingError) {
    return fallingPhase;
  }
  return preferredRising ? risingPhase : fallingPhase;
}

uint32_t advancePhase(uint32_t phase, uint32_t delta, bool& rollover) {
  const uint32_t next = static_cast<uint32_t>(phase + delta);
  rollover = next < phase;
  return next;
}
}  // namespace fmd::lfomath
