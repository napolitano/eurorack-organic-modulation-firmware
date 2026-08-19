/**
 * @file GenerativeAlgorithmMath.cpp
 * Implements pure mathematical primitives for the optional Generative bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/generative/GenerativeAlgorithmMath.h"

#include "fmd/domain/Types.h"

#include <stdint.h>

namespace fmd::turingmath {

uint32_t mutationCutoff(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  return (static_cast<uint32_t>(textureControl) * kMaximumMutationCutoff + 511UL) /
         1023UL;
}

bool shouldMutate(uint16_t textureControl, uint16_t randomWord) {
  return static_cast<uint32_t>(randomWord) < mutationCutoff(textureControl);
}

uint16_t advanceRegister(uint16_t registerState, bool mutateFeedback) {
  const uint16_t feedbackBit = static_cast<uint16_t>(registerState & 0x0001U);
  const uint16_t incomingBit = static_cast<uint16_t>(
      feedbackBit ^ static_cast<uint16_t>(mutateFeedback ? 1U : 0U));
  return static_cast<uint16_t>((registerState >> 1U) | (incomingBit << 15U));
}

}  // namespace fmd::turingmath

namespace fmd::markovmath {

bool useUniformExploration(uint16_t textureControl, uint16_t randomWord) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 0U) {
    return false;
  }
  if (textureControl == 1023U) {
    return true;
  }
  const uint32_t cutoff =
      (static_cast<uint32_t>(textureControl) * 65536UL) / 1023UL;
  return static_cast<uint32_t>(randomWord) < cutoff;
}

uint8_t structuredNextState(uint8_t currentState, uint8_t randomThreeBits) {
  currentState &= 0x07U;
  switch (randomThreeBits & 0x07U) {
    case 0U:
    case 1U:
    case 2U:
    case 3U:
      return currentState;
    case 4U:
    case 5U:
      return static_cast<uint8_t>((currentState + 1U) & 0x07U);
    case 6U:
      return static_cast<uint8_t>((currentState + 7U) & 0x07U);
    default:
      return static_cast<uint8_t>((currentState + 4U) & 0x07U);
  }
}

uint16_t stratifiedVocabularyValue(uint8_t bandIndex, uint16_t randomWord) {
  bandIndex &= 0x07U;
  return static_cast<uint16_t>(
      static_cast<uint16_t>(bandIndex * kVocabularyBandWidth) +
      static_cast<uint16_t>(randomWord & (kVocabularyBandWidth - 1U)));
}

uint8_t scaleRandomToRange(uint16_t randomWord, uint8_t rangeSize) {
  if (rangeSize == 0U) {
    return 0U;
  }
  return static_cast<uint8_t>(
      (static_cast<uint32_t>(randomWord) * rangeSize) >> 16U);
}

}  // namespace fmd::markovmath

namespace fmd::motifmath {

bool shouldEdit(uint16_t textureControl, uint16_t randomWord) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 0U) {
    return false;
  }
  if (textureControl == 1023U) {
    return true;
  }
  const uint32_t cutoff =
      (static_cast<uint32_t>(textureControl) * 65536UL) / 1023UL;
  return static_cast<uint32_t>(randomWord) < cutoff;
}

void rotate(uint16_t phrase[kPhraseLength], bool rotateLeft) {
  if (rotateLeft) {
    const uint16_t first = phrase[0];
    for (uint8_t index = 0U; index < kPhraseLength - 1U; ++index) {
      phrase[index] = phrase[static_cast<uint8_t>(index + 1U)];
    }
    phrase[kPhraseLength - 1U] = first;
    return;
  }

  const uint16_t last = phrase[kPhraseLength - 1U];
  for (uint8_t index = kPhraseLength - 1U; index > 0U; --index) {
    phrase[index] = phrase[static_cast<uint8_t>(index - 1U)];
  }
  phrase[0] = last;
}

void adjacentSwap(uint16_t phrase[kPhraseLength], uint8_t startIndex) {
  const uint8_t firstIndex = static_cast<uint8_t>(startIndex & 0x07U);
  const uint8_t secondIndex = static_cast<uint8_t>((firstIndex + 1U) & 0x07U);
  const uint16_t temporary = phrase[firstIndex];
  phrase[firstIndex] = phrase[secondIndex];
  phrase[secondIndex] = temporary;
}

void reverseThree(uint16_t phrase[kPhraseLength], uint8_t startIndex) {
  const uint8_t firstIndex = static_cast<uint8_t>(startIndex & 0x07U);
  const uint8_t thirdIndex = static_cast<uint8_t>((firstIndex + 2U) & 0x07U);
  const uint16_t temporary = phrase[firstIndex];
  phrase[firstIndex] = phrase[thirdIndex];
  phrase[thirdIndex] = temporary;
}

void replaceOne(uint16_t phrase[kPhraseLength], uint8_t index, uint16_t replacementValue) {
  phrase[static_cast<uint8_t>(index & 0x07U)] =
      static_cast<uint16_t>(replacementValue & 0x0FFFU);
}

}  // namespace fmd::motifmath

namespace fmd::urnmath {

uint16_t relaxWeight(uint16_t weight) {
  if (weight <= kBaselineWeight) {
    return kBaselineWeight;
  }
  if (weight > kMaximumWeight) {
    weight = kMaximumWeight;
  }
  const uint16_t excess = static_cast<uint16_t>(weight - kBaselineWeight);
  return static_cast<uint16_t>(
      kBaselineWeight + ((static_cast<uint32_t>(excess) * 31UL) >> 5U));
}

uint16_t reinforcementAmount(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(textureControl) * kMaximumReinforcement + 511UL) /
      1023UL);
}

uint16_t reinforceSaturating(uint16_t weight, uint16_t reinforcement) {
  const uint32_t sum = static_cast<uint32_t>(weight) + reinforcement;
  return sum > kMaximumWeight ? kMaximumWeight : static_cast<uint16_t>(sum);
}

uint16_t weightedTarget(uint16_t randomWord, uint16_t totalWeight) {
  if (totalWeight == 0U) {
    return 0U;
  }
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(randomWord) * totalWeight) >> 16U);
}

uint8_t selectWeightedState(const uint16_t weights[kStateCount], uint16_t randomWord) {
  uint16_t totalWeight = 0U;
  for (uint8_t index = 0U; index < kStateCount; ++index) {
    totalWeight = static_cast<uint16_t>(totalWeight + weights[index]);
  }
  if (totalWeight == 0U) {
    return 0U;
  }

  const uint16_t target = weightedTarget(randomWord, totalWeight);
  uint16_t cumulativeWeight = 0U;
  for (uint8_t index = 0U; index < kStateCount; ++index) {
    cumulativeWeight = static_cast<uint16_t>(cumulativeWeight + weights[index]);
    if (target < cumulativeWeight) {
      return index;
    }
  }
  return static_cast<uint8_t>(kStateCount - 1U);
}

uint16_t outputLevel(uint8_t state) {
  state &= 0x07U;
  return static_cast<uint16_t>(585U * state);
}

}  // namespace fmd::urnmath
