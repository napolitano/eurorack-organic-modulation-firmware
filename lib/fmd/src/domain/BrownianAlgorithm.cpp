#include "fmd/domain/BrownianAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"

#include <stdint.h>

namespace fmd {
BrownianAlgorithm::BrownianAlgorithm(uint16_t seed)
    : targetValue_(0),
      currentValue_(0),
      smoothingResidual_(0),
      smoothingDirection_(0),
      rng_(seed) {}

void BrownianAlgorithm::stepTargetValue(uint16_t speed) {
  targetValue_ = brownianmath::nextTarget(targetValue_, rng_.next(), speed);
}

void BrownianAlgorithm::stepSmoothedValue(uint16_t texture) {
  brownianmath::smoothToward(targetValue_,
                            brownianmath::textureAlphaQ0F16(texture),
                            currentValue_,
                            smoothingResidual_,
                            smoothingDirection_);
}

uint16_t BrownianAlgorithm::step(const ControlFrame& controls) {
  stepTargetValue(sumAdc(controls.speedKnob, controls.speedCv));
  stepSmoothedValue(sumAdc(controls.textureKnob, controls.textureCv));
  return static_cast<uint16_t>(currentValue_ >> 4U);
}
}  // namespace fmd
