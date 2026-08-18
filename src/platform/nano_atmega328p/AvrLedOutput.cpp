#include "platform/nano_atmega328p/AvrLedOutput.h"
#include "platform/nano_atmega328p/BoardConfig.h"
#include <Arduino.h>
namespace fmd::platform::nano { void AvrLedOutput::begin(){pinMode(board::kLedPin,OUTPUT);analogWrite(board::kLedPin,0);} void AvrLedOutput::setBrightness(uint8_t d){analogWrite(board::kLedPin,d);} }
