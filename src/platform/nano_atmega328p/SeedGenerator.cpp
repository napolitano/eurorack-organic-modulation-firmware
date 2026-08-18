#include "platform/nano_atmega328p/SeedGenerator.h"
#include <Arduino.h>
namespace fmd::platform::nano {
namespace { uint16_t readInternal(uint8_t mux){const uint8_t oldAdmux=ADMUX,oldAdcsra=ADCSRA;ADMUX=static_cast<uint8_t>(_BV(REFS0)|(mux&0x0FU));ADCSRA=_BV(ADEN)|_BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0)|_BV(ADSC);while((ADCSRA&_BV(ADSC))!=0){}const uint16_t v=ADC;ADMUX=oldAdmux;ADCSRA=oldAdcsra;return v;} }
uint16_t generateSeed(){
  const uint16_t a0=analogRead(A0),a1=analogRead(A1),a2=analogRead(A2),a3=analogRead(A3),a6=analogRead(A6),a7=analogRead(A7);
  const uint16_t temp=readInternal(8),vbg=readInternal(14);
  return static_cast<uint16_t>(((a0&7U)<<13U)|((a1&7U)<<10U)|((a2&7U)<<7U)|((a3&7U)<<4U)|((a6&1U)<<3U)|((a7&1U)<<2U)|((temp&1U)<<1U)|(vbg&1U));
}
}
