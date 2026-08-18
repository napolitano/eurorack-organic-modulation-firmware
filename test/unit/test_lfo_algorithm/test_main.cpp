#include <unity.h>

#include <cmath>
#include <cstdint>

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/LfoAlgorithm.h"
#include "fmd/domain/FrequencyMapping.h"
#include "MemoryReferenceTables.h"

namespace {
double waveformReference(double phase, double apex) {
  if (apex <= 0.0) return 1.0 - phase;
  if (apex >= 1.0) return phase;
  return phase <= apex ? phase / apex : (1.0 - phase) / (1.0 - apex);
}
}

void test_lfo_texture_maps_monotonically_to_exact_saw_endpoints() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::lfomath::apexFromTexture(0U));
  TEST_ASSERT_EQUAL_UINT16(0xFFFFU, fmd::lfomath::apexFromTexture(1023U));
  uint16_t previous = 0U;
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const uint16_t apex = fmd::lfomath::apexFromTexture(texture);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, apex);
    previous = apex;
  }
}

void test_lfo_apex_mapping_clamps_out_of_range_texture() {
  TEST_ASSERT_EQUAL_UINT16(0xFFFFU, fmd::lfomath::apexFromTexture(0xFFFFU));
}

void test_lfo_skew_triangle_has_exact_minimum_peak_and_cycle_end() {
  constexpr uint16_t apex = 32768U;
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::lfomath::waveformQ0F16(0U, apex));
  TEST_ASSERT_EQUAL_UINT16(0xFFFFU, fmd::lfomath::waveformQ0F16(apex, apex));
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::lfomath::waveformQ0F16(0xFFFFU, apex));
}

void test_lfo_endpoint_modes_are_exact_rising_and_falling_saws() {
  constexpr uint16_t phases[] = {0U, 1U, 16384U, 32768U, 49152U, 65534U, 65535U};
  for (const uint16_t phase : phases) {
    TEST_ASSERT_EQUAL_UINT16(phase, fmd::lfomath::waveformQ0F16(phase, 0xFFFFU));
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(0xFFFFU - phase),
                             fmd::lfomath::waveformQ0F16(phase, 0U));
  }
}

void test_lfo_waveform_matches_piecewise_linear_reference() {
  constexpr uint16_t apexes[] = {1U, 8192U, 16384U, 32768U, 49152U, 57344U, 65534U};
  for (const uint16_t apex : apexes) {
    for (uint32_t phase = 0U; phase <= 0xFFFFU; phase += 257U) {
      const double p = static_cast<double>(phase) / 65535.0;
      const double a = static_cast<double>(apex) / 65535.0;
      const double expected = waveformReference(p, a);
      const double actual = static_cast<double>(fmd::lfomath::waveformQ0F16(static_cast<uint16_t>(phase), apex)) / 65535.0;
      TEST_ASSERT_DOUBLE_WITHIN(0.00004, expected, actual);
    }
  }
}

void test_lfo_rise_and_fall_are_monotonic_for_each_apex() {
  constexpr uint16_t apexes[] = {1U, 8192U, 32768U, 57344U, 65534U};
  for (const uint16_t apex : apexes) {
    uint16_t previous = 0U;
    for (uint32_t phase = 0U; phase <= apex; ++phase) {
      const uint16_t current = fmd::lfomath::waveformQ0F16(static_cast<uint16_t>(phase), apex);
      TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
      previous = current;
    }
    previous = 0xFFFFU;
    for (uint32_t phase = static_cast<uint32_t>(apex) + 1U; phase <= 0xFFFFU; ++phase) {
      const uint16_t current = fmd::lfomath::waveformQ0F16(static_cast<uint16_t>(phase), apex);
      TEST_ASSERT_LESS_OR_EQUAL_UINT16(previous, current);
      previous = current;
    }
  }
}

void test_lfo_phase_remap_preserves_output_within_two_q0f16_codes() {
  constexpr uint16_t apexes[] = {0U, 1U, 8192U, 32768U, 57344U, 65534U, 65535U};
  constexpr uint16_t phases[] = {0U, 1U, 4096U, 16384U, 32768U, 49152U, 61440U, 65534U, 65535U};
  for (const uint16_t oldApex : apexes) {
    for (const uint16_t newApex : apexes) {
      for (const uint16_t phase : phases) {
        const uint16_t before = fmd::lfomath::waveformQ0F16(phase, oldApex);
        const uint16_t mapped = fmd::lfomath::remapPhasePreservingOutput(phase, oldApex, newApex);
        const uint16_t after = fmd::lfomath::waveformQ0F16(mapped, newApex);
        TEST_ASSERT_UINT16_WITHIN(2U, before, after);
      }
    }
  }
}

void test_lfo_phase_wrap_preserves_overshoot() {
  bool rollover = false;
  const uint32_t result = fmd::lfomath::advancePhase(0xFFFFFF00UL, 0x00000200UL, rollover);
  TEST_ASSERT_TRUE(rollover);
  TEST_ASSERT_EQUAL_UINT32(0x00000100UL, result);
}

void test_lfo_applies_texture_on_first_processing_step() {
  MemoryReferenceTables tables;
  fmd::LfoAlgorithm falling(tables);
  fmd::LfoAlgorithm rising(tables);
  const uint16_t a = falling.step({0U, 0U, 0U, 0U});
  const uint16_t b = rising.step({0U, 0U, 0U, 1023U});
  TEST_ASSERT_GREATER_THAN_UINT16(4000U, a);
  TEST_ASSERT_LESS_THAN_UINT16(10U, b);
}


void test_lfo_phase_remap_preserves_output_over_dense_phase_grid() {
  constexpr uint16_t apexes[] = {0U, 1U, 4096U, 16384U, 32768U, 49152U, 61440U, 65534U, 65535U};
  for (const uint16_t oldApex : apexes) {
    for (const uint16_t newApex : apexes) {
      for (uint32_t phase = 0U; phase <= 0xFFFFU; phase += 257U) {
        const uint16_t before = fmd::lfomath::waveformQ0F16(static_cast<uint16_t>(phase), oldApex);
        const uint16_t mapped = fmd::lfomath::remapPhasePreservingOutput(
            static_cast<uint16_t>(phase), oldApex, newApex);
        const uint16_t after = fmd::lfomath::waveformQ0F16(mapped, newApex);
        TEST_ASSERT_UINT16_WITHIN(2U, before, after);
      }
    }
  }
}

void test_lfo_constant_controls_match_phase_accumulator_across_multiple_wraps() {
  MemoryReferenceTables tables;
  fmd::LfoAlgorithm algorithm(tables);
  const fmd::ControlFrame controls{1023U, 0U, 1023U, 1023U};
  const uint16_t apex = fmd::lfomath::apexFromTexture(1023U);
  const uint32_t delta = fmd::getDeltaTime(tables, controls.speedKnob, controls.speedCv, 0);
  uint32_t phase = 0U;
  uint32_t wraps = 0U;

  for (uint32_t i = 0U; i < 400U; ++i) {
    const uint32_t previous = phase;
    phase = static_cast<uint32_t>(phase + delta);
    if (phase < previous) {
      ++wraps;
    }
    const uint16_t expected = fmd::lfomath::waveform12(static_cast<uint16_t>(phase >> 16U), apex);
    TEST_ASSERT_EQUAL_UINT16(expected, algorithm.step(controls));
  }
  TEST_ASSERT_GREATER_THAN_UINT32(5U, wraps);
}

void test_lfo_repeated_live_non_saw_texture_changes_remain_output_continuous_at_slow_rate() {
  MemoryReferenceTables tables;
  fmd::LfoAlgorithm algorithm(tables);
  constexpr uint16_t textures[] = {128U, 896U, 512U, 192U, 832U, 384U, 640U, 256U, 768U};
  uint16_t previous = algorithm.step({0U, 0U, 0U, 512U});

  for (uint32_t i = 0U; i < 5000U; ++i) {
    const uint16_t texture = textures[i % (sizeof(textures) / sizeof(textures[0]))];
    const uint16_t current = algorithm.step({0U, 0U, 0U, texture});
    const uint16_t difference = current > previous
        ? static_cast<uint16_t>(current - previous)
        : static_cast<uint16_t>(previous - current);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(3U, difference);
    previous = current;
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_lfo_texture_maps_monotonically_to_exact_saw_endpoints);
  RUN_TEST(test_lfo_apex_mapping_clamps_out_of_range_texture);
  RUN_TEST(test_lfo_skew_triangle_has_exact_minimum_peak_and_cycle_end);
  RUN_TEST(test_lfo_endpoint_modes_are_exact_rising_and_falling_saws);
  RUN_TEST(test_lfo_waveform_matches_piecewise_linear_reference);
  RUN_TEST(test_lfo_rise_and_fall_are_monotonic_for_each_apex);
  RUN_TEST(test_lfo_phase_remap_preserves_output_within_two_q0f16_codes);
  RUN_TEST(test_lfo_phase_wrap_preserves_overshoot);
  RUN_TEST(test_lfo_applies_texture_on_first_processing_step);
  RUN_TEST(test_lfo_phase_remap_preserves_output_over_dense_phase_grid);
  RUN_TEST(test_lfo_constant_controls_match_phase_accumulator_across_multiple_wraps);
  RUN_TEST(test_lfo_repeated_live_non_saw_texture_changes_remain_output_continuous_at_slow_rate);
  return UNITY_END();
}
