/**
 * @file GenerativeAlgorithmMath.h
 * Declares pure mathematical primitives used by the optional Generative algorithm bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_GENERATIVE_ALGORITHM_MATH_H
#define FMD_DOMAIN_GENERATIVE_ALGORITHM_MATH_H

#include <stdint.h>

namespace fmd::turingmath {

/** Maximum mutation threshold in the 16-bit random domain, exactly 1/2. */
constexpr uint32_t kMaximumMutationCutoff = 32768UL;

/**
 * @brief Map Texture to a mutation cutoff spanning probability 0..1/2.
 * @param textureControl Saturated or raw Texture value in the ADC domain.
 * @return Threshold in 0..32768 for comparison against a 16-bit random word.
 */
uint32_t mutationCutoff(uint16_t textureControl);

/**
 * @brief Decide whether one feedback bit mutates.
 * @param textureControl Texture value in the ADC domain.
 * @param randomWord Uniform 16-bit pseudo-random word.
 * @return true when the feedback bit must be inverted.
 */
bool shouldMutate(uint16_t textureControl, uint16_t randomWord);

/**
 * @brief Rotate one 16-bit register step with optional feedback inversion.
 * @param registerState Current 16-bit state; bit 0 is recycled into bit 15.
 * @param mutateFeedback true to invert the recycled feedback bit.
 * @return New 16-bit register state.
 */
uint16_t advanceRegister(uint16_t registerState, bool mutateFeedback);

/**
 * @brief Project the upper twelve register bits to the MCP4922 DAC domain.
 * @param registerState Current 16-bit state.
 * @return 12-bit DAC code 0..4095.
 */
constexpr uint16_t projectToDac12(uint16_t registerState) {
  return static_cast<uint16_t>(registerState >> 4U);
}

}  // namespace fmd::turingmath

namespace fmd::markovmath {

/** Number of symbolic states in the Generative Markov vocabulary. */
constexpr uint8_t kStateCount = 8U;
/** Width of one source band used during stratified vocabulary generation. */
constexpr uint16_t kVocabularyBandWidth = 512U;

/**
 * @brief Decide whether a transition uses uniform exploration.
 * @param textureControl Texture/Exploration in the ADC domain.
 * @param randomWord Uniform 16-bit pseudo-random word.
 * @return false at Texture 0, true at Texture 1023, otherwise Bernoulli(Texture/1023).
 */
bool useUniformExploration(uint16_t textureControl, uint16_t randomWord);

/**
 * @brief Apply the fixed 4/8, 2/8, 1/8, 1/8 structured transition grammar.
 * @param currentState Current symbolic state 0..7.
 * @param randomThreeBits Random decision value; only the low three bits are used.
 * @return Next symbolic state 0..7.
 */
uint8_t structuredNextState(uint8_t currentState, uint8_t randomThreeBits);

/**
 * @brief Convert one 16-bit random word to an exactly eight-way uniform state index.
 * @param randomWord Uniform 16-bit pseudo-random input.
 * @return State index 0..7.
 */
constexpr uint8_t uniformState(uint16_t randomWord) {
  return static_cast<uint8_t>(randomWord & 0x0007U);
}

/**
 * @brief Generate one value inside a selected 512-code DAC source band.
 * @param bandIndex Source band 0..7.
 * @param randomWord Uniform 16-bit pseudo-random input.
 * @return 12-bit value lying within the selected band.
 */
uint16_t stratifiedVocabularyValue(uint8_t bandIndex, uint16_t randomWord);

/**
 * @brief Scale a 16-bit random word into an integer range without division.
 * @param randomWord Uniform 16-bit input.
 * @param rangeSize Positive result range size.
 * @return Integer in 0..rangeSize-1 using multiply-high scaling.
 *
 * When rangeSize does not divide 65536 exactly, bucket counts differ by at most
 * one source word. This bounded bias is acceptable for eight-item startup shuffling.
 */
uint8_t scaleRandomToRange(uint16_t randomWord, uint8_t rangeSize);

}  // namespace fmd::markovmath

namespace fmd::motifmath {

/** Fixed phrase length of the Generative Motif algorithm. */
constexpr uint8_t kPhraseLength = 8U;

/** Supported project-defined structural phrase transformations. */
enum class Operation : uint8_t {
  Rotate = 0U,      ///< Circular rotation by one position.
  AdjacentSwap = 1U, ///< Exchange one adjacent circular pair.
  ReverseThree = 2U, ///< Reverse one circular three-step span.
  ReplaceOne = 3U    ///< Replace one explicit phrase value.
};

/**
 * @brief Decide whether one phrase-boundary edit occurs.
 * @param textureControl Texture/Variation in the ADC domain.
 * @param randomWord Uniform 16-bit pseudo-random word.
 * @return false at Texture 0, true at Texture 1023, otherwise Bernoulli(Texture/1023).
 */
bool shouldEdit(uint16_t textureControl, uint16_t randomWord);

/** @brief Select one of the four equiprobable transformation classes. */
constexpr Operation operationFromRandom(uint16_t randomWord) {
  return static_cast<Operation>(randomWord & 0x0003U);
}

/**
 * @brief Rotate an eight-value phrase one position.
 * @param phrase Mutable phrase array of exactly eight values.
 * @param rotateLeft true for left rotation, false for right rotation.
 */
void rotate(uint16_t phrase[kPhraseLength], bool rotateLeft);

/**
 * @brief Swap one circular adjacent pair in an eight-value phrase.
 * @param phrase Mutable phrase array.
 * @param startIndex First member of the pair; low three bits select 0..7.
 */
void adjacentSwap(uint16_t phrase[kPhraseLength], uint8_t startIndex);

/**
 * @brief Reverse one circular three-step span by exchanging its endpoints.
 * @param phrase Mutable phrase array.
 * @param startIndex First member of the span; low three bits select 0..7.
 */
void reverseThree(uint16_t phrase[kPhraseLength], uint8_t startIndex);

/**
 * @brief Replace one phrase value with a supplied 12-bit code.
 * @param phrase Mutable phrase array.
 * @param index Position to replace; low three bits select 0..7.
 * @param replacementValue Raw replacement code, defensively clamped to 12 bits.
 */
void replaceOne(uint16_t phrase[kPhraseLength], uint8_t index, uint16_t replacementValue);

}  // namespace fmd::motifmath

namespace fmd::urnmath {

/** Number of fixed output states in the reinforced vocabulary. */
constexpr uint8_t kStateCount = 8U;
/** Equal baseline selection weight for every state. */
constexpr uint16_t kBaselineWeight = 32U;
/** Saturation limit for one reinforced state weight. */
constexpr uint16_t kMaximumWeight = 1023U;
/** Maximum reinforcement applied by full-scale Texture. */
constexpr uint16_t kMaximumReinforcement = 64U;

/**
 * @brief Relax one bounded state weight toward the common baseline by 31/32.
 * @param weight Current weight, normally in kBaselineWeight..kMaximumWeight.
 * @return Relaxed weight, never below the baseline.
 */
uint16_t relaxWeight(uint16_t weight);

/**
 * @brief Map Texture monotonically to the reinforcement amount 0..64.
 * @param textureControl Texture/Reinforcement in the ADC domain.
 * @return Integer reinforcement amount.
 */
uint16_t reinforcementAmount(uint16_t textureControl);

/**
 * @brief Add reinforcement while saturating at kMaximumWeight.
 * @param weight Relaxed selected-state weight.
 * @param reinforcement Amount to add.
 * @return Saturated reinforced weight.
 */
uint16_t reinforceSaturating(uint16_t weight, uint16_t reinforcement);

/**
 * @brief Convert one random word into a weighted-draw target 0..totalWeight-1.
 * @param randomWord Uniform 16-bit pseudo-random input.
 * @param totalWeight Positive sum of all category weights.
 * @return Weighted cumulative target.
 *
 * Multiply-high scaling avoids runtime division. For totals that do not divide
 * 65536, category-source bucket sizes differ by at most one random word.
 */
uint16_t weightedTarget(uint16_t randomWord, uint16_t totalWeight);

/**
 * @brief Select one category from eight positive integer weights.
 * @param weights Array of eight bounded weights.
 * @param randomWord Uniform 16-bit pseudo-random input.
 * @return Selected state index 0..7.
 */
uint8_t selectWeightedState(const uint16_t weights[kStateCount], uint16_t randomWord);

/**
 * @brief Map one Urn state to its fixed evenly spaced 12-bit output level.
 * @param state State index 0..7.
 * @return Exact DAC code 585*state spanning 0..4095.
 */
uint16_t outputLevel(uint8_t state);

}  // namespace fmd::urnmath

#endif  // FMD_DOMAIN_GENERATIVE_ALGORITHM_MATH_H
