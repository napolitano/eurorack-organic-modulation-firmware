#include "platform/nano_atmega328p/AvrMcp4922Dac.h"
#include "platform/nano_atmega328p/BoardConfig.h"
#include <Arduino.h>
#include <SPI.h>
#include <avr/interrupt.h>
namespace fmd::platform::nano {
volatile bool AvrMcp4922Dac::queued_=false; volatile uint16_t AvrMcp4922Dac::missed_=0;
void AvrMcp4922Dac::begin(){pinMode(board::kDacCsPin,OUTPUT);digitalWrite(board::kDacCsPin,HIGH);SPI.begin();SPI.beginTransaction(SPISettings(8000000UL,MSBFIRST,SPI_MODE0));queued_=false;}
bool AvrMcp4922Dac::ready() const { const uint8_t s=SREG;cli();const bool r=!queued_;SREG=s;return r; }
void AvrMcp4922Dac::queue12Bit(uint16_t value){ value&=0x0FFFU; digitalWrite(board::kDacCsPin,LOW); const uint16_t word=static_cast<uint16_t>(0x3000U|value); SPI.transfer(static_cast<uint8_t>(word>>8U)); SPI.transfer(static_cast<uint8_t>(word)); const uint8_t s=SREG;cli();queued_=true;SREG=s; }
void AvrMcp4922Dac::latchFromTimerIsr(){ if(queued_){PORTB|=_BV(PORTB2);queued_=false;}else if(missed_!=0xFFFFU){++missed_;} }
uint16_t AvrMcp4922Dac::missedLatchCount() const {const uint8_t s=SREG;cli();const uint16_t n=missed_;SREG=s;return n;}
}
