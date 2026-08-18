#include "fmd/domain/PerlinAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FixedMath.h"
#include "fmd/domain/FrequencyMapping.h"

#include <stdint.h>

namespace fmd {
PerlinAlgorithm::PerlinAlgorithm(const IReferenceTables& tables, uint16_t seed)
    : tables_(tables), base_{0, 0, 0}, octave_{0, 0, 0}, rng_(seed) {
  base_.lastGrad = randomGrad(rng_);
  base_.nextGrad = randomGrad(rng_);
  octave_.lastGrad = randomGrad(rng_);
  octave_.nextGrad = randomGrad(rng_);
}

int16_t PerlinAlgorithm::randomGrad(ParallelLfsr& rng) {
  return perlinmath::gradientFromRandom(rng.next());
}

int16_t PerlinAlgorithm::stepOctave(Octave& octave, uint32_t deltaTime) {
  bool rollover = false;
  octave.time = perlinmath::advancePhase(octave.time, deltaTime, rollover);
  if (rollover) {
    octave.lastGrad = octave.nextGrad;
    octave.nextGrad = randomGrad(rng_);
  }
  return perlinmath::segmentQ1F15(static_cast<uint16_t>(octave.time >> 16U),
                                  octave.lastGrad,
                                  octave.nextGrad);
}

uint16_t PerlinAlgorithm::step(const ControlFrame& controls) {
  const uint32_t deltaTime = getDeltaTime(tables_, controls.speedKnob, controls.speedCv, 0);
  const int16_t base = stepOctave(base_, deltaTime);
  const int16_t octave = stepOctave(octave_, static_cast<uint32_t>(deltaTime * 4UL));
  const int16_t blend = static_cast<int16_t>(sumAdc(controls.textureKnob, controls.textureCv) << 5U);
  constexpr int16_t kOneQ1F15 = 0x7FFF;
  const int32_t raw = static_cast<int32_t>(base) * 3
                    + fixedmath::mulI1F15(base, static_cast<int16_t>(kOneQ1F15 - blend))
                    + fixedmath::mulI1F15(octave, blend);
  int32_t scaled = raw / 16 + 2048;
  if (scaled < 0) {
    scaled = 0;
  }
  if (scaled > 4095) {
    scaled = 4095;
  }
  return static_cast<uint16_t>(scaled);
}
}  // namespace fmd
