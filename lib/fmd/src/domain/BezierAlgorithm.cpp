#include "fmd/domain/BezierAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FixedMath.h"
#include "fmd/domain/FrequencyMapping.h"

#include <stdint.h>

namespace fmd {
BezierAlgorithm::BezierAlgorithm(const IReferenceTables& tables, uint16_t seed)
    : tables_(tables),
      time_(0),
      speedAdjust_(0),
      valueA_(0),
      valueB_(0),
      speedInitialized_(false),
      rng_(seed) {
  valueB_ = static_cast<uint16_t>(rng_.next() >> 4U);
}

uint32_t BezierAlgorithm::stepTime(uint16_t knob, uint16_t cv, bool& rollover) {
  const uint32_t deltaTime = getDeltaTime(tables_, knob, cv, speedAdjust_);
  time_ = beziermath::advancePhase(time_, deltaTime, rollover);
  return time_;
}

int16_t BezierAlgorithm::randomFromDistribution() {
  return beziermath::triangularIcdfQ1F15(static_cast<uint16_t>(rng_.next() & 0x7FFFU), tables_);
}

int16_t BezierAlgorithm::getSpeedAdjust(uint16_t knob, uint16_t cv) {
  const uint16_t scale = beziermath::speedVariationScaleQ1F15(knob, cv);
  if (scale == 0U) {
    return 0;
  }
  return fixedmath::mulI1F15(randomFromDistribution(), static_cast<int16_t>(scale));
}

uint16_t BezierAlgorithm::step(const ControlFrame& controls) {
  if (!speedInitialized_) {
    speedAdjust_ = getSpeedAdjust(controls.textureKnob, controls.textureCv);
    speedInitialized_ = true;
  }

  bool rollover = false;
  const uint32_t phase = stepTime(controls.speedKnob, controls.speedCv, rollover);
  if (rollover) {
    valueA_ = valueB_;
    valueB_ = static_cast<uint16_t>(rng_.next() >> 4U);
    speedAdjust_ = getSpeedAdjust(controls.textureKnob, controls.textureCv);
  }

  const uint16_t phaseQ4F12 = static_cast<uint16_t>(phase >> 20U);
  return beziermath::interpolateQ4F12(phaseQ4F12,
                                         valueA_,
                                         valueB_,
                                         controls.textureKnob);
}
}  // namespace fmd
