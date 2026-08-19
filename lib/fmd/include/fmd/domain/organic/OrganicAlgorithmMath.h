/**
 * @file OrganicAlgorithmMath.h
 * Declares pure mathematical primitives used by the optional Organic algorithm bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ORGANIC_ALGORITHM_MATH_H
#define FMD_DOMAIN_ORGANIC_ALGORITHM_MATH_H

#include <stdint.h>

namespace fmd::fractalmath {

/** @brief Constant-sum weights for the three gradient-noise scales. */
struct OctaveWeights {
  uint16_t macro;   ///< Weight of the 1x octave, denominator 1024.
  uint16_t meso;    ///< Weight of the 4x octave, denominator 1024.
  uint16_t detail;  ///< Weight of the 16x octave, denominator 1024.
};

/**
 * @brief Map Texture to a constant-gain redistribution across three noise scales.
 * @param textureControl Saturated Texture value in the 10-bit ADC domain.
 * @return Three weights whose sum is exactly 1024.
 *
 * At zero Texture the macro octave receives all weight. At full Texture the
 * distribution is 512/320/192, so finer structure is introduced without
 * removing the large-scale drift completely.
 */
OctaveWeights octaveWeights(uint16_t textureControl);

/**
 * @brief Mix three signed Q1.15 octave samples with constant-sum weights.
 * @param macroQ1F15 1x octave sample.
 * @param mesoQ1F15 4x octave sample.
 * @param detailQ1F15 16x octave sample.
 * @param weights Constant-sum weights from octaveWeights().
 * @return Mixed signed Q1.15 sample.
 */
int16_t mixOctavesQ1F15(int16_t macroQ1F15,
                        int16_t mesoQ1F15,
                        int16_t detailQ1F15,
                        const OctaveWeights& weights);

}  // namespace fmd::fractalmath

namespace fmd::vectormath {

/**
 * @brief Evaluate a continuous bipolar triangle wave from an unsigned Q0.16 phase.
 * @param phaseQ0F16 Complete 16-bit phase domain.
 * @return Approximately -1..+1 in signed Q1.15.
 */
int16_t triangleSignedQ1F15(uint16_t phaseQ0F16);

/**
 * @brief Perturb a base phase increment by another axis of the toroidal vector field.
 * @param basePhaseIncrement Uncoupled phase increment for this axis.
 * @param otherAxisWaveQ1F15 Bipolar triangle projection of the other state axis.
 * @param textureControl Coupling amount in the 10-bit ADC domain.
 * @param invertCoupling true to reverse the cross-coupling sign.
 * @return Non-negative coupled phase increment, bounded to uint32_t.
 *
 * Full-scale Texture limits cross-coupling to approximately +/-25 percent of
 * the base rate, keeping both phase axes forward-moving and numerically stable.
 */
uint32_t coupledPhaseIncrement(uint32_t basePhaseIncrement,
                               int16_t otherAxisWaveQ1F15,
                               uint16_t textureControl,
                               bool invertCoupling);

/**
 * @brief Project the two toroidal state axes onto the unipolar 12-bit output.
 * @param xWaveQ1F15 First bipolar state projection.
 * @param yWaveQ1F15 Second bipolar state projection.
 * @return Average of both axes mapped to DAC code 0..4095.
 */
uint16_t projectToDac12(int16_t xWaveQ1F15, int16_t yWaveQ1F15);

}  // namespace fmd::vectormath

namespace fmd::rainmath {

/** Minimum per-sample leaky-envelope decay coefficient in unsigned Q0.16. */
constexpr uint16_t kMinDecayAlphaQ0F16 = 4U;
/** Maximum per-sample leaky-envelope decay coefficient in unsigned Q0.16. */
constexpr uint16_t kMaxDecayAlphaQ0F16 = 8188U;

/**
 * @brief Map Rain Density to a discrete-time Bernoulli event threshold.
 * @param densityControl Saturated Texture/Density value in the 10-bit ADC domain.
 * @return Event cutoff in the complete 16-bit random domain.
 *
 * The quadratic law gives useful resolution for sparse events while reaching
 * approximately one event per four samples at full scale.
 */
uint16_t eventCutoff(uint16_t densityControl);

/**
 * @brief Map Rain Speed to the exponential-tail decay coefficient.
 * @param speedControl Saturated Speed value in the 10-bit ADC domain.
 * @return Monotonic Q0.16 decay coefficient from kMinDecayAlphaQ0F16 to
 *         kMaxDecayAlphaQ0F16.
 */
uint16_t decayAlphaQ0F16(uint16_t speedControl);

/**
 * @brief Convert a random word to one unipolar rain-drop impulse amplitude.
 * @param randomValue Uniform 16-bit pseudo-random input.
 * @return Impulse amplitude in the 16-bit envelope domain.
 */
uint16_t impulseAmplitude(uint16_t randomValue);

/**
 * @brief Apply one leaky-envelope decay step while preserving sub-code motion.
 * @param alphaQ0F16 Per-sample decay coefficient.
 * @param envelopeValue In/out 16-bit aggregate shot-noise envelope.
 * @param decayResidualQ0F16 In/out fractional decay remainder.
 */
void decayEnvelope(uint16_t alphaQ0F16,
                   uint16_t& envelopeValue,
                   uint16_t& decayResidualQ0F16);

/**
 * @brief Add one drop impulse to the aggregate envelope with saturation.
 * @param envelopeValue Current 16-bit envelope.
 * @param impulseValue Positive impulse amplitude.
 * @return Saturating sum in the 16-bit envelope domain.
 */
uint16_t addImpulseSaturating(uint16_t envelopeValue, uint16_t impulseValue);

}  // namespace fmd::rainmath

namespace fmd::attractormath {

/** Signed fixed-point coordinate used by the Hénon map, Q2.14. */
struct HenonState {
  int16_t xQ2F14;  ///< Horizontal state coordinate.
  int16_t yQ2F14;  ///< Vertical state coordinate.
};

/** Minimum Texture-selected Hénon a parameter, 1.20 in Q2.14. */
constexpr uint16_t kMinParameterAQ2F14 = 19661U;
/** Maximum Texture-selected Hénon a parameter, 1.40 in Q2.14. */
constexpr uint16_t kMaxParameterAQ2F14 = 22938U;
/** Fixed canonical Hénon b parameter, approximately 0.30 in Q2.14. */
constexpr uint16_t kParameterBQ2F14 = 4915U;

/**
 * @brief Map Texture to the Hénon a parameter over the bounded 1.20..1.40 range.
 * @param textureControl Saturated Texture value in the 10-bit ADC domain.
 * @return Parameter a in unsigned Q2.14.
 */
uint16_t parameterAQ2F14(uint16_t textureControl);

/**
 * @brief Iterate the Hénon map once in Q2.14 fixed point.
 * @param state Current x/y state.
 * @param parameterAQ2F14 Hénon a parameter from parameterAQ2F14().
 * @return Next map state using x[n+1] = 1 - a*x[n]^2 + y[n], y[n+1] = b*x[n].
 */
HenonState iterateHenon(const HenonState& state, uint16_t parameterAQ2F14);

/**
 * @brief Interpolate one signed Q2.14 coordinate using a Q0.12 phase fraction.
 * @param startQ2F14 Segment start coordinate.
 * @param endQ2F14 Segment destination coordinate.
 * @param phaseQ0F12 Interpolation phase 0..4095.
 * @return Linearly interpolated signed Q2.14 coordinate.
 */
int16_t interpolateQ2F14(int16_t startQ2F14,
                         int16_t endQ2F14,
                         uint16_t phaseQ0F12);

/**
 * @brief Map a full signed Q2.14 coordinate to the complete 12-bit DAC domain.
 * @param coordinateQ2F14 Signed coordinate where -2 maps to 0 and +2 approaches 4095.
 * @return Saturated 12-bit DAC code.
 */
uint16_t coordinateToDac12(int16_t coordinateQ2F14);

}  // namespace fmd::attractormath

#endif  // FMD_DOMAIN_ORGANIC_ALGORITHM_MATH_H
