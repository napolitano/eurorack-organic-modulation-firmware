#include "platform/nano_atmega328p/SampleTimer.h"
#include "platform/nano_atmega328p/AvrMcp4922Dac.h"
#include <Arduino.h>
#include <avr/interrupt.h>
namespace fmd::platform::nano {
void SampleTimer::begin(){const uint8_t s=SREG;cli();TCCR1A=0;TCCR1B=_BV(WGM12)|_BV(CS11)|_BV(CS10);OCR1A=99;TCNT1=0;TIMSK1|=_BV(OCIE1A);SREG=s;}
void SampleTimer::handleIsr(){
#ifdef FMD_TIMING_PROBE
  PINB=_BV(PINB1);
#endif
  AvrMcp4922Dac::latchFromTimerIsr();
}
}
ISR(TIMER1_COMPA_vect){fmd::platform::nano::SampleTimer::handleIsr();}
