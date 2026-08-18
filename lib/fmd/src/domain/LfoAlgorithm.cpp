#include "fmd/domain/LfoAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

#include <stdint.h>

namespace fmd {
LfoAlgorithm::LfoAlgorithm(const IReferenceTables& tables)
    : tables_(tables), phase_(0), apex_(0x7FFFU), initialized_(false) {}

uint16_t LfoAlgorithm::step(const ControlFrame& controls) {
  const uint16_t texture = sumAdc(controls.textureKnob, controls.textureCv);
  const uint16_t requestedApex = lfomath::apexFromTexture(texture);

  if (!initialized_) {
    apex_ = requestedApex;
    initialized_ = true;
  } else if (requestedApex != apex_) {
    const uint16_t phaseQ0F16 = static_cast<uint16_t>(phase_ >> 16U);
    const uint16_t remapped = lfomath::remapPhasePreservingOutput(phaseQ0F16,
                                                                  apex_,
                                                                  requestedApex);
    phase_ = (static_cast<uint32_t>(remapped) << 16U) | (phase_ & 0xFFFFU);
    apex_ = requestedApex;
  }

  bool rollover = false;
  phase_ = lfomath::advancePhase(phase_,
                                getDeltaTime(tables_, controls.speedKnob, controls.speedCv, 0),
                                rollover);
  (void)rollover;  // Wrap is intentional; no cycle-boundary state update is required.
  return lfomath::waveform12(static_cast<uint16_t>(phase_ >> 16U), apex_);
}
}  // namespace fmd
