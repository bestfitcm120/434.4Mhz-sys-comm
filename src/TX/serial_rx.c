/*
 * serial_rx.c - serial text input on PD0 (RXD) at 9600 baud, 8N1.
 *
 * The receive interrupt collects printable characters into a line buffer.
 * A carriage return or line feed completes the line. While a completed line
 * waits for main(), new characters are ignored, so a line is never
 * overwritten halfway. Characters beyond MSG_MAX_LEN are dropped.
 *
 * This file also owns the auto message: AUTO_MESSAGE becomes due every
 * AUTO_PERIOD_MS. Typing, a sent line, or a button message pushes it back,
 * so it never cuts in on something else.
 */
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include "config.h"
#include "timebase.h"
#include "serial_rx.h"

static volatile uint8_t rx_buf[MSG_MAX_LEN];
static volatile uint8_t rx_len;
static volatile bool    line_ready;
static volatile bool    typing;         /* set by the ISR on every accepted character */

static uint32_t auto_last_ms;

void serial_rx_init(void)
{
    UBRRH = (uint8_t)(UART_UBRR >> 8);
    UBRRL = (uint8_t)(UART_UBRR & 0xFF);
    UCSRA = 0;
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);     /* 8 data bits, no parity, 1 stop */
    UCSRB = (1 << RXEN) | (1 << RXCIE);                     /* receiver and its interrupt only */

    rx_len       = 0;
    line_ready   = false;
    typing       = false;
    auto_last_ms = timebase_ms();
}

ISR(USART_RXC_vect)
{
    uint8_t status = UCSRA;     /* must be read before UDR */
    uint8_t c      = UDR;

    if (status & ((1 << FE) | (1 << DOR) | (1 << PE))) {
        return;                 /* bad frame: drop the character */
    }
    if (line_ready) {
        return;                 /* previous line not collected yet */
    }

    if (c == '\r' || c == '\n') {
        if (rx_len > 0) {
            line_ready = true;
        }
        return;
    }

    if (c >= 0x20 && c <= 0x7E && rx_len < MSG_MAX_LEN) {
        rx_buf[rx_len++] = c;
        typing = true;
    }
}

uint8_t serial_rx_get_line(char *dst)
{
    uint8_t sreg = SREG;
    uint8_t len  = 0;

    cli();
    if (line_ready) {
        len = rx_len;
        for (uint8_t i = 0; i < len; i++) {
            dst[i] = (char)rx_buf[i];
        }
        rx_len     = 0;
        line_ready = false;
        typing     = false;     /* the line is complete, typing is over */
    }
    SREG = sreg;

    if (len > 0) {
        dst[len] = '\0';
        serial_rx_restart_auto();
    }
    return len;
}

void serial_rx_restart_auto(void)
{
    auto_last_ms = timebase_ms();
}

uint8_t serial_rx_get_auto(char *dst)
{
    uint8_t sreg = SREG;
    bool    was_typing;
    uint8_t len;

    cli();
    was_typing = typing;
    typing     = false;
    SREG       = sreg;

    if (was_typing) {           /* someone is typing: wait another period */
        serial_rx_restart_auto();
        return 0;
    }
    if ((timebase_ms() - auto_last_ms) < AUTO_PERIOD_MS) {
        return 0;
    }

    len = (uint8_t)(sizeof(AUTO_MESSAGE) - 1);
    memcpy(dst, AUTO_MESSAGE, len + 1U);    /* includes the terminator */
    serial_rx_restart_auto();
    return len;
}
