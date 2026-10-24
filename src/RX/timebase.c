/* timebase.c - 1 ms tick from Timer 0 in CTC mode. Uses the shared timebase.h. */
#include <avr/interrupt.h>

#include "rx_config.h"
#include "timebase.h"

static volatile uint32_t ms_count;

void timebase_init(void)
{
    TCCR0 = (1 << WGM01) | (1 << CS01) | (1 << CS00);   /* CTC, clk/64 */
    OCR0  = (uint8_t)TIMEBASE_OCR;
    TCNT0 = 0;
    TIMSK |= (1 << OCIE0);
}

ISR(TIMER0_COMP_vect)
{
    ms_count++;
}

uint32_t timebase_ms(void)
{
    uint8_t  sreg = SREG;
    uint32_t t;

    cli();                      /* a 32-bit read is not atomic on an 8-bit AVR */
    t = ms_count;
    SREG = sreg;
    return t;
}
