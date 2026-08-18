#ifndef FMD_TEST_DRIFT_TEST_RIG_H
#define FMD_TEST_DRIFT_TEST_RIG_H
#include "fmd/application/DriftRuntime.h"
#include "MemoryReferenceTables.h"
class FakeAnalog final:public fmd::IAnalogInputs{public:uint16_t v[4]={0,0,0,0};void beginCycle() override{}uint16_t read(uint8_t c)const override{return c<4?v[c]:0;}};
class FakeDac final:public fmd::IDacOutput{public:bool isReady=true;uint16_t queued=0;bool ready()const override{return isReady;}void queue12Bit(uint16_t x)override{queued=x;isReady=false;}void latch(){isReady=true;}};
class FakeLed final:public fmd::ILedOutput{public:uint8_t duty=0;void setBrightness(uint8_t d)override{duty=d;}};
class DriftTestRig{public:MemoryReferenceTables tables;FakeAnalog analog;FakeDac dac;FakeLed led;fmd::DriftRuntime runtime;DriftTestRig(fmd::Algorithm a,uint16_t seed):runtime(a,seed,analog,dac,led,tables){}uint16_t tick(uint16_t scv,uint16_t tcv,uint16_t sk,uint16_t tk){analog.v[0]=scv;analog.v[1]=tcv;analog.v[2]=sk;analog.v[3]=tk;dac.latch();runtime.processIfReady();return dac.queued;}};
#endif
