#include <unity.h>

#include <cstdint>

#include "fmd/domain/ParallelLfsr.h"

void test_upstream_parallel_lfsr_sequence() {
  fmd::ParallelLfsr r(0x1234U);
  const uint16_t expected[8] = {0x6CECU, 0x3676U, 0x9B3BU, 0x4D9DU,
                                0x26CEU, 0x9367U, 0x49B3U, 0xA4D9U};
  for (uint8_t i = 0U; i < 8U; ++i) {
    TEST_ASSERT_EQUAL_HEX16(expected[i], r.next());
  }
}

void test_parallel_lfsr_extreme_seeds_do_not_lock_at_zero_or_constant_output() {
  constexpr uint16_t seeds[] = {0x0000U, 0xFFFFU, 0x0001U, 0x8000U};
  for (const uint16_t seed : seeds) {
    fmd::ParallelLfsr rng(seed);
    const uint16_t first = rng.next();
    bool sawNonZero = first != 0U;
    bool sawDifferent = false;
    for (uint32_t i = 0U; i < 4095U; ++i) {
      const uint16_t value = rng.next();
      sawNonZero = sawNonZero || value != 0U;
      sawDifferent = sawDifferent || value != first;
    }
    TEST_ASSERT_TRUE(sawNonZero);
    TEST_ASSERT_TRUE(sawDifferent);
  }
}

void test_parallel_lfsr_long_runs_are_reproducible_and_have_high_short_window_diversity() {
  fmd::ParallelLfsr a(0xA55AU);
  fmd::ParallelLfsr b(0xA55AU);
  uint16_t seen[256] = {};
  uint16_t occupied = 0U;

  for (uint32_t i = 0U; i < 16384U; ++i) {
    const uint16_t x = a.next();
    const uint16_t y = b.next();
    TEST_ASSERT_EQUAL_UINT16(x, y);
    const uint8_t bucket = static_cast<uint8_t>(x >> 8U);
    if (seen[bucket] == 0U) {
      seen[bucket] = 1U;
      ++occupied;
    }
  }
  TEST_ASSERT_GREATER_THAN_UINT16(240U, occupied);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_upstream_parallel_lfsr_sequence);
  RUN_TEST(test_parallel_lfsr_extreme_seeds_do_not_lock_at_zero_or_constant_output);
  RUN_TEST(test_parallel_lfsr_long_runs_are_reproducible_and_have_high_short_window_diversity);
  return UNITY_END();
}
