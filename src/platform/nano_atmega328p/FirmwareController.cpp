#include "platform/nano_atmega328p/FirmwareController.h"
#include "platform/nano_atmega328p/BoardConfig.h"
#include "platform/nano_atmega328p/AvrAnalogInputs.h"
#include "platform/nano_atmega328p/AvrMcp4922Dac.h"
#include "platform/nano_atmega328p/AvrLedOutput.h"
#include "platform/nano_atmega328p/AvrReferenceTables.h"
#include "platform/nano_atmega328p/SampleTimer.h"
#include "platform/nano_atmega328p/SeedGenerator.h"
#include "fmd/application/DriftRuntime.h"
#include <Arduino.h>
#include <new>
namespace fmd::platform::nano {
namespace {
AvrAnalogInputs analog; AvrMcp4922Dac dac; AvrLedOutput led; AvrReferenceTables tables;
alignas(DriftRuntime) uint8_t runtimeStorage[sizeof(DriftRuntime)]; DriftRuntime* runtime=nullptr;
}
void FirmwareController::begin(){
#ifdef FMD_TIMING_PROBE
  pinMode(9,OUTPUT);
#endif
  pinMode(board::kConfig1Pin,INPUT_PULLUP);pinMode(board::kConfig2Pin,INPUT_PULLUP);
  const bool c1=digitalRead(board::kConfig1Pin)==LOW,c2=digitalRead(board::kConfig2Pin)==LOW; const uint16_t seed=generateSeed();
  analog.begin();dac.begin();led.begin(); runtime=new(runtimeStorage) DriftRuntime(algorithmFromConfig(c1,c2),seed,analog,dac,led,tables); SampleTimer::begin();
}
void FirmwareController::run(){if(runtime!=nullptr)runtime->processIfReady();}
}
