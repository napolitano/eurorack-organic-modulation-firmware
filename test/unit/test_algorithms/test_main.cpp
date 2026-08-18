#include <unity.h>
#include "fmd/domain/DriftEngine.h"
#include "MemoryReferenceTables.h"
void test_each_algorithm_stays_in_dac_range(){MemoryReferenceTables t;const fmd::ControlFrame c{1023,1023,1023,1023};for(uint8_t a=0;a<4;++a){fmd::DriftEngine e(static_cast<fmd::Algorithm>(a),0x4A51,t);for(int i=0;i<2000;++i)TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095,e.step(c));}}
void test_algorithms_are_deterministic_for_fixed_seed(){MemoryReferenceTables t;const fmd::ControlFrame c{222,333,444,555};for(uint8_t a=0;a<4;++a){fmd::DriftEngine x(static_cast<fmd::Algorithm>(a),0x1234,t),y(static_cast<fmd::Algorithm>(a),0x1234,t);for(int i=0;i<300;++i)TEST_ASSERT_EQUAL_UINT16(x.step(c),y.step(c));}}
void test_invalid_algorithm_enum_returns_safe_zero_output(){MemoryReferenceTables t;fmd::DriftEngine e(static_cast<fmd::Algorithm>(0xFFU),0x1234U,t);TEST_ASSERT_EQUAL_UINT16(0U,e.step({0U,0U,0U,0U}));}
int main(int,char**){UNITY_BEGIN();RUN_TEST(test_each_algorithm_stays_in_dac_range);RUN_TEST(test_algorithms_are_deterministic_for_fixed_seed);RUN_TEST(test_invalid_algorithm_enum_returns_safe_zero_output);return UNITY_END();}
