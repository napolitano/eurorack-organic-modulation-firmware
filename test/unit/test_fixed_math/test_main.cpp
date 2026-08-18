#include <unity.h>

#include <cstdint>

#include "fmd/domain/FixedMath.h"

void test_fixed_math_signed_multiply_matches_q1f15_reference() {
  constexpr int16_t values[] = {-16384, -8192, -2048, 0, 2048, 8192, 16384};
  for (const int16_t a : values) {
    for (const int16_t b : values) {
      const int16_t expected = static_cast<int16_t>((static_cast<int32_t>(a) * b) >> 15U);
      TEST_ASSERT_EQUAL_INT16(expected, fmd::fixedmath::mulI1F15(a, b));
    }
  }
}

void test_fixed_math_u0f16_lerp_handles_both_directions() {
  TEST_ASSERT_EQUAL_UINT16(1000U, fmd::fixedmath::lerpU0F16(0U, 1000U, 50000U));
  TEST_ASSERT_EQUAL_UINT16(50000U, fmd::fixedmath::lerpU0F16(0U, 50000U, 1000U));
  TEST_ASSERT_UINT16_WITHIN(1U, 25500U, fmd::fixedmath::lerpU0F16(32768U, 1000U, 50000U));
  TEST_ASSERT_UINT16_WITHIN(1U, 25500U, fmd::fixedmath::lerpU0F16(32768U, 50000U, 1000U));
}

void test_fixed_math_i1f15_lerp_handles_signed_endpoints() {
  TEST_ASSERT_EQUAL_INT16(-12000, fmd::fixedmath::lerpI1F15(0, -12000, 12000));
  TEST_ASSERT_INT16_WITHIN(1, 0, fmd::fixedmath::lerpI1F15(16384, -12000, 12000));
}

void test_fixed_math_u4f12_lerp_handles_both_directions() {
  TEST_ASSERT_EQUAL_UINT16(512U, fmd::fixedmath::lerpU4F12(0U, 512U, 3584U));
  TEST_ASSERT_EQUAL_UINT16(3584U, fmd::fixedmath::lerpU4F12(0U, 3584U, 512U));
  TEST_ASSERT_UINT16_WITHIN(1U, 2048U, fmd::fixedmath::lerpU4F12(2048U, 512U, 3584U));
  TEST_ASSERT_UINT16_WITHIN(1U, 2048U, fmd::fixedmath::lerpU4F12(2048U, 3584U, 512U));
}

void test_fixed_math_u16f16_lerp_handles_ascending_and_descending_tables() {
  TEST_ASSERT_EQUAL_UINT32(100000UL, fmd::fixedmath::lerpU16F16(0U, 100000UL, 200000UL));
  TEST_ASSERT_EQUAL_UINT32(200000UL, fmd::fixedmath::lerpU16F16(0U, 200000UL, 100000UL));
  TEST_ASSERT_EQUAL_UINT32(150000UL, fmd::fixedmath::lerpU16F16(32768U, 100000UL, 200000UL));
  TEST_ASSERT_EQUAL_UINT32(150000UL, fmd::fixedmath::lerpU16F16(32768U, 200000UL, 100000UL));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_fixed_math_signed_multiply_matches_q1f15_reference);
  RUN_TEST(test_fixed_math_u0f16_lerp_handles_both_directions);
  RUN_TEST(test_fixed_math_i1f15_lerp_handles_signed_endpoints);
  RUN_TEST(test_fixed_math_u4f12_lerp_handles_both_directions);
  RUN_TEST(test_fixed_math_u16f16_lerp_handles_ascending_and_descending_tables);
  return UNITY_END();
}
