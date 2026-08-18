#include <unity.h>
#include "fmd/domain/DriftEngine.h"
#include "MemoryReferenceTables.h"
void test_control_grid_preserves_12bit_output_invariant(){MemoryReferenceTables t;const uint16_t p[]={0,1,127,511,512,1019,1020,1023};for(uint8_t a=0;a<4;++a){fmd::DriftEngine e(static_cast<fmd::Algorithm>(a),0xBEEF,t);for(uint16_t sc:p)for(uint16_t tc:p)for(uint16_t sk:p)for(uint16_t tk:p){TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095,e.step({sc,tc,sk,tk}));}}}
int main(int,char**){UNITY_BEGIN();RUN_TEST(test_control_grid_preserves_12bit_output_invariant);return UNITY_END();}
