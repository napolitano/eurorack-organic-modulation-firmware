#include <unity.h>
#include "fmd/domain/DriftEngine.h"
void test_original_config_pin_mapping(){TEST_ASSERT_EQUAL_INT((int)fmd::Algorithm::Perlin,(int)fmd::algorithmFromConfig(false,false));TEST_ASSERT_EQUAL_INT((int)fmd::Algorithm::Brownian,(int)fmd::algorithmFromConfig(false,true));TEST_ASSERT_EQUAL_INT((int)fmd::Algorithm::Bezier,(int)fmd::algorithmFromConfig(true,false));TEST_ASSERT_EQUAL_INT((int)fmd::Algorithm::Lfo,(int)fmd::algorithmFromConfig(true,true));}
int main(int,char**){UNITY_BEGIN();RUN_TEST(test_original_config_pin_mapping);return UNITY_END();}
