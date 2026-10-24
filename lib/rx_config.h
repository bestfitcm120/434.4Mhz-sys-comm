/*
 * rx_config.h - hardware and decoder definitions for the receiver.
 *
 * The protocol constants (sync bytes, CRC polynomial, MSG_MAX_LEN, HALF_BIT_US,
 * the 1 ms timebase) come from the transmitter's config.h, so both sides always
 * agree. Pins here match the receiver schematic (R7, R8).
 */
#ifndef RX_CONFIG_H
#define RX_CONFIG_H

#include "config.h"             /* shared header from lib/TX */

/* ---------- LCD, HD44780, 4-bit mode, RW tied to GND (R8) ---------- */

#define LCD_PORT        PORTA
#define LCD_DDR         DDRA
#define LCD_D4_BIT      PA0     /* D4..D7 = PA0..PA3, must be consecutive */
#define LCD_RS_BIT      PA4
#define LCD_E_BIT       PA5
#define LCD_COLS        16U
#define LCD_ROWS        2U

#define LCD_WAITING_TEXT "WAITING FOR DATA"

/* ---------- lock LED (PD7 -> resistor -> LED -> GND) ---------- */

#define RXLED_DDR       DDRD
#define RXLED_PORT      PORTD
#define RXLED_BIT       PD7
#define RXLED_BLINK_MS  100U

/* ---------- slicer input on ICP1 (PD6) ---------- */

#define RX_IN_DDR       DDRD
#define RX_IN_PIN       PIND
#define RX_IN_BIT       PD6
#define RX_SLICER_INVERTED  0   /* set to 1 if the slicer output is high when the carrier is off */

/* ---------- edge timing (Timer 1, clk/8 = 1 us per tick at 8 MHz) ---------- */

#define RX_TIMER_PRESCALER  8UL
#define RX_US_TO_TICKS(us)  (((us) * (F_CPU / RX_TIMER_PRESCALER)) / 1000000UL)

#define RX_T_HALF       RX_US_TO_TICKS(HALF_BIT_US)         /* one half-bit: 250 */
#define RX_T_MIN        (RX_T_HALF / 2UL)                   /* shorter than this = noise */
#define RX_T_MID        ((RX_T_HALF * 3UL) / 2UL)           /* below = one half-bit, above = two */
#define RX_T_MAX        ((RX_T_HALF * 5UL) / 2UL)           /* longer than this = not Manchester */
#define RX_TIMEOUT_US   1000UL                              /* no edge for this long = end of packet */
#define RX_TIMEOUT_TICKS RX_US_TO_TICKS(RX_TIMEOUT_US)

/* ---------- repeats ---------- */

/* The transmitter sends each message TX_REPEATS times, 30 ms apart. A copy that
 * arrives this soon after an identical one is not shown or blinked again. */
#define RX_DUP_WINDOW_MS    400UL

#endif /* RX_CONFIG_H */
