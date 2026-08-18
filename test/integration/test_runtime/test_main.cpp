#include <unity.h>
#include "fmd/application/DriftRuntime.h"
#include "DriftTestRig.h"
void test_runtime_queues_dac_and_led_together(){DriftTestRig r(fmd::Algorithm::Lfo,1);const uint16_t out=r.tick(0,0,512,512);TEST_ASSERT_EQUAL_UINT16(out,r.runtime.lastOutput());TEST_ASSERT_EQUAL_UINT8(r.tables.gamma8(static_cast<uint8_t>(out>>4)),r.led.duty);TEST_ASSERT_FALSE(r.dac.isReady);}
void test_runtime_does_not_advance_while_dac_busy(){DriftTestRig r(fmd::Algorithm::Lfo,1);r.tick(0,0,512,512);const uint16_t first=r.runtime.lastOutput();TEST_ASSERT_FALSE(r.runtime.processIfReady());TEST_ASSERT_EQUAL_UINT16(first,r.runtime.lastOutput());}
int main(int,char**){UNITY_BEGIN();RUN_TEST(test_runtime_queues_dac_and_led_together);RUN_TEST(test_runtime_does_not_advance_while_dac_busy);return UNITY_END();}
