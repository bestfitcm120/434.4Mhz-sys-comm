/*
 * radio_tx.c - Manchester OOK transmitter.
 *
 * Timer 1 interrupts every HALF_BIT_US (250 us). Bits go out MSB first:
 *   bit 1 = carrier on for one half-bit, then off
 *   bit 0 = carrier off for one half-bit, then on
 * so the signal always has equal on and off time.
 *
 * KEY high switches the RF supply on through the T4 keyer. The line rests low.
 */
#include <avr/interrupt.h>
#include <string.h>

#include "config.h"
#include "radio_tx.h"

typedef enum {
    TX_IDLE = 0,
    TX_SENDING,
    TX_GAP
} tx_state_t;

static volatile tx_state_t state;

static uint8_t buf[PKT_MAX_LEN];
static uint8_t buf_len;
static uint8_t byte_idx;
static uint8_t bit_idx;
static bool    second_half;
static uint8_t repeats_left;
static uint16_t gap_ticks;

static inline void key_set(bool on)
{
    if (on) {
        KEY_PORT |= (1 << KEY_BIT);
    } else {
        KEY_PORT &= (uint8_t)~(1 << KEY_BIT);
    }
}

static void start_pass(void)
{
    byte_idx    = 0;
    bit_idx     = 0;
    second_half = false;
}

void radio_tx_init(void)
{
    KEY_DDR |= (1 << KEY_BIT);
    key_set(false);

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11);    /* CTC, top = OCR1A, clk/8 */
    OCR1A  = (uint16_t)RADIO_TIMER_TOP;
    TCNT1  = 0;

    state = TX_IDLE;
}

bool radio_tx_send(const uint8_t *pkt, uint8_t len)
{
    uint8_t sreg;

    if (len == 0 || len > PKT_MAX_LEN) {
        return false;
    }

    sreg = SREG;
    cli();
    if (state != TX_IDLE) {
        SREG = sreg;
        return false;
    }

    memcpy(buf, pkt, len);
    buf_len      = len;
    repeats_left = TX_REPEATS - 1U;
    start_pass();
    state = TX_SENDING;

    TCNT1 = 0;
    TIFR  = (1 << OCF1A);                   /* drop any stale flag */
    TIMSK |= (1 << OCIE1A);
    SREG  = sreg;
    return true;
}

bool radio_tx_busy(void)
{
    return state != TX_IDLE;
}

ISR(TIMER1_COMPA_vect)
{
    switch (state) {
    case TX_SENDING:
        if (byte_idx >= buf_len) {
            /* every bit of this pass is out */
            key_set(false);
            if (repeats_left > 0) {
                repeats_left--;
                gap_ticks = (uint16_t)TX_GAP_TICKS;
                state     = TX_GAP;
            } else {
                state = TX_IDLE;
                TIMSK &= (uint8_t)~(1 << OCIE1A);
            }
        } else {
            uint8_t bit = (uint8_t)((buf[byte_idx] >> (7U - bit_idx)) & 1U);

            key_set(second_half ? (bit == 0U) : (bit != 0U));

            if (second_half) {
                second_half = false;
                if (++bit_idx == 8U) {
                    bit_idx = 0;
                    byte_idx++;
                }
            } else {
                second_half = true;
            }
        }
        break;

    case TX_GAP:
        if (--gap_ticks == 0) {
            start_pass();
            state = TX_SENDING;
        }
        break;

    default:
        TIMSK &= (uint8_t)~(1 << OCIE1A);
        break;
    }
}
