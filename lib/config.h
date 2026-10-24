/*
 * config.h - every hardware and protocol definition for the transmitter.
 * Matches the 434.4 MHz Link project doc (T1-T8).
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 8000000UL              /* 8 MHz crystal on X1 */
#endif

/* ---------- pins (T3 schematic) ---------- */

/* PB0 -> T4 OOK keyer. High = carrier on, low = carrier off. */
#define KEY_DDR         DDRB
#define KEY_PORT        PORTB
#define KEY_BIT         PB0

/* PB1 -> R2 330 ohm -> D2 -> GND. High = LED on. */
#define LED_DDR         DDRB
#define LED_PORT        PORTB
#define LED_BIT         PB1

/* PD2 -> S1 -> GND, internal pull-up. Low = pressed. */
#define BTN_DDR         DDRD
#define BTN_PORT        PORTD
#define BTN_PINREG      PIND
#define BTN_BIT         PD2

/* Serial input arrives on PD0 (RXD), wired from T2 or a USB-UART adapter. */

/* ---------- 1 ms time base (Timer 0, CTC) ---------- */

#define TIMEBASE_PRESCALER      64UL
#define TIMEBASE_OCR            ((F_CPU / TIMEBASE_PRESCALER / 1000UL) - 1UL)   /* 124 */

/* ---------- button ---------- */

#define BTN_SAMPLE_MS           5U      /* read the pin every 5 ms */
#define BTN_DEBOUNCE_SAMPLES    6U      /* 6 x 5 ms = 30 ms stable */

/* ---------- serial input (T2 -> PD0) ---------- */

#define UART_BAUD               9600UL  /* 8 data bits, no parity, 1 stop bit */
#define UART_UBRR               ((F_CPU / (16UL * UART_BAUD)) - 1UL)            /* 51 */

/* ---------- messages ---------- */

#define MSG_MAX_LEN             32U     /* two LCD lines of 16 characters */

#define BUTTON_MESSAGE          "BUTTON PRESSED"    /* sent when S1 is pressed */
#define AUTO_MESSAGE            "HELLO FROM CAIRO"  /* sent every AUTO_PERIOD_MS */
#define AUTO_PERIOD_MS          2000UL

/* ---------- packet (see "Packet format" in the project doc) ---------- */

#define PKT_PREAMBLE_BYTE       0xAAU   /* sent twice */
#define PKT_SYNC_HI             0x2DU
#define PKT_SYNC_LO             0xD4U
#define PKT_ADDRESS             0x01U
#define PKT_CRC_POLY            0x07U   /* CRC-8, initial value 0 */
#define PKT_OVERHEAD            7U      /* 2 preamble + 2 sync + ADDR + LEN + CRC */
#define PKT_MAX_LEN             (PKT_OVERHEAD + MSG_MAX_LEN)

/* ---------- radio (Manchester OOK on KEY) ---------- */

#define HALF_BIT_US             250UL   /* 500 us per bit = 2 kbit/s */
#define RADIO_TIMER_PRESCALER   8UL     /* Timer 1 clock = 1 MHz */
#define RADIO_TIMER_TOP         (((F_CPU / RADIO_TIMER_PRESCALER / 1000000UL) * HALF_BIT_US) - 1UL)

#define TX_REPEATS              3U      /* every message goes out three times */
#define TX_GAP_MS               30U     /* quiet time between the repeats */
#define TX_GAP_TICKS            ((TX_GAP_MS * 1000UL) / HALF_BIT_US)            /* 120 */

#endif /* CONFIG_H */
