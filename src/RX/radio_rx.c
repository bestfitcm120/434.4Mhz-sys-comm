/*
 * radio_rx.c - Manchester OOK receiver.
 *
 * Timer 1 runs at 1 us per tick and captures every edge on ICP1. The gap since
 * the previous edge is one half-bit (about 250 us) or two (about 500 us), so
 * each edge adds one or two half-bit levels to the decoder. The level that was
 * held before the edge is the one that gets added.
 *
 *   bit 1 = high half-bit, then low
 *   bit 0 = low half-bit, then high
 *
 * The decoder keeps the last 32 half-bits and waits for the sync word 2D D4.
 * After that it pairs half-bits into bits, bits into bytes, and checks ADDR,
 * LEN and the CRC-8. If no edge arrives for RX_TIMEOUT_US the packet is over:
 * the level still held at that moment is the last half-bit, so it is added too.
 */
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>

#include "rx_config.h"
#include "packet.h"
#include "radio_rx.h"

typedef enum {
    ST_HUNT = 0,    /* looking for the sync word */
    ST_ADDR,
    ST_LEN,
    ST_DATA,
    ST_CRC
} rx_state_t;

/* ---- decoder state (touched only inside the two interrupts) ---- */

static rx_state_t state;
static uint32_t   history;              /* last 32 half-bits, newest in bit 0 */
static uint32_t   sync_pattern;         /* Manchester form of 2D D4 */
static uint8_t    half;                 /* 0 = next half-bit starts a bit, 1 = ends it */
static uint8_t    first_half;
static uint8_t    cur_byte;
static uint8_t    bit_count;
static uint8_t    work[3U + MSG_MAX_LEN];   /* ADDR, LEN, message, CRC */
static uint8_t    work_len;             /* bytes in work[] */
static uint8_t    want_len;             /* LEN field */

static bool       have_last;
static uint16_t   last_edge;
static uint8_t    cur_level;            /* level held since the last edge */

/* ---- hand-over to main ---- */

static volatile uint8_t ready;
static uint8_t          out_len;
static char             out_buf[MSG_MAX_LEN];

static uint32_t manchester16(uint16_t value)
{
    uint32_t r = 0;

    for (uint8_t i = 0; i < 16; i++) {
        uint8_t bit = (uint8_t)((value >> (15 - i)) & 1U);
        r = (r << 2) | (bit ? 0x2UL : 0x1UL);       /* 1 -> high,low  0 -> low,high */
    }
    return r;
}

static void decoder_reset(void)
{
    state     = ST_HUNT;
    history   = 0;
    half      = 0;
    bit_count = 0;
    cur_byte  = 0;
    work_len  = 0;
}

static void packet_done(void)
{
    uint8_t crc_rx = work[work_len];        /* stored by byte_done() */

    if (packet_crc8(work, work_len) == crc_rx && !ready) {
        memcpy(out_buf, &work[2], want_len);
        out_len = want_len;
        ready   = 1;
    }
    decoder_reset();
}

static void byte_done(uint8_t b)
{
    switch (state) {
    case ST_ADDR:
        if (b != PKT_ADDRESS) {
            decoder_reset();
            return;
        }
        work[work_len++] = b;
        state = ST_LEN;
        break;

    case ST_LEN:
        if (b == 0 || b > MSG_MAX_LEN) {
            decoder_reset();
            return;
        }
        want_len = b;
        work[work_len++] = b;
        state = ST_DATA;
        break;

    case ST_DATA:
        work[work_len++] = b;
        if (work_len == (uint8_t)(2U + want_len)) {
            state = ST_CRC;
        }
        break;

    case ST_CRC:
        work[work_len] = b;                 /* one spare slot after the message */
        packet_done();
        break;

    default:
        break;
    }
}

/* One half-bit level (1 = high) from the line. */
static void push_half(uint8_t level)
{
    history = (history << 1) | level;

    if (state == ST_HUNT) {
        if (history == sync_pattern) {
            state     = ST_ADDR;
            half      = 0;
            bit_count = 0;
            cur_byte  = 0;
            work_len  = 0;
        }
        return;
    }

    if (half == 0) {
        first_half = level;
        half = 1;
        return;
    }

    half = 0;
    if (first_half == level) {              /* no mid-bit transition: not Manchester */
        decoder_reset();
        return;
    }
    cur_byte = (uint8_t)((cur_byte << 1) | first_half);
    if (++bit_count == 8) {
        uint8_t b = cur_byte;
        bit_count = 0;
        cur_byte  = 0;
        byte_done(b);
    }
}

/* Aim the capture at the next edge, given the level the line has now. */
static void arm_for_level(uint8_t physical_high)
{
    if (physical_high) {
        TCCR1B &= (uint8_t)~(1 << ICES1);   /* next edge is falling */
    } else {
        TCCR1B |= (1 << ICES1);             /* next edge is rising */
    }
    TIFR = (1 << ICF1);                     /* changing the edge can set the flag */
}

void radio_rx_init(void)
{
    RX_IN_DDR &= (uint8_t)~(1 << RX_IN_BIT);    /* input, the slicer has its own pull-up */

    sync_pattern = manchester16(((uint16_t)PKT_SYNC_HI << 8) | PKT_SYNC_LO);
    decoder_reset();
    have_last = false;
    ready     = 0;

    TCCR1A = 0;
    TCCR1B = (1 << ICNC1) | (1 << CS11);        /* normal mode, clk/8, noise canceller on */
    arm_for_level((RX_IN_PIN >> RX_IN_BIT) & 1U);
    TIFR   = (1 << OCF1A);
    TIMSK |= (1 << TICIE1);
}

/* Every edge from the slicer. */
ISR(TIMER1_CAPT_vect)
{
    uint8_t  rising  = (TCCR1B & (1 << ICES1)) ? 1U : 0U;
    uint16_t now     = ICR1;
    uint8_t  new_lvl = (uint8_t)(rising ^ RX_SLICER_INVERTED);

    TCCR1B ^= (1 << ICES1);                     /* the next edge is the opposite one */
    TIFR    = (1 << ICF1);

    if (have_last) {
        uint16_t dt = (uint16_t)(now - last_edge);

        if (dt < RX_T_MIN || dt > RX_T_MAX) {
            decoder_reset();                    /* glitch or gap: start looking again */
        } else {
            push_half(cur_level);
            if (dt >= RX_T_MID) {
                push_half(cur_level);           /* two half-bits long */
            }
        }
    }

    have_last = true;
    last_edge = now;
    cur_level = new_lvl;

    OCR1A  = (uint16_t)(now + RX_TIMEOUT_TICKS);    /* silence timer */
    TIFR   = (1 << OCF1A);
    TIMSK |= (1 << OCIE1A);
}

/* No edge for RX_TIMEOUT_US: the packet (if any) has ended. */
ISR(TIMER1_COMPA_vect)
{
    TIMSK &= (uint8_t)~(1 << OCIE1A);

    if (have_last && state != ST_HUNT) {
        push_half(cur_level);                   /* the half-bit that had no edge after it */
    }
    decoder_reset();
    have_last = false;

    arm_for_level((RX_IN_PIN >> RX_IN_BIT) & 1U);   /* re-sync the edge select to the real pin */
}

uint8_t radio_rx_get(char *out)
{
    uint8_t n;

    if (!ready) {
        return 0;
    }
    n = out_len;
    memcpy(out, out_buf, n);
    out[n] = '\0';
    ready = 0;
    return n;
}
