#include "platform/nano_atmega328p/AvrAnalogInputs.h"
#include "platform/nano_atmega328p/BoardConfig.h"
#include <Arduino.h>
#include <avr/interrupt.h>
namespace fmd::platform::nano {
volatile uint16_t AvrAnalogInputs::latest_[4]={0,0,0,0}; volatile uint8_t AvrAnalogInputs::index_=0;
namespace { constexpr uint8_t kChannels[4]={board::kSpeedCvChannel,board::kTextureCvChannel,board::kSpeedKnobChannel,board::kTextureKnobChannel}; constexpr uint8_t kChannelCount=4U; }
void AvrAnalogInputs::begin(){
  latest_[0]=analogRead(A4); latest_[1]=analogRead(A5); latest_[2]=analogRead(A6); latest_[3]=analogRead(A7);
  index_=0U; ADMUX=static_cast<uint8_t>(_BV(REFS0)|kChannels[0]); ADCSRB=0U;
  ADCSRA=static_cast<uint8_t>(_BV(ADEN)|_BV(ADSC)|_BV(ADATE)|_BV(ADIE)|_BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0));
  ADMUX=static_cast<uint8_t>(_BV(REFS0)|kChannels[1]);
}
void AvrAnalogInputs::handleIsr(){ latest_[index_]=ADC; index_=static_cast<uint8_t>((index_+1U)%kChannelCount); const uint8_t lookAhead=static_cast<uint8_t>((index_+1U)%kChannelCount); ADMUX=static_cast<uint8_t>(_BV(REFS0)|kChannels[lookAhead]); }
void AvrAnalogInputs::beginCycle(){ const uint8_t s=SREG; cli(); for(uint8_t i=0U;i<kChannelCount;++i)snapshot_[i]=latest_[i]; SREG=s; }
uint16_t AvrAnalogInputs::read(uint8_t c) const { return c<kChannelCount?snapshot_[c]:0U; }
}
ISR(ADC_vect){ fmd::platform::nano::AvrAnalogInputs::handleIsr(); }
